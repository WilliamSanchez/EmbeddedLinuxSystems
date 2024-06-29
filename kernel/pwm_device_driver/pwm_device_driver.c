#include <linux/input.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/pwm.h>

struct pwm_servo_motor{
   struct input_dev *input;
   struct pwm_device *pwm;
   struct work_struct work;
   unsigned long period;
   bool suspend;
};

static int pwm_servo_motor_on(struct pwm_servo_motor *servo_motor, unsigned long period)
{
   struct pwm_state state;
   int error;
  
   state.enabled = true;
   state.period = period;
   
   pwm_set_relative_duty_cycle(&state, 50, 100);
  
   error = pwm_apply_state(servo_motor->pwm, &state);
   if(error)
  	return error;
  
   return 0;
}

static void pwm_servo_motor_off(struct pwm_servo_motor *servo_motor)
{
	pwm_disable(servo_motor->pwm);
}

static void pwm_servo_motor_work(struct work_struct *work)
{
    struct pwm_servo_motor *servo_motor = container_of(work, struct pwm_servo_motor, work); 
    unsigned long period = READ_ONCE(servo_motor->period);
       
    if(period)
    	pwm_servo_motor_on(servo_motor, period);
    else
    	pwm_servo_motor_off(servo_motor);
}

static void pwm_servo_motor_stop(struct pwm_servo_motor *servo_motor)
{
   cancel_work_sync(&servo_motor->work);
   pwm_servo_motor_off(servo_motor);
}

static int pwm_servo_motor_probe(struct platform_device *pdev)
{
   struct device *dev = &pdev->dev;	
   struct pwm_servo_motor *servo_motor;
   struct pwm_state state;
   int error;
  
   servo_motor = devm_kzalloc(dev,sizeof(*servo_motor), GFP_KERNEL);
   if(!servo_motor)
   	return -ENOMEM;
   
   servo_motor->pwm = devm_pwm_get(dev,NULL);
   if(IS_ERR(servo_motor->pwm))
   {
   	error=PTR_ERR(servo_motor->pwm);
   	if(error != -EPROBE_DEFER)
   		dev_err(dev,"Failed to apply initial PWM satte: %d\n",error);
   	return error;
   }
  
   /* Sync up PWM satte and ensure it is off  */
   pwm_init_state(servo_motor->pwm, &state);
   state.enabled = false;
   error = pwm_apply_state(servo_motor->pwm, &state);
   
   if(error)
   {
   	dev_err(dev,"Failed to apply initial PWM state: %d\n", error);
   	return error;
   }
   
   INIT_WORK(&servo_motor->work, pwm_servo_motor_work);
   
   servo_motor->input = devm_input_allocate_device(dev);
   if(!servo_motor->input)
   {
   	dev_err(dev, "Failed to allocate input device\n");
   	return -ENOMEM;
   }
   
   servo_motor->input->name = ("pwm_servo_motor");
   
   return 0;
}

static int __maybe_unused pwm_servo_motor_suspend(struct device *dev)
{
   struct pwm_servo_motor *servo_motor = dev_get_drvdata(dev);
   
   //spin_lock_irq(&servo_motor->input->event->lock);
   servo_motor->suspend = true;
   //spin_unlock_irq(&servo_motor->input->event->event_lock); 

   pwm_servo_motor_stop(servo_motor);

   return 0;
}

static int __maybe_unused pwm_servo_motor_resume(struct device *dev)
{
   struct pwm_servo_motor *servo_motor = dev_get_drvdata(dev);
  
   //spin_lock_irq(&servo_motor->input->event_lock);
   servo_motor->suspend = false;
   //spin_unlock_irq(&servo_motor->input->event_lock);
  
   schedule_work(&servo_motor->work);
  
   return 0;
}

static SIMPLE_DEV_PM_OPS(pwm_servo_motor_pm_ops, pwm_servo_motor_suspend, pwm_servo_motor_resume);

#ifdef CONFIG_OF
	static const struct of_device_id pwm_servo_motor_match[] = {
		{.compatible = "pwm-servo_motor"},
		{},
	};
MODULE_DEVICE_TABLE(of, pwm_servo_motor_match);
#endif

static struct platform_driver pwm_servo_motor = {
	.probe = pwm_servo_motor_probe,
	.driver = {
		.name 	= "pwm-servo_motor",
		.pm 	= &pwm_servo_motor_pm_ops,
		.of_match_table = of_match_ptr(pwm_servo_motor_match),
	},
};

module_platform_driver(pwm_servo_motor);

MODULE_AUTHOR("William Sanchez");
MODULE_DESCRIPTION("PWM, Servo Motor");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:pwm-servo_motor");
