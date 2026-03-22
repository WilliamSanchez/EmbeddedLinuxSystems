#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/string.h>

#include <linux/gpio/consumer.h>

struct mlx90640_conf{
    struct gpio_desc *pclk_gpio;
    struct gpio_desc *hsync_gpio;
    struct gpio_desc *vsync_gpio;
};

struct mlx90640_dev {
    struct mlx90640_conf *pdata;
    struct i2c_client *client;
    struct mutex lock;
};

uint8_t mlx90640_TxData(struct mlx90640_dev *device, uint8_t *reg, size_t add_len, uint8_t *data, size_t tx_len);
uint8_t mlx90640_RxData(struct mlx90640_dev *device, uint8_t *reg, size_t tx_len, uint8_t *data, size_t rx_len);