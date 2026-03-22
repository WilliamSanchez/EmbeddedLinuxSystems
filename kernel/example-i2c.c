#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>

#define DRIVER_NAME "example_i2c_irq"


/*
	device tree
	
example_device@50 {
    compatible = "example,i2c-irq-device";
    reg = <0x50>;

    irq-gpio = <&gpio1 5 GPIO_ACTIVE_LOW>;
};

*/

struct example_data {
    struct i2c_client *client;
    int irq_gpio;
    int irq_num;
};

/* IRQ Handler */
static irqreturn_t example_irq_handler(int irq, void *dev_id)
{
    struct example_data *data = dev_id;
    int value;

    printk(KERN_INFO "IRQ triggered!\n");

    /* Example: read register 0x00 from I2C device */
    value = i2c_smbus_read_byte_data(data->client, 0x00);

    printk(KERN_INFO "I2C register value: %d\n", value);

    return IRQ_HANDLED;
}

/* I2C Probe */
static int example_probe(struct i2c_client *client,
                         const struct i2c_device_id *id)
{
    struct example_data *data;
    int ret;

    printk(KERN_INFO "Example I2C driver probe\n");

    data = devm_kzalloc(&client->dev, sizeof(*data), GFP_KERNEL);
    if (!data)
        return -ENOMEM;

    data->client = client;

    /* Get GPIO from device tree */
    data->irq_gpio = of_get_named_gpio(client->dev.of_node, "irq-gpio", 0);
    if (!gpio_is_valid(data->irq_gpio)) {
        printk(KERN_ERR "Invalid GPIO\n");
        return -EINVAL;
    }

    ret = devm_gpio_request_one(&client->dev, data->irq_gpio,
                                GPIOF_IN, "example_irq_gpio");
    if (ret) {
        printk(KERN_ERR "GPIO request failed\n");
        return ret;
    }

    data->irq_num = gpio_to_irq(data->irq_gpio);
    if (data->irq_num < 0) {
        printk(KERN_ERR "Failed to get IRQ number\n");
        return data->irq_num;
    }

    /* Request interrupt */
    ret = devm_request_threaded_irq(&client->dev,
                                    data->irq_num,
                                    NULL,
                                    example_irq_handler,
                                    IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
                                    DRIVER_NAME,
                                    data);

    if (ret) {
        printk(KERN_ERR "IRQ request failed\n");
        return ret;
    }

    i2c_set_clientdata(client, data);

    printk(KERN_INFO "Driver initialized successfully\n");

    return 0;
}

/* Remove function */
static void example_remove(struct i2c_client *client)
{
    printk(KERN_INFO "Example driver removed\n");
}

/* I2C Device IDs */
static const struct i2c_device_id example_id[] = {
    { "example_device", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, example_id);

/* Device Tree Match */
static const struct of_device_id example_of_match[] = {
    { .compatible = "example,i2c-irq-device" },
    { }
};
MODULE_DEVICE_TABLE(of, example_of_match);

/* I2C Driver Structure */
static struct i2c_driver example_driver = {
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = example_of_match,
    },
    .probe = example_probe,
    .remove = example_remove,
    .id_table = example_id,
};

/* Module Init */
module_i2c_driver(example_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Example");
MODULE_DESCRIPTION("I2C device driver with GPIO IRQ");
