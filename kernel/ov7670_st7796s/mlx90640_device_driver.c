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

#include "mlx90640.h"
/* 
	TF-Luna support up to 400kps
	default adddress 0x10
	&i2c1 i2c1-sda = p9_18, i2c1-scl = p9_17
*/

struct mlx90640_device{
    struct i2c_client *client;
    struct gpio_desc *pclk_gpio;
    struct gpio_desc *hsync_gpio;
    struct gpio_desc *vsync_gpio;
};

static int mlx90640_probe(struct i2c_client *client)
{
    pr_info("Initializing sensor image mlx90640");

    //int major;
    unsigned char data[2];
    unsigned char reg_addr[2];
    int err;
    struct mlx90640_conf *mlx90640_conf_t;
    struct mlx90640_dev *mlx90640_dev_t;
    mlx90640_conf_t = devm_kzalloc(&client->dev,sizeof(*mlx90640_conf_t),GFP_KERNEL);
    if(!mlx90640_conf_t)
        return -ENOMEM;

    mlx90640_dev_t = devm_kzalloc(&client->dev,sizeof(*mlx90640_dev_t),GFP_KERNEL);
    if(!mlx90640_dev_t)
        return -ENOMEM;

    //struct device *device=NULL;

    mlx90640_dev_t->client = client;

    if(!i2c_check_functionality(mlx90640_dev_t->client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
        return -EIO;

    reg_addr[0]=0x80;
    reg_addr[1]=0x00;

    err = mlx90640_RxData(mlx90640_dev_t, reg_addr,  2, data, 2);
    uint16_t status_register = 0x0000;
    status_register |= (data[1] & 0x00) << 8 ;
    status_register |= data[0] & 0x08;

    pr_info("Status Register[%x%X] : %x|%x\n",reg_addr[0],reg_addr[1],data[0],data[1]);

    pr_info("Finish initializing set sensor image mlx90640");

    return 0;
}

static void mlx90640_remove(struct i2c_client *client)
{
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

































