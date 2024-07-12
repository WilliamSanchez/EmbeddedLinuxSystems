#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/pinctrl/consumer.h>
#include <linux/platform_device.h>
#include <linux/pwm.h>

#define SERVO_PWM_PERIOD	20000000
#define SERVO_MAX_DUTY		2550000
#define SERVO_MIN_DUTY		500000
#define SERVO_DEGREE ((SERVO_MAX_DUTY-SERVO_MIN_DUTY)/180)

struct pwm_servo_motor{
   struct pwm_device *pwm;
   unsigned int angle;
   unsigned int angle_duty;
   unsigned int standby;
};

static ssize_t pwm_servo_show_angle(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct pwm_servo_motor *servo_motor = platform_get_drvdata(to_platform_device(dev));
    return sprintf(buf,"%u\n", servo_motor->angle);
}

static int pwm_servo_store_angle(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
   unsigned long tmp;
   int error;
   struct pwm_servo_motor *servo_motor = platform_get_drvdata(to_platform_device(dev));
   
   error=kstrtoul(buf,10,&tmp);
   if(error < 0)
   	return error;
   
   if(tmp<0 || tmp > 180)
   	return -EINVAL;
   
   error= pwm_config(servo_motor->pwm,SERVO_MIN_DUTY+SERVO_DEGREE*tmp, SERVO_PWM_PERIOD);
   if(error < 0)
   	return error;
   
   servo_motor->angle = tmp;
   servo_motor->angle_duty = SERVO_MIN_DUTY+SERVO_DEGREE*tmp;
  
   return count;
}

static ssize_t pwm_servo_show_standby(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct pwm_servo_motor *servo_motor = platform_get_drvdata(to_platform_device(dev));
    return sprintf(buf,"%u\n", servo_motor->standby);
}

static int pwm_servo_store_standby(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
   unsigned long tmp;
   int error;
   struct pwm_servo_motor *servo_motor = platform_get_drvdata(to_platform_device(dev));
   
   error=kstrtoul(buf,10,&tmp);
   if(error < 0)
   	return error;
   
   if(tmp != 0 && tmp != 1)
   	return -EINVAL;
   
   if(tmp == 1 && servo_motor->standby != 1)
   	pwm_disable(servo_motor->pwm);
   else if (tmp == 0 && servo_motor->standby != 0)
   {
   	error = pwm_enable(servo_motor->pwm);
   	if(error)
   		return error;
   }	
   servo_motor->standby = tmp;
  
   return count;
}

static DEVICE_ATTR(angle, S_IWUSR|S_IRUGO, pwm_servo_show_angle, pwm_servo_store_angle);
static DEVICE_ATTR(standby, S_IWUSR|S_IRUGO, pwm_servo_show_standby, pwm_servo_store_standby);

static struct attribute *pwm_servo_attributes[] = {
    &dev_attr_angle.attr,
    &dev_attr_standby.attr,
    NULL,	
};

static const struct attribute_group pwm_servo_group =
{
  .attrs = pwm_servo_attributes,
};

static int pwm_servo_motor_probe(struct platform_device *pdev)
{
   struct device *dev = &pdev->dev;
   struct pinctrl *pinctrl;	
   struct pwm_servo_motor *servo_motor;
   struct device_node *node = dev->of_node;
   int error;
  
  pr_info("Initializing servo device driver\n");
  
   if(node == NULL)
   {
   	dev_err(dev,"Non DT Platfomrs supprted\n");
   	return -EINVAL;
   }
   	
     
   pinctrl = devm_pinctrl_get_select_default(dev);
   if(!servo_motor)
   	dev_warn(dev,"Unable to select pin group\n");
  
   servo_motor = devm_kzalloc(dev,sizeof(*servo_motor), GFP_KERNEL);
   if(!servo_motor){
   	pr_err("Fail to allocate memory\n");
   	return -ENOMEM;
   }
   
   servo_motor->pwm = devm_pwm_get(dev,NULL);
   if(IS_ERR(servo_motor->pwm))
   {
   	error=PTR_ERR(servo_motor->pwm);
   	if(error != -EPROBE_DEFER)
   		dev_err(dev,"Failed to apply initial PWM satte: %d\n",error);
   	return error;
   }
  
   servo_motor->angle=0;
   servo_motor->angle_duty = SERVO_MIN_DUTY;
   servo_motor->standby =1;
  
   error=pwm_config(servo_motor->pwm,servo_motor->angle_duty,SERVO_PWM_PERIOD);
   if(error)
   {
    	dev_err(dev,"Failed to config PWM: %d\n",error);
   	return error;
   }

   platform_set_drvdata(pdev,servo_motor);   
   error=sysfs_create_group(&pdev->dev.kobj, &pwm_servo_group);
   if(error){
       	dev_err(dev,"Failed to create group: %d\n",error);
   	return error;
   }

   pr_info("Finish initialization servo device driver\n");
   
   return 0;
}

static int pwm_servo_motor_remove(struct platform_device *pdev)
{
	struct pwm_servo_motor *servo_motor = platform_get_drvdata(pdev);
	pwm_disable(servo_motor->pwm);
	return 0;
}

static const struct of_device_id pwm_servo_motor_match[] = {
	{.compatible = "servo_motor,w_pwm_servo_motor",},
	{},
};
MODULE_DEVICE_TABLE(of, pwm_servo_motor_match);


static struct platform_driver pwm_servo_motor_driver = {
	.probe = pwm_servo_motor_probe,
	.remove = pwm_servo_motor_remove,
	.driver = {
		.name 	= "pwm-servo_motor",
		.of_match_table = of_match_ptr(pwm_servo_motor_match),
	},
};

module_platform_driver(pwm_servo_motor_driver);

MODULE_AUTHOR("William Sanchez");
MODULE_DESCRIPTION("PWM, Servo Motor");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:pwm-servo_motor");
