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
#include <linux/delay.h>
#include <linux/string.h>

#include <linux/of_gpio.h>

#include "st7796s.h"
#include "images.h"

#define CS_ST9776S_PIN (39)
#define DEFAULT_FREQ    ( 6000000 )

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

static int st7796s_display_probe(struct spi_device *spi)
{
    pr_info("Initializing display st7796s");

    int err;
    struct display_st7796s_conf *st7796s_conf;
    struct display_st7796s_dev *st7796s_dev;
    st7796s_conf = devm_kzalloc(&spi->dev,sizeof(*st7796s_conf),GFP_KERNEL);
    if(!st7796s_conf)
        return -ENOMEM;

    st7796s_dev = devm_kzalloc(&spi->dev,sizeof(*st7796s_dev),GFP_KERNEL);
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
        gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 0);


        st7796s_conf->cmd_st7796s = devm_gpiod_get_optional(&spi->dev,"command",GPIOD_OUT_LOW);
        if(st7796s_conf->cmd_st7796s < 0)
        {
            pr_err("Error gpio request %d", CS_ST9776S_PIN);
            return -1;
        }

        st7796s_conf->send_command = select_command;
        st7796s_conf->send_data = select_data;
        if( gpiod_direction_output(st7796s_conf->cmd_st7796s ,1) < 0)
        {
            pr_err("Error gpio set direction request %d", CS_ST9776S_PIN);
            return -1;
        }
    }


    struct spi_delay transfer_word_delay = { 
            .unit = SPI_DELAY_UNIT_NSECS, 
            .value = 2,
        };

    spi->dev.platform_data = st7796s_conf;

    spi->max_speed_hz = min(spi->max_speed_hz,(u32)DEFAULT_FREQ);
    spi->bits_per_word = 8;
    spi->mode = SPI_MODE_0;
    spi->rt = true;
    spi->word_delay = transfer_word_delay;
    spi->cs_setup = transfer_word_delay;
    spi->cs_hold = transfer_word_delay;

    st7796s_dev->spi = spi;
    st7796s_dev->pdata = st7796s_conf;

    err = spi_setup(st7796s_dev->spi);
    if(err)
    {
        pr_info("not setup spi, error %d",err);
        return err;
    }

    pr_info("Setup SPIO is OK");
    pr_info("SPI cs :%d",spi->chip_select);
    pr_info("SPI max speed hz %d", spi->max_speed_hz);

    /**************************************************************************
     SPI transfers always write the same number of bytes as they read. Protocol 
     drivers should always provide rx_buf and/or tx_buf. In some cases, they may 
     also want to provide DMA addresses for the data being transferred; that may 
     reduce overhead, when the underlying driver uses DMA.
    
     If the transmit buffer is NULL, zeroes will be shifted out while filling rx_buf. 
     If the receive buffer is NULL, the data shifted in will be discarded. Only “len” 
     bytes shift out (or in). 
    **************************************************************************/

    /**************************************************************************
     A spi_message is used to execute an atomic sequence of data transfers, 
     each represented by a struct spi_transfer. The sequence is “atomic” in 
     the sense that no other spi_message may use that SPI bus until that sequence 
     completes.
    ***************************************************************************/

    //struct spi_message m;     

    /*  spi_write:    
        writes the buffer buf. Callable only from contexts that can sleep. 

        spi_sync_transfer:
        Does a synchronous SPI data transfer of the given spi_transfer array
        according on spi_sync.

        spi_sync:
        This call may only be used from a context that may sleep. The sleep is 
        non-interruptible, and has no timeout. Low-overhead controller drivers 
        may DMA directly into and out of the message buffers.

    */  


    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////

    const uint16_t Horizontal_line = 320; //320
    const uint16_t Vertical_line = 480;//480;
    const uint32_t numPixelbits =   2*Horizontal_line*Vertical_line;

    uint8_t YMLBs = (Vertical_line >> 8) & 0xFF;
    uint8_t YLSBs = Vertical_line & 0xFF;
    uint8_t XMLBs = (Horizontal_line >> 8) & 0xFF;
    uint8_t XLSBs = Horizontal_line & 0xFF;

    u8 cmd, data;

    st7796s_dev->pdata->prepare(st7796s_dev);

    /////////////////////////////////////////////////////////////////////////////////////////

    cmd = 0x01;
    err = st7796s_TxData(st7796s_dev,cmd);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }
    msleep(150);
    /////////////////////////////////////////////////////////////////////////////////////////

    cmd = 0xF0;
    data = 0xC3; 
    err = st7796s_write_reg(st7796s_dev, cmd, &data, 1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }
    /////////////////////////////////////////////////////////////////////////////////////////

    cmd = 0xF0;
    data = 0x96; 
    err = st7796s_write_reg(st7796s_dev, cmd, &data, 1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }
    /////////////////////////////////////////////////////////////////////////////////////////
    
    cmd = 0xC5;
    data = 0x1C; 
    err = st7796s_write_reg(st7796s_dev, cmd, &data, 1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }
     /////////////////////////////////////////////////////////////////////////////////////////

    cmd = 0x3A;
    data = 0x55; 
    err = st7796s_write_reg(st7796s_dev, cmd, &data, 1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }
    /////////////////////////////////////////////////////////////////////////////////////////

    cmd = 0x36;
    data = 0x88; //0x88; 
    err = st7796s_write_reg(st7796s_dev, cmd, &data, 1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }
    /////////////////////////////////////////////////////////////////////////////////////////

    cmd = 0xB0;
    data = 0x80; 
    err = st7796s_write_reg(st7796s_dev, cmd, &data, 1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }
    /////////////////////////////////////////////////////////////////////////////////////////

    cmd = 0xB4;
    data = 0x00; 
    err = st7796s_write_reg(st7796s_dev, cmd, &data, 1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }
    /////////////////////////////////////////////////////////////////////////////////////////

    cmd = 0xB6;
    const char data_reg[] = {0x80, 0x02, 0x3B};
    err = st7796s_write_reg(st7796s_dev, cmd, (uint8_t *)data_reg, 3);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }   
    /////////////////////////////////////////////////////////////////////////////////////////

    cmd = 0xB7;
    data = 0xC6; 
    err = st7796s_write_reg(st7796s_dev, cmd, &data, 1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }
    /////////////////////////////////////////////////////////////////////////////////////////

    cmd = 0xF0;
    data = 0x69; 
    err = st7796s_write_reg(st7796s_dev, cmd, &data, 1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }
    /////////////////////////////////////////////////////////////////////////////////////////

    cmd = 0xF0;
    data = 0x3C; 
    err = st7796s_write_reg(st7796s_dev, cmd, &data, 1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }
    /////////////////////////////////////////////////////////////////////////////////////////

    cmd = 0x2A;
    const char data_col[] = {0x00, 0x00, XMLBs, XLSBs};
    err = st7796s_write_reg(st7796s_dev, cmd, (uint8_t*)data_col, 4);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }
    ///////////////////////////////////////////////////////////////////////////////////////// 

    cmd = 0x2B;
    const char data_row[] = {0x00, 0x00, YMLBs, YLSBs};
    err = st7796s_write_reg(st7796s_dev, cmd, (uint8_t *)data_row, 4);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }
    /////////////////////////////////////////////////////////////////////////////////////////

    cmd = 0x11;
    err = st7796s_TxData(st7796s_dev,cmd);

    msleep(150);

    cmd = 0x29;
    err = st7796s_TxData(st7796s_dev,cmd);
    /////////////////////////////////////////////////////////////////////////////////////////
  
    //gpiod_set_value_cansleep(st7796s_conf->sel_st7796s, 0);
    //mutex_unlock(&lock_spi);
    st7796s_dev->pdata->finish(st7796s_dev);

    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////

    msleep(150);

    /////////////////////////////////////////////////////////////////////////////////////////

    st7796s_dev->pdata->prepare(st7796s_dev);

    uint16_t color = 0x05C6;
    uint16_t *pixel = kmalloc(numPixelbits, GFP_KERNEL);
    
    memset(pixel, color, numPixelbits);    
    st7796s_write_pixel(st7796s_dev, pixel, numPixelbits);
    kfree(pixel);

    msleep(250);

    color = 0xFDCF;
    pixel = kmalloc(numPixelbits, GFP_KERNEL);
    memset(pixel, color, numPixelbits);
    st7796s_write_pixel(st7796s_dev, pixel, numPixelbits);
    kfree(pixel);

    msleep(250);

    color = 0x001F;
    pixel = kmalloc(numPixelbits, GFP_KERNEL);
    memset(pixel, color, numPixelbits);
    st7796s_write_pixel(st7796s_dev, pixel, numPixelbits);
    kfree(pixel);

    msleep(250);

    uint16_t *_pixel = kmalloc(2*Horizontal*Vertical, GFP_KERNEL);
    sunset(_pixel);
    st7796s_write_pixel(st7796s_dev, _pixel, numPixelbits);
    kfree(_pixel);

    msleep(250);

    uint16_t *_william = kmalloc(2*Horizontal*Vertical, GFP_KERNEL);
    william(_william);
    st7796s_write_pixel(st7796s_dev, _william, numPixelbits);
    kfree(_william);

    st7796s_dev->pdata->finish(st7796s_dev);

//mutex_unlock(&lock_spi);

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////



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
