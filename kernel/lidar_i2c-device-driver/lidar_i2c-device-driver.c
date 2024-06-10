#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/uaccess.h>

/* 
	TF-Luna support up to 400kps
	default adddress 0x10
	&i2c1 i2c1-sda = p9_18, i2c1-scl = p9_17

*/


#define DEVICE_NAME "w_lidar_i2c"

static struct class *lidar_class = NULL;
static struct i2c_client *lidar_client;

enum{
    CDEV_NOT_USED=0,
    CDEV_EXCLUSIVE_OPEN=1,
};

static int major;

static LIST_HEAD(device_list);
static DEFINE_MUTEX(device_list_lock);

struct lidar_dev{
   unsigned char *data;
   struct i2c_client *client;
   struct mutex lidar_mutex;
   struct list_head device_entry;
   dev_t devt;
   unsigned users;
};

static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED);


/*	open	*/

static int lidar_open(struct inode *inode, struct file *file)
{
       
    if(atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN))
    	return -EBUSY;
    
    try_module_get(THIS_MODULE);
    return 0;
}

/*	RELEASE		*/
static int lidar_release(struct inode *inode, struct file *file)
{
    atomic_set(&already_open, CDEV_NOT_USED);
    module_put(THIS_MODULE);
    return 0;
}



/*	READ		*/

ssize_t lidar_read(struct file *filp, char __user *buf, size_t count, loff_t *f_ops)
{

    unsigned char data_readed[5];
    size_t retval = 0;
    struct i2c_msg msg[2];
    struct lidar_dev *lidar = filp->private_data;
    u8 reg_addr[2];
    
      printk("Read registers addr %x\n",lidar_client->addr); 
    
   // if(mutex_lock_killable(&lidar->lidar_mutex))
   // 	return -EINTR;
    
    reg_addr[0]=0x00;
    reg_addr[1]=0x00;
    
    msg[0].addr = 0x10;
    msg[0].flags = 0;
    msg[0].len = 1;
    msg[0].buf = reg_addr; 
    
    msg[1].addr = 0x10;
    msg[1].flags = I2C_M_RD;
    msg[1].len = count;
    msg[1].buf = data_readed; 
   

    if(i2c_transfer(lidar_client->adapter,msg,2) < 0)
    	pr_err("lidar: i2ctransfer failed\n");
    	
    printk("lidar read: data received \n"); 
    
    for(int i = 0; i< 5; i++)
    	printk("lidar read: %x \n", data_readed[i]);
    	
    if(copy_to_user(buf, data_readed, 5)!=0){
    	pr_err("lidar: transfer data tu user failed\n");
	goto end_read;
     }

  //  mutex_unlock(&lidar->lidar_mutex);    	

    end_read:
    //	mutex_unlock(&lidar->lidar_mutex);
    return retval;
}


/*	WRITE		*/

ssize_t lidar_write(struct file *file, const char __user *buf, size_t count, loff_t *f_ops)
{

    size_t retval = 0;
    //struct i2c_msg msg[2];
    struct lidar_dev *lidar = file->private_data;
    //u8 reg_addr[2];
    
    if(mutex_lock_killable(&lidar->lidar_mutex))
    	return -EINTR;

    mutex_unlock(&lidar->lidar_mutex);    	
    return retval;
}

struct file_operations lidar_fops = {
   .owner 	= THIS_MODULE,
   .open 	= lidar_open,
   .release 	= lidar_release,
   .read 	= lidar_read,
   .write 	= lidar_write,
   //.llseek 	= lidar_llseek,	
};

static int lidar_probe(struct i2c_client *client)
{
    
    unsigned char data[5];
    u8 reg_addr[2];
    struct i2c_msg msg[2];
    int err = 0;
    
    struct lidar_dev *lidar = NULL;
    struct device *device = NULL;
    
    pr_info("Lidar: inside of probe function, detecting device addr %x\n",client->addr);
    
    lidar_client = client;
    
    if(!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)){
    	pr_err("Lidar: not defined funcionality\n");
    	return -EIO;
    }
    
    reg_addr[0]=0x00;
    reg_addr[1]=0x00;
    
    msg[0].addr = client->addr;
    msg[0].flags = 0;	//write//
    msg[0].len=1;
    msg[0].buf = reg_addr;
    
    msg[1].addr = client->addr;
    msg[1].flags = I2C_M_RD | I2C_M_NOSTART;
    msg[1].len=5;
    msg[1].buf = data;
    
    if(i2c_transfer(client->adapter, msg, 2) < 0)
    {
    	pr_err("Lidar: i2c transfer failed\n");
    	return -ENODEV;
    }
    
    printk("lidar: data received \n"); 
    
    for(int i = 0; i< 5; i++)
    	printk("lidar: %x \n", data[i]);
    	
    printk("Distance: %d%d cm\n", data[1],data[0]);	
    
    
    lidar = devm_kzalloc(&client->dev, sizeof(*lidar), GFP_KERNEL);
    if(!lidar)
    	return -ENOMEM;
  
  
    major = register_chrdev(0, "lidar", &lidar_fops);
    if(major < 0)
    {
    	pr_err("[target] register chrdv() failed\n");
    	return major;
    }
    
    mutex_init(&lidar->lidar_mutex);
    device = device_create(lidar_class,NULL,MKDEV(major,0),NULL,DEVICE_NAME);
    if(IS_ERR(device)){
        err = PTR_ERR(device);
        pr_err("Error creatind device create");  
        goto fail;     
    }
    
    pr_info("Device created on /dev/%s\n",DEVICE_NAME);
    
    mutex_lock(&lidar->lidar_mutex);
    i2c_set_clientdata(client,lidar);
    mutex_unlock(&lidar->lidar_mutex);
    
    pr_info("Lidar device probed successfully\n");
    
    return 0;
    
    
    fail:
       if(lidar)
          kfree(lidar);
       return err;
}

/*	REMOVE		*/

#if KERNEL_VERSION(5, 16, 0) <= LINUX_VERSION_CODE
   static void lidar_remove(struct i2c_client *client)
#else   
   static int lidar_remove(struct i2c_client *client)
#endif
{
    //struct lidar_dev *lidar = i2c_get_clientdata(client);
    pr_info("Lidar device removed successfully\n");
    device_destroy(lidar_class,MKDEV(major,0));
    #if KERNEL_VERSION(5, 16, 0) > LINUX_VERSION_CODE
    	return 0;
    #endif

}

static const struct of_device_id lidar_i2c_of_table[] = {
	{.compatible="lidar,measure_distance",},
	{},
};
MODULE_DEVICE_TABLE(of,lidar_i2c_of_table);



static const struct i2c_device_id lidar_id[] = {
  	{"lidar-i2c",0},
  	{},
};
MODULE_DEVICE_TABLE(i2c,lidar_id);


static struct i2c_driver tf_luna_i2c_driver = {
   	.driver = {
   		.owner = THIS_MODULE,
   		.name = "lidar-i2c",
   		.of_match_table = of_match_ptr(lidar_i2c_of_table),
   	},
   	.probe = lidar_probe,
   	.remove = lidar_remove,
   	.id_table = lidar_id,
};


static int __init lidar_drv_init(void)
{
   int status=0;
 
   printk("Initializing lidar module\n");  	
      
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,4,0)
   lidar_class = class_create("lidar");
#else   
   lidar_class = class_create(THIS_MODULE,"lidar");
#endif

   if(IS_ERR(lidar_class))
      	return PTR_ERR(lidar_class);   
   
   status = i2c_register_driver(THIS_MODULE,&tf_luna_i2c_driver);
   if(status < 0)
   	class_destroy(lidar_class);  
   
   printk("Initializated lidar module\n");  
   
   return status;

}

static void __exit lidar_drv_exit(void)
{
   i2c_del_driver(&tf_luna_i2c_driver);
   class_destroy(lidar_class);
   
    printk("Exit lidar module\n");  
}

module_init(lidar_drv_init);
module_exit(lidar_drv_exit);

//module_i2c_driver(tf_luna_i2c_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("William Sanchez");
MODULE_DESCRIPTION("driver to reads LIDAR sensor");

































