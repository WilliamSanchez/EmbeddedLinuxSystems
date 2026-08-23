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

#include "mlx90640.h"
/* 
	TF-Luna support up to 400kps
	default adddress 0x10
	&i2c1 i2c1-sda = p9_18, i2c1-scl = p9_17
*/

//name    : frameData; 
//mode    : who can read/write the sys file
//show    : read from file; {read frame data + aux data}
//store   : write to file {set control register}; 
//DEVICE_ATTR(name1, S_IWUSR | S_IRUGO, show1, store1); // asking about there are vailbale data and reset or init the device. 
//DEVICE_ATTR(name2, mode2, show2, store2); // read frame data and aux data, and set control register. 


struct mlx90640_conf *mlx90640_conf_t = NULL;
struct mlx90640_dev *mlx90640_dev_t = NULL;
struct device *device;

struct completion read_done;
struct task_struct *read_data;
static struct class *mlx90640_class = NULL;

//    create a function that read the frame and aux data using completion
static int readData(void *unused)
{
    while (!kthread_should_stop()) {

        // Wait for signal
        wait_for_completion(&read_done);

        if (kthread_should_stop())
            break;

        pr_info("Reading data from IR sensor\n");

        // Do your work here
        mlx90640_init(mlx90640_dev_t);
        // IMPORTANT: reinitialize for next use
        reinit_completion(&read_done);
    }

    return 0;
}

static ssize_t show1(struct device *dev, struct device_attribute *attr, char *buf)
{

    mlx90640_init(mlx90640_dev_t);
    return sprintf(buf,"Get from IR sensor\n");
}

static ssize_t store1(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    unsigned char data[2];
    unsigned char reg_addr[2];
    reg_addr[0]=0x80; reg_addr[1]=0x00; 
    if(strcmp(buf,"status"))
    {   
        pr_info("read status register");
        reg_addr[0]=0x80; reg_addr[1]=0x00;    
    }

    mlx90640_RxData(mlx90640_dev_t, reg_addr,  2, data, 2);
    pr_info("Status Register[%x%X] : %x|%x|%x\n",reg_addr[0],reg_addr[1],data[0],data[1],data[2]);
    pr_info("data  from user %s count %d\n",buf,count);
    return count;
}

static DEVICE_ATTR(mlx90640, S_IWUSR | S_IRUSR, show1, store1); // asking about there are vailbale data and reset or init the device. 

static struct attribute *mlx90640attr[] = {
    &dev_attr_mlx90640.attr,
    NULL,
};

static const struct attribute_group mlx90640_attribute_group = {
    .attrs = mlx90640attr,
};

int mlx90640_open(struct inode *inode, struct file *file)
{
    pr_info("Device file opened return ok if the IR sensor is OK after initialization ..\n");
    return 0;    
}

int mlx90640_release(struct inode *inode, struct file *file)
{
    pr_info("Device file closed ..\n");
    return 0;    
}

ssize_t mlx90640_read(struct file *file, char __user *buf, size_t len, loff_t *f_pos)
{
    pr_info("Reading data\n");
//    if(mutex_lock_killable())
//        return -EINTR;

    int _reg_addr;
    _reg_addr = (int)(*f_pos); 

    //mlx90640_RxData(struct mlx90640_dev *device, uint8_t *reg,  size_t tx_len, uint8_t *data, size_t rx_len)

    if(!completion_done(&read_done))
        complete(&read_done);

    return 0;
}

ssize_t mlx90640_write(struct file *file, const char __user *buf, size_t count, loff_t *off)
{

    pr_info("writing data\n");

    return 0;
}

static struct file_operations fops = 
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

    mlx90640_conf_t = devm_kzalloc(&client->dev,sizeof(*mlx90640_conf_t),GFP_KERNEL);
    if(!mlx90640_conf_t)
        return -ENOMEM;

    mlx90640_conf_t->frameData = kzalloc(MLX90640_PIXEL_NUM,GFP_KERNEL);
    if(!mlx90640_conf_t->frameData)
        return -ENOMEM;

    mlx90640_conf_t->auxData = kzalloc(MLX90640_AUX_NUM,GFP_KERNEL);
    if(!mlx90640_conf_t->auxData)
        return -ENOMEM;

    mlx90640_dev_t = devm_kzalloc(&client->dev,sizeof(*mlx90640_dev_t),GFP_KERNEL);
    if(!mlx90640_dev_t)
        return -ENOMEM;

    mlx90640_dev_t->client = client;

    if(!i2c_check_functionality(mlx90640_dev_t->client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
        return -EIO;

    mdelay(1000);

    mlx90640_init(mlx90640_dev_t);

    mlx90640_dev_t->pdata = mlx90640_conf_t;

    mutex_init(&mlx90640_dev_t->lock);

    alloc_chrdev_region(&mlx90640_dev_t->devt, 0, 1,"IR_SENSOR");
    cdev_init(&mlx90640_dev_t->mlx90640_cdev, &fops);
    mlx90640_dev_t->mlx90640_cdev.owner = THIS_MODULE;
    mlx90640_dev_t->mlx90640_cdev.ops = &fops;
    if((cdev_add(&mlx90640_dev_t->mlx90640_cdev,mlx90640_dev_t->devt,1)) < 0)
    {
        pr_err("Cannot add the device to the system\n");
    }

    mlx90640_class = class_create("mlx90640_class");
    device = device_create(mlx90640_class,NULL,mlx90640_dev_t->devt,NULL,"IR_SENSOR");
    if(IS_ERR(device))
    {
        err = PTR_ERR(device);
        pr_err("[target] Error create a device\n");
        goto fail;
    }


    //  create group of attribubte
    int error = sysfs_create_group(&mlx90640_dev_t->client->dev.kobj, &mlx90640_attribute_group);
    if(error){
        dev_err(&mlx90640_dev_t->client->dev,"sys_create_group() failed %d", error);
        return error;
    }

    //after create the function, create a thread to call the read frame and aux daa
    //Create the kernel thread with name 'mythread'
    read_data = kthread_create(readData, NULL, "WaitThread");
    if (read_data) {
            pr_info("Thread Created successfully\n");
            kthread_bind(read_data, 0); // bind to CPU 0
            wake_up_process(read_data);
    } else {
            pr_err("Thread creation failed\n");
    }

    init_completion(&read_done); 
    i2c_set_clientdata(client, mlx90640_dev_t);

    pr_info("Finish initializing set sensor image mlx90640");

    return 0;

    fail:
        if(mlx90640_dev_t)
            kfree(mlx90640_dev_t);
        if(mlx90640_conf_t)
            kfree(mlx90640_conf_t);
        return err;
}

static void mlx90640_remove(struct i2c_client *client)
{
    kthread_stop(read_data);

    // Wake it if it's sleeping
    complete(&read_done);
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

module_i2c_driver(mlx90640_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("William Sanchez");
MODULE_DESCRIPTION("mlx90640 device driver ");

































