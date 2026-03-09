#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/spi/spi.h>

#include <linux/gpio/consumer.h>


struct display_st7796s_conf {
    void (*prepare)(void*);
    void (*finish)(void*);
    void (*send_data)(void*);
    void (*send_command)(void*);
    struct gpio_desc *sel_st7796s;
    struct gpio_desc *cmd_st7796s;
};

struct display_st7796s_dev {
    struct display_st7796s_conf *pdata;
    struct spi_device *spi;
    struct mutex lock;
};

void select_assert(void *contex);
void select_deassert(void *contex);

void select_data(void *contex);
void select_command(void *contex);

uint8_t st7796s_TxData(struct display_st7796s_dev *device, uint8_t reg);
uint8_t st7796s_write_reg(struct display_st7796s_dev *device, uint8_t reg, uint8_t *regvalue,  size_t len);
uint8_t st7796s_write_pixel(struct display_st7796s_dev *device, uint16_t *regvalue,  size_t len);