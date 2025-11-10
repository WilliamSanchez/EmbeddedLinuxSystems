#include <linux/delay.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>

#include "st7796s_device_driver.h"

#define ST9776S_DC_PIN          (  39 )   // Data/Command pin is GPIO 39
/*
struct display_st7796s_dev {
    struct spi_device *spi;
    struct mutex lock;
    int addrlen;
    int size;
};

static const struct of_device_id st7796s_display_of_table[] = {
    {.compatible = "display,display_st9776s"},
    {},
}; 

static int st7796s_display_probe(struct spi_device *spi)
{
    pr_info("Initializing display st7796s");


    struct display_st7796s_conf *st7796s_conf;
    struct display_st7796s_dev *edev;
    int err;

    if (gpio_is_valid(ST9776S_DC_PIN))
    {

    };

    if(gpio_request(ST9776S_DC_PIN,"ST9776S_DC_PIN") < 0)
    {

    };

    gpio_direction_output(ST9776S_DC_PIN,0);

    return 0;
}

//static int st7796s_display_remove(struct spi_device *spi)
//{
//     pr_info("Finish display st7796s");
//    return 0;
//}

static struct spi_driver st7796s_driver =
{
    .driver = {
        .name = "Display-ST9776S",
        .of_match_table = of_match_ptr(st7796s_display_of_table),
    },
    .probe = st7796s_display_probe,
    //.remove = st7796s_display_remove,

};

module_spi_driver(st7796s_driver);
*/


static int __init init_display(void)
{
    pr_info("Initializing display st7796s");
    return 0;
}

static void __init exit_display(void)
{
    pr_info("End used display");
}

module_init(init_display);
module_exit(exit_display);

MODULE_AUTHOR("William Sanchez");
MODULE_DESCRIPTION("Display, ST7796S");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:display-st7796s");
