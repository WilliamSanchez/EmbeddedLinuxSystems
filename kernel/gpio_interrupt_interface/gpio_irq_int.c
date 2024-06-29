#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/version.h>

#define SIGETX 		44
#define DEVICE_NAME 	"w_gpio_irq"

/*	Global variable and defines for usersapce app registrartion	*/
#define REGISTER_UAPP _IO('R','g')
#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)

static unsigned int gpio_led, gpio_btn;
static unsigned int irq;
static struct task_struct *task = NULL;

dev_t dev = 0;
static struct class *dev_class = NULL;
static struct device *gpioIrq_dev=NULL;


static irqreturn_t btn_irq(int irq, void *dev_id)
{
   struct siginfo info;
     
   pr_info("Reporting an interrupt state %d\n",gpio_get_value(gpio_btn));

   if(task != NULL){
   	memset(&info,0,sizeof(info));
   	info.si_signo = SIGETX;
   	info.si_code=SI_QUEUE;
   	if(send_sig_info(SIGETX,(struct kernel_siginfo *) &info, (struct task_struct *) task) < 0)
   	    pr_err("gpio error sending signal!\n");
   	pr_info("Sending signal successing!\n");
   }

   return IRQ_HANDLED;
}
static long int btn_ioctl(struct file *file, unsigned cmd, unsigned long arg)
{
   int state;
   int32_t answer;
   switch(cmd)
   {	
   	case REGISTER_UAPP:
   		task = get_current();
   		printk("gpio_irq_signal: Userspace app with PID %d is registered\n",irq);
   	     break;
   	case WR_VALUE:
   		if(copy_from_user(&answer, (int32_t *) arg, sizeof(answer))){
   			gpio_set_value(gpio_led, answer); 
			printk("ioctl_get value from user - %d\n",answer);
		}		
   	     break;   		
   	case RD_VALUE:
   	       state = gpio_get_value(gpio_btn); 
   		if(copy_to_user((int32_t *) arg, &answer, sizeof(answer))) 
			printk("ioctl_example - The answer was copied!\n");
	     break;
   	
   }
   
   return 0;
}

static int btn_open(struct inode *device_file, struct file *instance) {
	printk("W_GPIO-IRQ_LED- open was called!\n");
	return 0;
}

static int btn_close(struct inode *device_file, struct file *instance)
{
   if(!task)
   	task=NULL;
   return 0;
}


static struct file_operations fops = {
    .owner = THIS_MODULE,
    .release = btn_close,
    .open = btn_open,
    .unlocked_ioctl = btn_ioctl,
}; 

static const struct of_device_id btn_irq_ids[] = {
   {.compatible="button, input-button",},
   {}
};

static int btn_probe(struct platform_device *pdev)
{
     
    int retval;
    int major;
    struct device_node *np = pdev->dev.of_node;
    pr_info("Initilizing GPIO_IRQ device drive\n");
 

 
    if(!np)
    	return -ENOENT;
    	
    gpio_led=of_get_named_gpio(np,"led-gpios",0);
    gpio_btn=of_get_named_gpio(np,"sw-gpios",0);
    
    pr_info("Gpio button %d\n",gpio_btn);
    pr_info("Gpio led %d\n",gpio_led);

    gpio_request(gpio_led,"led");
    gpio_request(gpio_btn,"button");
    
    gpio_direction_input(gpio_btn);
    gpio_direction_output(gpio_led,1);
    
    irq=gpio_to_irq(gpio_btn);
    retval = request_threaded_irq(irq,NULL,btn_irq, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING|IRQF_ONESHOT, "custom-irq_led gpio",NULL);
    //retval = request_irq(irq,btn_irq, IRQF_TRIGGER_FALLING|IRQF_TRIGGER_RISING, "custom-irq_led gpio",(void *)(btn_irq));
    if(retval != 0)
    {
    	pr_info("Erro configurate GPIO_IRQ device drive %d\n",irq);
    	gpio_free(gpio_btn);gpio_free(gpio_led);
    	return retval;
    }
    
    major = register_chrdev(0, "W_GPIO-LED", &fops);  
    if(major< 0)
    {
    	pr_info("Error, can not register device number!\n");
    	goto fail_chrdev;
    	return -1;
    }
        
   gpioIrq_dev = device_create(dev_class,NULL,MKDEV(major,0),NULL,DEVICE_NAME);
   if(IS_ERR(gpioIrq_dev))
   {
   	retval=PTR_ERR(gpioIrq_dev);
   	pr_err("Cannot create the Device %d",retval);
   	goto fail_chrdev;
   }
    
   pr_info("Finalized GPIO_IRQ device drive\n");
   return 0;
    
   fail_chrdev:
    	gpio_free(gpio_btn);
    	gpio_free(gpio_led);
    	free_irq(irq,NULL);
    	
    return retval;     
}

static int btn_remove(struct platform_device *pdev)
{

   free_irq(irq,NULL);
   gpio_free(gpio_btn);
   gpio_free(gpio_led);
   
   pr_info("End module GPIO-LED IRQ\n");
   return 0;
}

static struct platform_driver gpio_drv={
   .probe = btn_probe,
   .remove = btn_remove,
   .driver={
   	.name = "irq_Wgpio",
   	.of_match_table = of_match_ptr(btn_irq_ids),
   	.owner = THIS_MODULE,
   	},
};

static int __init gpioiq_led_init(void)
{
   int status=0;
 
   printk("Initializing GPI-IRQ_LED module\n"); 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,4,0)
   dev_class  = class_create("WIrqLed");
#else   
   dev_class  = class_create(THIS_MODULE,"WIrqLed");
#endif

   if(IS_ERR(dev_class))
      	return PTR_ERR(dev_class);   
   
   status = platform_driver_register(&gpio_drv);
   if(status < 0)
   	class_destroy(dev_class);  
   
   printk("Initializated lidar module\n");  
   
   return status;

}

static void __exit gpioiq_led_exit(void)
{	
	platform_driver_unregister(&gpio_drv);
	class_destroy(dev_class);
	
	printk("Exit GPIO-IRQ_LED module\n"); 
}

module_init(gpioiq_led_init);
module_exit(gpioiq_led_exit);

MODULE_AUTHOR("William Sanchez");
MODULE_DESCRIPTION("GPIO, Interrupt input");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:GPIO-IRQ input");

