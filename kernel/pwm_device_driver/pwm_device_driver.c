#include <linux/version.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/pwm.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>

#define DEVICE_NAME "w_pwm_motor"

#define PWM_MOTOR_ON _IOW('a','a',int32_t*)
#define PWM_MOTOR_OFF _IOW('a','a',int32_t*)

#define SERVO_PWM_PERIOD  	20000000
#define SERVO_MAX_DUTY  	 2500000
#define SERVO_MIN_DUTY		 1050000
	
static int major;

struct pwm_servo_motor{
   struct pwm_device *pwm;
   unsigned long period;
};

static struct class *custom_pwm_class = NULL;
struct pwm_servo_motor *servo_motor;

static int servo_pwm_motor_on(struct pwm_device *pwm_motor, unsigned int duty_cycle)
{
      struct pwm_state state;
      int error;
      
      pr_info("Turn on PWM servo motor\n");
      
      pwm_get_state(pwm_motor, &state);
      pwm_set_duty_cycle(pwm_motor,1000000);
      state.enabled = true;               
      error = pwm_apply_state(pwm_motor,&state);
      if(error)
      {
      	pr_err("Fail to turn on pwm servo\n");
      	return error;
      }
      
      pr_info("%d Period %d duty_cicle %d Enable %d",error, (int)state.period,(int)state.duty_cycle, state.enabled);
      
      return 0; 
}

static int pwm_servo_open(struct inode *device_file, struct file *instance) {
	printk("Open PWM Servo service motor!\n");
	return 0;
}

static long pwm_servo_ioctl(struct file *fp,  unsigned int cmd, unsigned long arg) {
	 int32_t value = 0; 
         switch(cmd) {
                case PWM_MOTOR_ON:
                        if( copy_from_user(&value ,(int32_t*) arg, sizeof(value)) )
                              pr_err("Data Write : Err!\n");
                       
                        servo_pwm_motor_on(servo_motor->pwm, 1500000);
                        pr_info("Value = %d\n", value);
                        break;
                default:
                        pr_info("Default\n");
         }  
	return 0;
}

struct file_operations fops={
	.owner			= THIS_MODULE,
	.open			= pwm_servo_open,
	.unlocked_ioctl 	= pwm_servo_ioctl,
};

static int pwm_servo_motor_probe(struct platform_device *pdev)
{
   struct device *dev = &pdev->dev;	
   struct device_node *node = dev->of_node;
   struct pwm_state state;
   struct device *pwm_device;
   //struct pwm_servo_motor *servo_motor;
   
   int error;
  
  pr_info("Probe servo device driver %s\n",pdev->name);
  
   if(node == NULL)
   {
   	dev_err(dev,"Non DT Platfomrs supprted\n");
   	return -EINVAL;
   }
   	
  
   servo_motor = devm_kzalloc(dev,sizeof(*servo_motor), GFP_KERNEL);
   if(!servo_motor){
   	pr_err("Fail to allocate memory\n");
   	return -ENOMEM;
   }
   
   major = register_chrdev(0,"pwm_servo",&fops);
   
   if(major < 0)
   {
       pr_err("Failed to register character device\n");
       return major;
   }
   
   pwm_device = device_create(custom_pwm_class, NULL, MKDEV(major,0),NULL, DEVICE_NAME);
   if(IS_ERR(pwm_device))
   {
   	error = PTR_ERR(pwm_device);
        pr_err("Error %d try to create the device\n",error);
        goto fail;
   }
   
   servo_motor->pwm = devm_pwm_get(dev,NULL);
   if(IS_ERR(servo_motor->pwm))
   {
   	error=PTR_ERR(servo_motor->pwm);
   	dev_err(dev,"Failed to apply initial PWM satte: %d\n",error);
   	return error;
   }
  
   
   pwm_init_state(servo_motor->pwm, &state);
   state.enabled = false;
   state.polarity = PWM_POLARITY_NORMAL;
   error = pwm_apply_state(servo_motor->pwm, &state);   
   if(error)
   {
   	dev_err(dev,"Failed to apply initial PWM state: %d\n", error);
   	return error;
   }
   
   error = pwm_config(servo_motor->pwm, 700000, 20000000); 
   if(error)
   {
   	dev_err(dev,"Failed to config PWM: %d\n", error);
   	return error;
   }
    platform_set_drvdata(pdev,servo_motor);
    pr_info("Finish initialization servo device driver\n");
   
   return 0;
   
   fail:
   	if(servo_motor)
   	    kfree(servo_motor);
   	    
   return error;
}

static int pwm_servo_motor_remove(struct platform_device *pdev)
{
     struct pwm_servo_motor *servo = platform_get_drvdata(pdev);
     pwm_disable(servo->pwm);
     device_destroy(custom_pwm_class,MKDEV(major,0));
     return 0;
}

static const struct of_device_id pwm_servo_motor_match[] = {
	{.compatible = "servo_motor,w_pwm_servo_motor",},
	{},
};
MODULE_DEVICE_TABLE(of, pwm_servo_motor_match);

static struct platform_driver pwm_servo_motor = {
	.probe = pwm_servo_motor_probe,
	.remove = pwm_servo_motor_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name 	= "pwm-servo_motor",
		.of_match_table = of_match_ptr(pwm_servo_motor_match),
	},
};

static int __init custom_pwm_init(void)
{
   int status;
   
   pr_info("Initializind pwm device driver\n");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,4,0)
   custom_pwm_class  = class_create("custom_pwm");
#else   
   custom_pwm_class  = class_create(THIS_MODULE,"custom_pwm");
#endif
   
   if(IS_ERR(custom_pwm_class))
   	return PTR_ERR(custom_pwm_class);
   
   status = platform_driver_register(&pwm_servo_motor);
   if(status < 0){
   	pr_err("Failt to register call\n");
   	class_destroy(custom_pwm_class); 
   }
   return status;
}

static void __exit custom_pwm_exit(void)
{
   platform_driver_unregister(&pwm_servo_motor);
   class_destroy(custom_pwm_class);
   pr_info("Finish the use of device driver\n");
}

module_init(custom_pwm_init);
module_exit(custom_pwm_exit);

MODULE_AUTHOR("William Sanchez");
MODULE_DESCRIPTION("PWM, Servo Motor");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:pwm-servo_motor");
