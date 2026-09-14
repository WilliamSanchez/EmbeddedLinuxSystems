#include "st7796s.h"




void select_assert(void *contex)
{
    struct display_st7796s_dev *device = contex;
    gpiod_set_value_cansleep(device->pdata->sel_st7796s, 1);

}

void select_deassert(void *contex)
{
    struct display_st7796s_dev *device = contex;
    gpiod_set_value_cansleep(device->pdata->sel_st7796s, 0);
}

void select_data(void *contex)
{
    struct display_st7796s_dev *device = contex;
    gpiod_set_value_cansleep(device->pdata->cmd_st7796s, 0);

}

void select_command(void *contex)
{
    struct display_st7796s_dev *device = contex;
    gpiod_set_value_cansleep(device->pdata->cmd_st7796s, 1);
}

uint8_t st7796s_TxData(struct display_st7796s_dev *device, uint8_t reg)
{

    int err;

    struct spi_transfer t; 
    memset(&t, 0, sizeof(t));  

    t.tx_buf = &reg;
    t.len = 1;
    device->pdata->send_command(device);

    err = spi_sync_transfer(device->spi,&t,1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    return 0;

}

uint8_t st7796s_write_reg(struct display_st7796s_dev *device, uint8_t reg, uint8_t *regvalue,  size_t len)
{
    struct spi_delay transfer_delay = { 
        .unit = SPI_DELAY_UNIT_NSECS, 
        .value = 1,
    };

    int err;

    struct spi_transfer t[2]; 
    memset(&t, 0, sizeof(t));  

    t[0].tx_buf = &reg;
    t[0].delay = transfer_delay;
    t[0].len = 1;

    t[1].tx_buf = regvalue;
    t[1].delay = transfer_delay;
    t[1].len = len;

    device->pdata->send_command(device);

    err = spi_sync_transfer(device->spi,&t[0],1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    device->pdata->send_data(device);

    err = spi_sync_transfer(device->spi,&t[1],1);
     if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

   return 0;
}

uint8_t st7796s_write_pixel(struct display_st7796s_dev *device, uint16_t *pixel,  size_t len)
{
    struct spi_delay transfer_delay = { 
        .unit = SPI_DELAY_UNIT_NSECS, 
        .value = 1,
    };

    int err;
    u8 cmd = 0x2C;

    struct spi_transfer t[2]; 
    memset(&t, 0, sizeof(t));  

    t[0].tx_buf = &cmd;
    t[0].delay = transfer_delay;
    t[0].len = 1;

    t[1].tx_buf = pixel;
    t[1].delay = transfer_delay;
    t[1].bits_per_word = 16;
    t[1].len = len;

    device->pdata->send_command(device);

    err = spi_sync_transfer(device->spi,&t[0],1);
    if(err < 0)
    {
        pr_info("SPI can not transfer data");
        return err;
    }

    device->pdata->send_data(device);

    err = spi_sync_transfer(device->spi,&t[1],1);
    if(err < 0)
        {
            pr_info("SPI can not transfer data"); kfree(pixel);
            return err;
        }

    return 0;
}