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

#define CS_ST9776S_PIN (39)
#define DEFAULT_FREQ    ( 1000000 )

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
    struct display_st7796s_conf *pdata;
    struct spi_device *spi;
    struct mutex lock;
    int addrlen;
    int size;
};

static void select_assert(void *context)
{
    struct display_st7796s_dev *edev = context;
    gpiod_set_value_cansleep(edev->pdata->sel_st7796s,1);
}

static void select_deassert(void *context)
{
    struct display_st7796s_dev *edev = context;
    gpiod_set_value_cansleep(edev->pdata->sel_st7796s,0);
}

static int st7796s_display_probe(struct spi_device *spi)
{
    pr_info("Initializing display st7796s");

    struct display_st7796s_conf *st7796s_conf;
    st7796s_conf = devm_kzalloc(&spi->dev,sizeof(*st7796s_conf),GFP_KERNEL);

    int err;

    if(!st7796s_conf)
        return -ENOMEM;

    if(spi->dev.of_node)
    {        

        st7796s_conf->sel_st7796s = devm_gpiod_get_optional(&spi->dev,"cs",GPIOD_OUT_LOW);
        if(st7796s_conf->sel_st7796s < 0)
        {
            pr_err("Error gpio request %d", CS_ST9776S_PIN);
            return -1;
        };

        st7796s_conf->prepare = select_assert;
        st7796s_conf->finish = select_deassert;
        if( gpiod_direction_output(st7796s_conf->sel_st7796s ,0) < 0)
        {
            pr_err("Error gpio set direction request %d", CS_ST9776S_PIN);
            return -1;
        }
        gpiod_set_value_cansleep(st7796s_conf->sel_st7796s, 0);

        st7796s_conf->cmd_st7796s = devm_gpiod_get_optional(&spi->dev,"command",GPIOD_OUT_LOW);
        if(st7796s_conf->cmd_st7796s < 0)
        {
            pr_err("Error gpio request %d", CS_ST9776S_PIN);
            return -1;
        }

        if( gpiod_direction_output(st7796s_conf->cmd_st7796s ,0) < 0)
        {
            pr_err("Error gpio set direction request %d", CS_ST9776S_PIN);
            return -1;
        }
        gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 0);
    }

    spi->dev.platform_data = st7796s_conf;

    spi->max_speed_hz = min(spi->max_speed_hz,(u32)DEFAULT_FREQ);
    spi->bits_per_word = 8;
    spi->mode = SPI_MODE_0;
    spi->rt = true;

    err = spi_setup(spi);
    if(err)
    {
        pr_info("not setup spi, error %d",err);
        return err;
    }

    pr_info("Setup SPIO is OK");
    pr_info("SPI cs :%d",spi->chip_select);
    pr_info("SPI max speed hz %d", spi->max_speed_hz);

    spi->dev.platform_data = st7796s_conf;

    struct spi_transfer t[2];
    struct spi_message m;
    spi_message_init(&m);

    memset(&t, 0, sizeof(t));    

    u16 cmd = 0xAC;
    u8 id = 0x00;

    t[0].tx_buf = &cmd;
    t[0].len = sizeof(cmd);

    spi_message_add_tail(&t[0], &m);

    t[1].rx_buf = &id;
    t[1].len = 1;

    spi_message_add_tail(&t[1], &m);

    err = spi_sync(spi,&m);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    pr_info("Finish initializing set display st7796s");

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

MODULE_AUTHOR("William Sanchez");
MODULE_DESCRIPTION("Display, ST7796S");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:display-st7796s");
