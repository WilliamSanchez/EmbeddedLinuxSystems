#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/gpio/consumer.h>
#include <linux/regmap.h>
#include <linux/workqueue.h>

#define DRIVER_NAME "example_i2c_irq"

/* Example registers */
#define REG_STATUS  0x00
#define REG_DATA    0x01

struct example_dev {
    struct i2c_client *client;
    struct regmap *regmap;
    struct gpio_desc *irq_gpio;
    int irq;

    struct work_struct work;
};

/* regmap configuration */
static const struct regmap_config example_regmap_config = {
    .reg_bits = 8,
    .val_bits = 8,
};

/* Deferred work */
static void example_work_handler(struct work_struct *work)
{
    struct example_dev *dev =
        container_of(work, struct example_dev, work);

    unsigned int status;
    unsigned int data;

    regmap_read(dev->regmap, REG_STATUS, &status);
    regmap_read(dev->regmap, REG_DATA, &data);

    dev_info(&dev->client->dev,
             "Device event: status=%u data=%u\n",
             status, data);
}

/* Threaded IRQ handler */
static irqreturn_t example_irq_thread(int irq, void *data)
{
    struct example_dev *dev = data;

    schedule_work(&dev->work);

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

    /* Initialize regmap */
    dev->regmap = devm_regmap_init_i2c(client, &example_regmap_config);
    if (IS_ERR(dev->regmap))
        return PTR_ERR(dev->regmap);

    /* Get interrupt GPIO */
    dev->irq_gpio = devm_gpiod_get(&client->dev, "irq", GPIOD_IN);
    if (IS_ERR(dev->irq_gpio))
        return PTR_ERR(dev->irq_gpio);

    dev->irq = gpiod_to_irq(dev->irq_gpio);
    if (dev->irq < 0)
        return dev->irq;

    INIT_WORK(&dev->work, example_work_handler);

    /* Request threaded IRQ */
    ret = devm_request_threaded_irq(&client->dev,
                                    dev->irq,
                                    NULL,
                                    example_irq_thread,
                                    IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
                                    DRIVER_NAME,
                                    dev);

    if (ret) {
        dev_err(&client->dev, "Failed to request IRQ\n");
        return ret;
    }

    i2c_set_clientdata(client, dev);

    dev_info(&client->dev, "Example I2C IRQ driver loaded\n");

    return 0;
}

/* Remove */
static void example_remove(struct i2c_client *client)
{
    struct example_dev *dev = i2c_get_clientdata(client);

    cancel_work_sync(&dev->work);

    dev_info(&client->dev, "Driver removed\n");
}

/* Device Tree match */
static const struct of_device_id example_of_match[] = {
    { .compatible = "example,i2c-irq-device" },
    { }
};
MODULE_DEVICE_TABLE(of, example_of_match);

/* I2C driver */
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
MODULE_DESCRIPTION("Production-style I2C driver with IRQ");
