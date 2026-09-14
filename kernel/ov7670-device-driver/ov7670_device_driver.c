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

#include <linux/interrupt.h>
#include <linux/gpio/consumer.h>
#include <linux/of_gpio.h>
#include <linux/of.h>

/* 
	TF-Luna support up to 400kps
	default adddress 0x10
	&i2c1 i2c1-sda = p9_18, i2c1-scl = p9_17
*/

struct ov7670_device{
    struct i2c_client *client;
    struct gpio_desc *pclk_gpio;
    struct gpio_desc *hsync_gpio;
    struct gpio_desc *vsync_gpio;
};

static int ov7670_probe(struct i2c_client *client)
{
    pr_info("Initializing sensor image ov7670");

    //int major;
    unsigned char data[2];
    u8 reg_addr[2];
    struct i2c_msg msg[2];
    //int err;

    struct ov7670_device *ov7670=NULL;
    //struct device *device=NULL;

    if(!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
        return -EIO;

    ov7670 = devm_kzalloc(&client->dev, sizeof(*ov7670),GFP_KERNEL);
    if(!ov7670)
        return -ENOMEM;
    
    //ov7670 ID number
    //0x0A PID        => 76 MSB
    //0x0B VER        => 73 LSB

    reg_addr[0]=0x0A;
    reg_addr[1]=0x0B;

    msg[0].addr = client->addr;
    msg[0].flags=0;         //  write
    msg[0].flags=0;         //  write
    msg[0].len=1;           //  Address is 2 byte coded
    msg[0].buf=reg_addr;

    msg[1].addr = client->addr;
    msg[1].flags= I2C_M_RD | I2C_M_NOSTART;
    msg[1].len=2;
    msg[1].buf=data;

    if(i2c_transfer(client->adapter,msg,2)<0)
    {
        pr_err("ov7670 [%x]: i2c transfer failed\n",client->addr);
        return -ENODEV;
    }

    pr_info("PID: %x, VER %x\n",data[0],data[1]);

    pr_info("Finish initializing set sensor image ov7670");

    return 0;
}

static void ov7670_remove(struct i2c_client *client)
{
    pr_info("Removing device driver\n");
}

static const struct i2c_device_id i2c_ov7670_id[] ={
    {"ov7670",0},
    {},
};

MODULE_DEVICE_TABLE(i2c,i2c_ov7670_id);

static const struct of_device_id ov7670_id[]={
    {.compatible="ov7670,image_sensor"},
    {},
};

MODULE_DEVICE_TABLE(of,ov7670_id);

static struct i2c_driver ov7670_driver = {
    .driver={
        .owner          =THIS_MODULE,
        .name           ="ov7670",
        .of_match_table =of_match_ptr(ov7670_id),
    },
    .probe      = ov7670_probe,
    .remove     = ov7670_remove,
    .id_table   = i2c_ov7670_id,
};

module_i2c_driver(ov7670_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("William Sanchez");
MODULE_DESCRIPTION("ov7670 device driver ");

































