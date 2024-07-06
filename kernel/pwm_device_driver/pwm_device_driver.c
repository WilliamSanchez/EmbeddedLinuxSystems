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

struct pwm_servo_motor{
   struct pwm_device *pwm;
   unsigned long period;
};

static struct class *custom_pwm_class = NULL;

static int pwm_servo_motor_probe(struct platform_device *pdev)
{
   struct device *dev = &pdev->dev;	
   struct pwm_servo_motor *servo_motor;
   struct device_node *node = dev->of_node;
   struct pwm_state state;
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
   
   servo_motor->pwm = devm_pwm_get(dev,NULL);
   if(IS_ERR(servo_motor->pwm))
   {
   	error=PTR_ERR(servo_motor->pwm);
   	dev_err(dev,"Failed to apply initial PWM satte: %d\n",error);
   	return error;
   }
  
   
   pwm_init_state(servo_motor->pwm, &state);
   state.enabled = true;
   state.polarity = PWM_POLARITY_NORMAL;
   error = pwm_apply_state(servo_motor->pwm, &state);   
   if(error)
   {
   	dev_err(dev,"Failed to apply initial PWM state: %d\n", error);
   	return error;
   }
   
   error = pwm_config(servo_motor->pwm, 550000, 20000000);   
   if(error)
   {
   	dev_err(dev,"Failed to config PWM: %d\n", error);
   	return error;
   }
   /*
   error = pwm_set_polarity(servo_motor->pwm, PWM_POLARITY_NORMAL);   
   if(error)
   {
   	dev_err(dev,"Failed to config polarity PWM: %d\n", error);
   	return error;
   }
   */
   platform_set_drvdata(pdev,servo_motor);
   
   /*
   INIT_WORK(&servo_motor->work, pwm_servo_motor_work);
   
   servo_motor->input = devm_input_allocate_device(dev);
   if(!servo_motor->input)
   {
   	dev_err(dev, "Failed to allocate input device\n");
   	return -ENOMEM;
   }
   
   servo_motor->input->name = ("pwm_servo_motor");
   */
    pr_info("Finish initialization servo device driver\n");
   
   return 0;
}

static const struct of_device_id pwm_servo_motor_match[] = {
	{.compatible = "servo_motor,w_pwm_servo_motor",},
	{},
};
MODULE_DEVICE_TABLE(of, pwm_servo_motor_match);

static struct platform_driver pwm_servo_motor = {
	.probe = pwm_servo_motor_probe,
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
