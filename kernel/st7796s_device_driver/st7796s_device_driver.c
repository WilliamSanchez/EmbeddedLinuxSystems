#include <linux/delay.h>
#include <linux/device.h>
#include <linux/gpio/consumer.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>

#include <linux/init.h>
#include <linux/printk.h>

#include <linux/of_gpio.h>

#include "st7796s_device_driver.h"

#define ST9776S_DC_PIN          (  39 )   // Data/Command pin is GPIO 39
#define CS_ST9776S_PIN          (  113 )   // Data/Command pin is GPIO 39

static unsigned sel_command;

static const struct of_device_id st7796s_display_of_table[] = {
    {.compatible = "display,st9776s"},
    {},
}; 
MODULE_DEVICE_TABLE(of, st7796s_display_of_table);

static struct spi_device_id spi_st7796s [] = {
	{"st9776s", 0},
	{ },
};
MODULE_DEVICE_TABLE(spi, spi_st7796s);

struct display_st7796s_dev {
    struct spi_device *spi;
    struct mutex lock;
    int addrlen;
    int size;
};

static void select_assert(void *context)
{
    //gpiod_set_value_cansleep(,1)
}

static void select_deassert(void *context)
{
    //gpiod_set_value_cansleep(,0)
}

static int st7796s_display_probe(struct spi_device *spi)
{
    pr_info("Initializing display st7796s");

    struct gpio_desc *sel_st7796s;
    struct gpio_desc *cmd_st7796s;
    struct display_st7796s_conf *st7796s_conf;
    struct display_st7796s_dev *edev;

    if(spi->dev.of_node){   

        const struct of_device_id *of_id = of_match_device(st7796s_display_of_table,&spi->dev);
        struct device_node *np = spi->dev.of_node;
   
        sel_st7796s = devm_gpiod_get_optional(&spi->dev,"cs",GPIOD_OUT_LOW);
        if(sel_st7796s < 0)
        {
            pr_err("Error gpio request %d", CS_ST9776S_PIN);
            return -1;
        };

        if( gpiod_direction_output(sel_st7796s ,0) < 0)
        {
            pr_err("Error gpio set direction request %d", CS_ST9776S_PIN);
            return -1;
        }
        gpiod_set_value_cansleep(sel_st7796s, 0);

        cmd_st7796s = devm_gpiod_get_optional(&spi->dev,"command",GPIOD_OUT_LOW);
        if(cmd_st7796s < 0)
        {
            pr_err("Error gpio request %d", CS_ST9776S_PIN);
            return -1;
        };

        if( gpiod_direction_output(cmd_st7796s ,0) < 0)
        {
            pr_err("Error gpio set direction request %d", CS_ST9776S_PIN);
            return -1;
        }
        gpiod_set_value_cansleep(cmd_st7796s, 0);
    }    


    pr_info("Finish initializing display st7796s");

    return 0;
}

static void st7796s_display_remove(struct spi_device *spi)
{
     pr_info("Finish display st7796s");
}


static struct spi_driver st7796s_driver =
{
    .probe = st7796s_display_probe,
    .id_table = spi_st7796s,
    .remove = st7796s_display_remove,
    .driver = {
        .name = "Display-ST9776S",
        .of_match_table = of_match_ptr(st7796s_display_of_table),
    },
};

module_spi_driver(st7796s_driver);

/*

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
*/
MODULE_AUTHOR("William Sanchez");
MODULE_DESCRIPTION("Display, ST7796S");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:display-st7796s");
