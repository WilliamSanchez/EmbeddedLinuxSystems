#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/kthread.h>
#include <linux/cdev.h>
#include <linux/gpio/consumer.h>

//      DEFINE
#define MLX90640_EEPROM_START_ADDRESS 0x2400
#define MLX90640_EEPROM_DUMP_NUM 832
#define MLX90640_PIXEL_DATA_START_ADDRESS 0x0400
#define MLX90640_PIXEL_NUM 768
#define MLX90640_LINE_NUM 24
#define MLX90640_COLUMN_NUM 32
#define MLX90640_LINE_SIZE 32
#define MLX90640_COLUMN_SIZE 24
#define MLX90640_AUX_DATA_START_ADDRESS 0x0700
#define MLX90640_AUX_NUM 64
#define MLX90640_STATUS_REG 0x8000
#define MLX90640_INIT_STATUS_VALUE 0x0030 


struct mlx90640_conf{
    uint16_t *frameData;
    uint16_t *auxData;
};

struct mlx90640_dev {
    struct mlx90640_conf *pdata;
    struct i2c_client *client;
    struct work_struct work;
    struct mutex lock;
    struct cdev mlx90640_cdev;
    dev_t devt;
};

uint8_t mlx90640_init(struct mlx90640_dev *device);
uint8_t mlx90640_TxData(struct mlx90640_dev *device, uint8_t *reg, size_t add_len, uint8_t *data, size_t tx_len);
uint8_t mlx90640_RxData(struct mlx90640_dev *device, uint8_t *reg, size_t tx_len, uint8_t *data, size_t rx_len);