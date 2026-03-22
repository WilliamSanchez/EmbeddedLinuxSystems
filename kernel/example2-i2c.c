#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/gpio/consumer.h>
#include <linux/mutex.h>

#define DRIVER_NAME "example_i2c_transfer"

/* Example registers */
#define REG_STATUS 0x00
#define REG_DATA   0x01

struct example_dev {
    struct i2c_client *client;
    struct gpio_desc *irq_gpio;
    int irq;

    struct mutex lock;
};

/* Low level I2C read using i2c_transfer */
static int example_i2c_read(struct example_dev *dev, u8 reg, u8 *val)
{
    struct i2c_msg msgs[2];
    u8 reg_buf = reg;
    int ret;

    msgs[0].addr  = dev->client->addr;
    msgs[0].flags = 0;
    msgs[0].len   = 1;
    msgs[0].buf   = &reg_buf;

    msgs[1].addr  = dev->client->addr;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len   = 1;
    msgs[1].buf   = val;

    ret = i2c_transfer(dev->client->adapter, msgs, 2);

    if (ret < 0)
        return ret;

    if (ret != 2)
        return -EIO;

    return 0;
}

/* Low level I2C write */
static int example_i2c_write(struct example_dev *dev, u8 reg, u8 val)
{
    u8 buf[2];
    struct i2c_msg msg;

    buf[0] = reg;
    buf[1] = val;

    msg.addr  = dev->client->addr;
    msg.flags = 0;
    msg.len   = sizeof(buf);
    msg.buf   = buf;

    if (i2c_transfer(dev->client->adapter, &msg, 1) != 1)
        return -EIO;

    return 0;
}

/* IRQ thread */
static irqreturn_t example_irq_thread(int irq, void *data)
{
    struct example_dev *dev = data;
    u8 status;
    u8 value;
    int ret;

    mutex_lock(&dev->lock);

    ret = example_i2c_read(dev, REG_STATUS, &status);
    if (ret)
        goto out;

    ret = example_i2c_read(dev, REG_DATA, &value);
    if (ret)
        goto out;

    dev_info(&dev->client->dev,
             "IRQ event: status=%u data=%u\n",
             status, value);

out:
    mutex_unlock(&dev->lock);
    return IRQ_HANDLED;
}

/* Probe */
static int example_probe(struct i2c_client *client)
{
    struct example_dev *dev;
    int ret;

    dev = devm_kzalloc(&client->dev, sizeof(*dev), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;

    dev->client = client;
    mutex_init(&dev->lock);

    /* Get IRQ GPIO */
    dev->irq_gpio = devm_gpiod_get(&client->dev, "irq", GPIOD_IN);
    if (IS_ERR(dev->irq_gpio))
        return PTR_ERR(dev->irq_gpio);

    dev->irq = gpiod_to_irq(dev->irq_gpio);
    if (dev->irq < 0)
        return dev->irq;

    ret = devm_request_threaded_irq(&client->dev,
                                    dev->irq,
                                    NULL,
                                    example_irq_thread,
                                    IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
                                    DRIVER_NAME,
                                    dev);

    if (ret)
        return ret;

    i2c_set_clientdata(client, dev);

    dev_info(&client->dev, "I2C transfer driver initialized\n");

    return 0;
}

/* Remove */
static void example_remove(struct i2c_client *client)
{
    dev_info(&client->dev, "Driver removed\n");
}

static const struct of_device_id example_of_match[] = {
    { .compatible = "example,i2c-transfer-device" },
    { }
};
MODULE_DEVICE_TABLE(of, example_of_match);

static struct i2c_driver example_driver = {
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = example_of_match,
    },
    .probe = example_probe,
    .remove = example_remove,
};

module_i2c_driver(example_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Example");
MODULE_DESCRIPTION("I2C driver using i2c_transfer");
