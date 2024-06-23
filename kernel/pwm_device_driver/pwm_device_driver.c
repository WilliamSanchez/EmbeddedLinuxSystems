#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/pwm.h>
#include <linux/version.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("William Samchez");
MODULE_DESCRIPTION("A PWM device driver to turn a motor server");

#define DRIVER_NAME 	"pwd_device_driver"
#define DRIVER_CLASS	"pwm_class"

static dev_t my_device_nr;
static struct class *pwmClass;
static struct cdev pwmDevice;

struct pwm_device *pwm0 = NULL;
struct device *devicePWM = NULL;
u32 pwm_on_time = 10000000;

static ssize_t pwm_write(struct file *filep, const char *user_buffer, size_t count, loff_t *offs)
{
    int to_copy, not_copied, delta;
    char value;
    
    /* Get amoint of data to copy */
    to_copy = min(count, sizeof(value));
    
    /* Copy data to user */
    not_copied = copy_from_user(&value, user_buffer, to_copy);
    
    /* Set PWM on time */
    if(value < 'a' || value > 'j')
    	printk("Invalid value\n");
    else
    	pwm_config(pwm0, 20000000*(value - 'a'), 20000000);
    	
    delta = to_copy - not_copied;
    	
    return delta;
} 

static int pwm_release(struct inode *device_file, struct file *instance)
{
  printk("Open pwm device driver");
  return 0;
}

static int pwm_open(struct inode *device_file, struct file *instance)
{
  printk("Close pwm device driver");
  return 0;
}

static struct file_operations pwmfops = {
	.owner		= THIS_MODULE,
	.open		= pwm_open,
	.release	= pwm_release,
	.write		= pwm_write
};

static int __init ModuleInit(void)
{
    printk("Initializing the module\n");
    
    /*  allocate a device nr */
    if(alloc_chrdev_region(&my_device_nr, 0, 1, DRIVER_NAME) < 0)
    {
    	printk("Device can't be allocated\n");
    	return -1;
    }
    
    printk("PWM device Major: %d, Minor: %d was registered\n",my_device_nr>>20, my_device_nr && 0xfffff);
    
    /*	create device class	*/
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(6,4,0)
    	if((pwmClass=class_create(DRIVER_CLASS))== NULL)
    #else
    	if((pwmClass=class_create(THIS_MODULE,DRIVER_CLASS))== NULL)
    #endif
    {
        printk("PWM Device class was not created witth success\n");
        goto classError;
    } 
    
    /* Create device file */
    if(device_create(pwmClass, NULL, my_device_nr, NULL, DRIVER_NAME) == NULL)
    {
    	printk("Can not create device file!\n");
    	goto fileError;
    }
    
    /* Initialize device file  */
    cdev_init(&pwmDevice, &pwmfops);
    
    /* Registering device to kernel */
    if(cdev_add(&pwmDevice, my_device_nr, 1) == -1)
    {
    	printk("Registering of device to kernel failed!\n");
    	goto addError;
    }
    
    
    pwm0 = pwm_get(devicePWM, "pwm-server_motor");
    if(pwm0 == NULL)
    {
       printk("Cold not get PWM0!\n");
       goto addError;
    }
    
    pwm_config(pwm0, pwm_on_time, 20000000);
    pwm_enable(pwm0);
    
    printk("PWM module initialized seccesful\n");
    return 0;
    
    addError:
    	device_destroy(pwmClass, my_device_nr);
    fileError:
    	class_destroy(pwmClass);    
    classError:
    	unregister_chrdev_region(my_device_nr,1);
    return -1;

}

static void __exit ModuleExit(void){
	pwm_disable(pwm0);
	pwm_put(pwm0);
	cdev_del(&pwmDevice);
	device_destroy(pwmClass, my_device_nr);
	class_destroy(pwmClass);
	unregister_chrdev_region(my_device_nr,1);
	printk("Exit PWM\n");
}

module_init(ModuleInit);
module_exit(ModuleExit);








