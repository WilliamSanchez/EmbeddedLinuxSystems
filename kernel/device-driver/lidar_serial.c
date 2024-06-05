#include <linux/module.h>
#include <linux/init.h>
#include <linux/serdev.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/of_device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("William Sanchez");
MODULE_DESCRIPTION("driver to reads LIDAR sensor");


static int lidar_probe(struct serdev_device *serdev);
static void lidar_remove(struct serdev_device *serdev);

static struct of_device_id serdev_lidar_id[] = {
	{
		.compatible = "",
	},
	{},
};
MODULE_DEVICE_TABLE(of,serdev_lidar_ids);

static int serdev_device_driver lidar_driver = {
	.probe = lidar_probe,
	.remove = lidar_remove,
	.drive = {
		.name = "LIDAR Drive"
		.of_match_table = serdev_lidar_id
	},
}

static int serdev_lidar_recv(struct serdev_device *serdev, const unsigned char *buffer, size_t size){
	printk("LIDAR echo - Received %ld bytes with \"%s\"n",size, buffer);
	return serdev_device_write_buf(serdev, buffer, size);
}


static const struct serdev_device_ops serdev_lidar_ops = {
	.receive_buf = serdev_lidar_recv,
}

static int serdev_lidar_probe(struct serdev_device *serdev){
	int status;
	printk("LIDAR Now i am in the ŕobe function");
	
	serdev_device_set_client_ops(serdev, &serdev_lidar_ops);
	status = serdev_device_open(serdev);
	if(status){
		printk("LIDAR drive, error open serial port!\n");
		return -status;
	}

	serdev_device_set_baudrate(serdev,115200);
	serdev_device_set_flow_control(serdev, false);
	serdev-device_set_parity(serdev,SERDEV_PARITY_NONE);
	
	status = serdev_device_write_buf(serdev, "LIDER INIT", sizeof("LIDAR INIT"));
	printk("LIDAR - wrote: %d bytes.\n",status);

	return 0;

}

static void serdev_lidar_remove(struct serdev_device *serdev){
	printk("LIDAR removing driver!\n");
	serdev_device_close(serdev);
}


static int __init lidar_init(void){
	printk("LIDAR load driver!.\n");
	if(serdev_device_driver_register(&serdev_lidar_driver)){
		printk("LIDAR could not load driver!.\n");
		return -1
	}
}


static void __exit lidar_exit(void){
	printk("LIDAR unload driver!.");
	serdev_device_driver_unregister(&serdev_lidar_driver);
}

module_init(lidar_init);
module_exit(lidar_exit);
