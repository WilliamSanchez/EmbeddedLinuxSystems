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

#include "st7796s_device_driver.h"

#define CS_ST9776S_PIN (39)
#define DEFAULT_FREQ    ( 50000000 )

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

    struct mutex lock_spi;

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

        if( gpiod_direction_output(st7796s_conf->cmd_st7796s ,1) < 0)
        {
            pr_err("Error gpio set direction request %d", CS_ST9776S_PIN);
            return -1;
        }
        gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 1);
    }


    /*
    for(int t=0; t< 10; t++){
        gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 1);
        msleep(250);
        gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 0);
        msleep(250);
    }
    */

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

    /**************************************************************************
     SPI transfers always write the same number of bytes as they read. Protocol 
     drivers should always provide rx_buf and/or tx_buf. In some cases, they may 
     also want to provide DMA addresses for the data being transferred; that may 
     reduce overhead, when the underlying driver uses DMA.
    
     If the transmit buffer is NULL, zeroes will be shifted out while filling rx_buf. 
     If the receive buffer is NULL, the data shifted in will be discarded. Only “len” 
     bytes shift out (or in). 
    **************************************************************************/

    struct spi_delay transfer_delay = { 
        .unit = SPI_DELAY_UNIT_NSECS, 
        .value = 1,
    };

    struct spi_transfer t[2];  

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

const uint16_t HORIZONTAL =       320;
const uint16_t VERTICAL   =       2*480;//125;//
const uint32_t numPixelbits =   HORIZONTAL*VERTICAL;

    uint8_t YMLBs = (Vertical_line >> 8) & 0xFF;
    uint8_t YLSBs = Vertical_line & 0xFF;
    uint8_t XMLBs = (Horizontal_line >> 8) & 0xFF;
    uint8_t XLSBs = Horizontal_line & 0xFF;


    u8 cmd, data;

    //mutex_lock(&lock_spi);
    gpiod_set_value_cansleep(st7796s_conf->sel_st7796s, 1);

        /////////////////////////////////////////////////////////////////////////////////////////

    memset(&t, 0, sizeof(t));  

    cmd = 0x01;

    t[0].tx_buf = &cmd;
    t[0].delay = transfer_delay;
    t[0].len = 1;

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 1);

    err = spi_sync_transfer(spi,&t[0],1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }


    /////////////////////////////////////////////////////////////////////////////////////////

    memset(&t, 0, sizeof(t));  

    cmd = 0xF0;
    data = 0xC3; 

    t[0].tx_buf = &cmd;
    t[0].delay = transfer_delay;
    t[0].len = 1;

    t[1].tx_buf = &data;
    t[1].delay = transfer_delay;
    t[1].len = 1;

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 1);

    err = spi_sync_transfer(spi,&t[0],1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 0);

    err = spi_sync_transfer(spi,&t[1],1);
     if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

     /////////////////////////////////////////////////////////////////////////////////////////

    memset(&t, 0, sizeof(t));  

    cmd = 0xF0;
    data = 0x96; 

    t[0].tx_buf = &cmd;
    t[0].delay = transfer_delay;
    t[0].len = 1;

    t[1].tx_buf = &data;
    t[1].delay = transfer_delay;
    t[1].len = 1;

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 1);

    err = spi_sync_transfer(spi,&t[0],1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }
    
    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 0);

    err = spi_sync_transfer(spi,&t[1],1);
     if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    /////////////////////////////////////////////////////////////////////////////////////////

    memset(&t, 0, sizeof(t));  
    cmd = 0xC5;
    data = 0x1C; 

    t[0].tx_buf = &cmd;
    t[0].delay = transfer_delay;
    t[0].len = 1;

    t[1].tx_buf = &data;
    t[1].delay = transfer_delay;
    t[1].len = 1;

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 1);

    err = spi_sync_transfer(spi,&t[0],1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 0);

    err = spi_sync_transfer(spi,&t[1],1);
     if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

     /////////////////////////////////////////////////////////////////////////////////////////

    memset(&t, 0, sizeof(t));  

    cmd = 0x3A;
    data = 0x55; 

    t[0].tx_buf = &cmd;
    t[0].delay = transfer_delay;
    t[0].len = 1;

    t[1].tx_buf = &data;
    t[1].delay = transfer_delay;
    t[1].len = 1;

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 1);

    err = spi_sync_transfer(spi,&t[0],1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 0);

    err = spi_sync_transfer(spi,&t[1],1);
     if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    /////////////////////////////////////////////////////////////////////////////////////////

        memset(&t, 0, sizeof(t));  

    cmd = 0x36;
    data = 0x48; 

    t[0].tx_buf = &cmd;
    t[0].delay = transfer_delay;
    t[0].len = 1;

    t[1].tx_buf = &data;
    t[1].delay = transfer_delay;
    t[1].len = 1;

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 1);

    err = spi_sync_transfer(spi,&t[0],1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 0);

    err = spi_sync_transfer(spi,&t[1],1);
     if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    /////////////////////////////////////////////////////////////////////////////////////////

    memset(&t, 0, sizeof(t));  

    cmd = 0xB0;
    data = 0x80; 

    t[0].tx_buf = &cmd;
    t[0].delay = transfer_delay;
    t[0].len = 1;

    t[1].tx_buf = &data;
    t[1].delay = transfer_delay;
    t[1].len = 1;

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 1);

    err = spi_sync_transfer(spi,&t[0],1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 0);

    err = spi_sync_transfer(spi,&t[1],1);
     if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    /////////////////////////////////////////////////////////////////////////////////////////

    memset(&t, 0, sizeof(t));  

    cmd = 0xB4;
    data = 0x00; 

    t[0].tx_buf = &cmd;
    t[0].delay = transfer_delay;
    t[0].len = 1;

    t[1].tx_buf = &data;
    t[1].delay = transfer_delay;
    t[1].len = 1;

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 1);

    err = spi_sync_transfer(spi,&t[0],1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 0);

    err = spi_sync_transfer(spi,&t[1],1);
     if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    /////////////////////////////////////////////////////////////////////////////////////////

    memset(&t, 0, sizeof(t));  

    cmd = 0xB6;
    const char data_reg[] = {0x80, 0x02, 0x3B};

    t[0].tx_buf = &cmd;
    t[0].delay = transfer_delay;
    t[0].len = 1;

    t[1].tx_buf = data_reg;
    t[1].delay = transfer_delay;
    t[1].len = 3;

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 1);

    err = spi_sync_transfer(spi,&t[0],1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 0);

    err = spi_sync_transfer(spi,&t[1],1);
     if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    
    memset(&t, 0, sizeof(t));  

    cmd = 0xB7;
    data = 0xC6; 

    t[0].tx_buf = &cmd;
    t[0].delay = transfer_delay;
    t[0].len = 1;

    t[1].tx_buf = &data;
    t[1].delay = transfer_delay;
    t[1].len = 1;

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 1);

    err = spi_sync_transfer(spi,&t[0],1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 0);

    err = spi_sync_transfer(spi,&t[1],1);
     if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    /////////////////////////////////////////////////////////////////////////////////////////

    memset(&t, 0, sizeof(t));  

    cmd = 0xF0;
    data = 0x69; 

    t[0].tx_buf = &cmd;
    t[0].delay = transfer_delay;
    t[0].len = 1;

    t[1].tx_buf = &data;
    t[1].delay = transfer_delay;
    t[1].len = 1;

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 1);

    err = spi_sync_transfer(spi,&t[0],1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 0);

    err = spi_sync_transfer(spi,&t[1],1);
     if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    /////////////////////////////////////////////////////////////////////////////////////////

    memset(&t, 0, sizeof(t));  

    cmd = 0xF0;
    data = 0x3C; 

    t[0].tx_buf = &cmd;
    t[0].delay = transfer_delay;
    t[0].len = 1;

    t[1].tx_buf = &data;
    t[1].delay = transfer_delay;
    t[1].len = 1;

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 1);

    err = spi_sync_transfer(spi,&t[0],1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 0);

    err = spi_sync_transfer(spi,&t[1],1);
     if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    memset(&t, 0, sizeof(t));  

    cmd = 0x2A;
    const char data_col[] = {0x00, 0x00, XMLBs, XLSBs};

    t[0].tx_buf = &cmd;
    t[0].delay = transfer_delay;
    t[0].len = 1;

    t[1].tx_buf = data_col;
    t[1].delay = transfer_delay;
    t[1].len = 4;

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 1);

    err = spi_sync_transfer(spi,&t[0],1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 0);

    err = spi_sync_transfer(spi,&t[1],1);
     if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    /////////////////////////////////////////////////////////////////////////////////////////

    memset(&t, 0, sizeof(t));  

    cmd = 0x2B;
    const char data_row[] = {0x00, 0x00, YMLBs, YLSBs};

    t[0].tx_buf = &cmd;
    t[0].delay = transfer_delay;
    t[0].len = 1;

    t[1].tx_buf = data_row;
    t[1].delay = transfer_delay;
    t[1].len = 4;

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 1);

    err = spi_sync_transfer(spi,&t[0],1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 0);

    err = spi_sync_transfer(spi,&t[1],1);
     if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    /////////////////////////////////////////////////////////////////////////////////////////
    memset(&t, 0, sizeof(t));  

    cmd = 0x11;
    data = 0x29; 

    t[0].tx_buf = &cmd;
    t[0].delay = transfer_delay;
    t[0].len = 1;

    t[1].tx_buf = &data;
    t[1].delay = transfer_delay;
    t[1].len = 1;

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 1);

    err = spi_sync_transfer(spi,&t[0],1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    msleep(150);

    err = spi_sync_transfer(spi,&t[1],1);
     if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

  /////////////////////////////////////////////////////////////////////////////////////////

    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 1);
    gpiod_set_value_cansleep(st7796s_conf->sel_st7796s, 0);
    //mutex_unlock(&lock_spi);

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////


    msleep(150);

    /////////////////////////////////////////////////////////////////////////////////////////

    memset(&t, 0, sizeof(t));  

    t[0].tx_buf = &cmd;
    t[0].delay = transfer_delay;
    t[0].len = 1;

    cmd = 0x2C;
    uint16_t color = 0;

    gpiod_set_value_cansleep(st7796s_conf->sel_st7796s, 1);
    gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 1);

    u8 tem=0;
    while (tem < 256 )
    {
        uint16_t *pixel = kmalloc(numPixelbits, GFP_KERNEL);
        memset(pixel, color, numPixelbits);
    
        t[1].tx_buf = pixel;
        t[1].delay = transfer_delay;
        t[1].bits_per_word = 16;
        t[1].len = numPixelbits;

        err = spi_sync_transfer(spi,&t[0],1);
        if(err < 0)
        {
            pr_info("SPI can not transfer data");
            break;
        }

        gpiod_set_value_cansleep(st7796s_conf->cmd_st7796s, 0);

        err = spi_sync_transfer(spi,&t[1],1);
         if(err < 0)
        {
            pr_info("SPI can not transfer data"); kfree(pixel);
            break;
        }
        kfree(pixel);
        tem++;
        color++;
        msleep(150);
    }


    gpiod_set_value_cansleep(st7796s_conf->sel_st7796s, 0);

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
