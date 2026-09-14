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
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/completion.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/gpio/consumer.h>
#include <linux/of_gpio.h>
#include <linux/of.h>

#define DEVICE_NAME  "IMAGE_IR"

struct mlx90640_dev {
    struct i2c_client *client;
    struct work_struct work;
    struct mutex lock;
    struct cdev mlx90640_cdev;
    dev_t devt;
};

struct mlx90640_dev *mlx90640_dev_t = NULL;
struct device *device;

struct completion read_done;
struct task_struct *read_data;
static struct class *mlx90640_class = NULL;
static int major;

//    create a function that read the frame and aux data using completion


static int mlx90640_open(struct inode *inode, struct file *file)
{
    pr_info("Device file opened return ok if the IR sensor is OK after initialization ..\n");
    return 0;    
}

static int mlx90640_release(struct inode *inode, struct file *file)
{
    pr_info("Device file closed ..\n");
    return 0;    
}

ssize_t mlx90640_read(struct file *file, char __user *buf, size_t len, loff_t *f_pos)
{
    pr_info("Reading data\n");
    unsigned char data_readed[2];
    size_t retval = 0;
    struct i2c_msg msg[2];

    u8 reg_addr[2];
    
    reg_addr[0]=0x80;
    reg_addr[1]=0x00;    
    msg[0].addr = mlx90640_dev_t->client->addr;
    msg[0].flags = 0;
    msg[0].len = 2;
    msg[0].buf = reg_addr; 
    
    msg[1].addr = mlx90640_dev_t->client->addr;
    msg[1].flags = I2C_M_RD;
    msg[1].len = 2;
    msg[1].buf = data_readed; 
   

    if(i2c_transfer(mlx90640_dev_t->client->adapter,msg,2) < 0)
    	pr_err("lidar: i2ctransfer failed\n");
    
    pr_info("mlx90640 address[%x] ADDR: %x%X {%x,%x}\n",mlx90640_dev_t->client->addr,reg_addr[0], reg_addr[1], data_readed[0],data_readed[1]);
    	
    if(copy_to_user(buf, data_readed, 2)!=0){
    	pr_err("lidar: transfer data tu user failed\n");
	    goto end_read;
    }

    //  mutex_unlock(&lidar->lidar_mutex);    
    end_read:
    //	mutex_unlock(&lidar->lidar_mutex);
    return retval;
}

ssize_t mlx90640_write(struct file *file, const char __user *buf, size_t count, loff_t *off)
{

    pr_info("writing data\n");

    return 0;
}

static struct file_operations mlx90640_fops = 
{
    .owner      = THIS_MODULE,
    .open       = mlx90640_open,
    .read       = mlx90640_read,
    .write      = mlx90640_write,
    .release    = mlx90640_release,
};

static int mlx90640_probe(struct i2c_client *client)
{
    pr_info("Initializing sensor image mlx90640");    

    int err;

    mlx90640_dev_t = devm_kzalloc(&client->dev,sizeof(*mlx90640_dev_t),GFP_KERNEL);
    if(!mlx90640_dev_t)
        return -ENOMEM;

    mlx90640_dev_t->client = client;
    if(!i2c_check_functionality(mlx90640_dev_t->client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
        return -EIO;

    mdelay(1000);

    u8 reg_init[2];
    struct i2c_msg msg_init[2];
    unsigned char DeviceID[2];

    reg_init[0] = 0x24;
    reg_init[1] = 0x07;

    msg_init[0].addr = mlx90640_dev_t->client->addr;
    msg_init[0].flags = 0;
    msg_init[0].len = 2;
    msg_init[0].buf = reg_init;

    msg_init[1].addr = mlx90640_dev_t->client->addr;
    msg_init[1].flags = I2C_M_RD;
    msg_init[1].len = 2;
    msg_init[1].buf = DeviceID;

    if(i2c_transfer(mlx90640_dev_t->client->adapter, msg_init, 2) < 0)
    {
    	pr_err("mlx90640: i2c transfer failed\n");
    	return -ENODEV;
    }

    pr_info("mlx90640 address[%x] ADDR: %x%X {%x,%x}\n",mlx90640_dev_t->client->addr,reg_init[0], reg_init[1], DeviceID[0],DeviceID[1]);

    reg_init[0] = 0x24;
    reg_init[1] = 0x08;

    msg_init[0].addr = mlx90640_dev_t->client->addr;
    msg_init[0].flags = 0;
    msg_init[0].len = 2;
    msg_init[0].buf = reg_init;

    msg_init[1].addr = mlx90640_dev_t->client->addr;
    msg_init[1].flags = I2C_M_RD;
    msg_init[1].len = 2;
    msg_init[1].buf = DeviceID;

    if(i2c_transfer(mlx90640_dev_t->client->adapter, msg_init, 2) < 0)
    {
    	pr_err("mlx90640: i2c transfer failed\n");
    	return -ENODEV;
    }

    pr_info("mlx90640 address[%x] ADDR: %x%X {%x,%x}\n",mlx90640_dev_t->client->addr,reg_init[0], reg_init[1], DeviceID[0],DeviceID[1]);

    reg_init[0] = 0x24;
    reg_init[1] = 0x09;

    msg_init[0].addr = mlx90640_dev_t->client->addr;
    msg_init[0].flags = 0;
    msg_init[0].len = 2;
    msg_init[0].buf = reg_init;

    msg_init[1].addr = mlx90640_dev_t->client->addr;
    msg_init[1].flags = I2C_M_RD;
    msg_init[1].len = 2;
    msg_init[1].buf = DeviceID;

    if(i2c_transfer(mlx90640_dev_t->client->adapter, msg_init, 2) < 0)
    {
    	pr_err("mlx90640: i2c transfer failed\n");
    	return -ENODEV;
    }

    pr_info("mlx90640 address[%x] ADDR: %x%X {%x,%x}\n",mlx90640_dev_t->client->addr,reg_init[0], reg_init[1], DeviceID[0],DeviceID[1]);

    major = register_chrdev(0, "mlx90640", &mlx90640_fops);
    if(major < 0)
    {
    	pr_err("[target] register chrdv() failed\n");
    	return major;
    }
    

     mutex_init(&mlx90640_dev_t->lock);

    device = device_create(mlx90640_class,NULL,MKDEV(major,0),NULL,DEVICE_NAME);
    if(IS_ERR(device)){
        err = PTR_ERR(device);
        pr_err("Error creatind device create");  
        goto fail;     
    }
 
    mutex_lock(&mlx90640_dev_t->lock);
    i2c_set_clientdata(mlx90640_dev_t->client, mlx90640_dev_t);
    mutex_unlock(&mlx90640_dev_t->lock);

    pr_info("Finish initializing set sensor image mlx90640");

    return 0;

    fail:
        if(mlx90640_dev_t)
            kfree(mlx90640_dev_t);
        return err;
}

static void mlx90640_remove(struct i2c_client *client)
{
    kthread_stop(read_data);
    kfree(mlx90640_dev_t);
    pr_info("Removing device driver\n");
}

static const struct i2c_device_id i2c_mlx90640_id[] ={
    {"mlx90640",0},
    {},
};

MODULE_DEVICE_TABLE(i2c,i2c_mlx90640_id);

static const struct of_device_id mlx90640_id[]={
    {.compatible="mlx90640,infrared_image"},
    {},
};

MODULE_DEVICE_TABLE(of,mlx90640_id);

static struct i2c_driver mlx90640_driver = {
    .driver={
        .owner          =THIS_MODULE,
        .name           ="mlx90640",
        .of_match_table =of_match_ptr(mlx90640_id),
    },
    .probe      = mlx90640_probe,
    .remove     = mlx90640_remove,
    .id_table   = i2c_mlx90640_id,
};

static int __init mlx90640_init(void)
{
    int status;
    mlx90640_class = class_create("mlx90640");
    if(IS_ERR(mlx90640_class))
        return PTR_ERR(mlx90640_class);

    status = i2c_register_driver(THIS_MODULE,&mlx90640_driver);
    if(status < 0)
        class_destroy(mlx90640_class);

    return status;
}

static void __exit mlx90640_exit(void)
{
    i2c_del_driver(&mlx90640_driver);
    class_destroy(mlx90640_class);
}

module_init(mlx90640_init);
module_exit(mlx90640_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("William Sanchez");
MODULE_DESCRIPTION("mlx90640 device driver ");

































