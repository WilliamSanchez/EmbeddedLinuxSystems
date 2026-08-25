#include <linux/module.h>
#include <linux/init.h>
#include <linux/serdev.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/of_device.h>

#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME  "bluetooth_serial"
#define mem_size	1024


MODULE_LICENSE("GPL");
MODULE_AUTHOR("William Sanchez");
MODULE_DESCRIPTION("driver to sends and receives seral data using bluetooth");

static struct class *BluetoothSerialPC_class = NULL;
struct serdev_device *BluetoothSerial;

uint8_t *datafromuser;
uint8_t *datatopc;
static char bluetoothtopc[1024];

static int bluetooth_probe(struct serdev_device *serdev);
static void bluetooth_remove(struct serdev_device *serdev);



int serialpc_open(struct inode *inode, struct file *filp)
{

     pr_info("Open device\n");
     return 0;
}

int serialpc_release(struct inode *inode, struct file *filp)
{

     pr_info("Release device\n");
     return 0;
}

static ssize_t serialpc_read(struct file *filp, char __user *buffer, size_t length, loff_t *loff)
{
    //if(copy_to_user(buffer, datatopc, strlen(datatopc)) != 0){
    if(copy_to_user(buffer, bluetoothtopc, strlen(bluetoothtopc)) != 0){
    	pr_err("bluetooth: transfer data to user failed\n");
		goto end_read;
    } 
	//memset(datatopc,0x00,strlen(datatopc));
    return 0;
     
    end_read:
     	return -1;
}

static ssize_t serialpc_write(struct file *file, const char __user *buf, size_t count, loff_t *d_ops)
{
//     char *datafromuser; 
     if(copy_from_user(datafromuser, buf, count)!=0){
    	pr_err("bluetooth: transfer data tu user failed\n");
		goto end_write;
     }

	printk("Received data from user");
	serdev_device_write_buf(BluetoothSerial, datafromuser, count);
   	//  pr_info("The user wrote %s",datafromuser);
     return 0; //scount;
     
     end_write:
     	return -1;
}

static struct file_operations fops = 
{
   .owner 	= THIS_MODULE,
   .read	= serialpc_read,
   .write	= serialpc_write,
   .open	= serialpc_open,
   .release	= serialpc_release,

}; 

static struct of_device_id serdev_bluetooth_id[] = {
	{
		.compatible = "serialuc,microcontroller",
	},
	{},
};

MODULE_DEVICE_TABLE(of,serdev_bluetooth_id);

static struct serdev_device_driver bluetooth_driver = {
	.probe = bluetooth_probe,
	.remove = bluetooth_remove,
	.driver = {
		.name = "Serial bluetooth drive",
		.of_match_table = serdev_bluetooth_id,
	},
};

static int serdev_bluetooth_recv(struct serdev_device *serdev, const unsigned char *buffer, size_t size){
	//printk("bluetooth echo - Received %d bytes with %s",size, buffer);
	//if(strstr(buffer,"\r")){
		//memcpy(datatopc,buffer,size);	
		memcpy(bluetoothtopc,buffer,size);
		return size; //serdev_device_write_buf(serdev, buffer, size);
	//}
	//return 0;

}

static const struct serdev_device_ops serdev_bluetooth_ops = {
	.receive_buf = serdev_bluetooth_recv,
};

static int bluetooth_probe(struct serdev_device *serdev){
	int status;
	int major;
	int err;
	struct device *device = NULL;
	BluetoothSerial = serdev;
	printk("Blueetooth Now i am in the probe function");
	
	major = register_chrdev(0,"serial_bluetooth_pc", &fops);
	if(major < 0)
	{
	   pr_err("Error to register char dev\n");
	   return major;
	}
	
	device = device_create(BluetoothSerialPC_class,NULL,MKDEV(major,0),NULL,DEVICE_NAME);	
	if(IS_ERR(device))
	{
	   err = PTR_ERR(device);
	   pr_err("Not create the device erro %d",err);
	   goto fail;
	}
		
	serdev_device_set_client_ops(serdev, &serdev_bluetooth_ops);
	status = serdev_device_open(serdev);
	if(status){
		printk("Bluetooth drive, error open serial port!\n");
		return -status;
	}

	serdev_device_set_baudrate(serdev,115200);
	serdev_device_set_flow_control(serdev, false);
	serdev_device_set_parity(serdev,SERDEV_PARITY_NONE);

	if((datafromuser = kmalloc(mem_size,GFP_KERNEL)) == 0){
		pr_info("Cannot allocate memory in kernel\n");
		goto fail;
   	}
	
	if((datatopc = kmalloc(mem_size,GFP_KERNEL)) == 0){
		pr_info("Cannot allocate memory in kernel\n");
		goto fail;
   	}
	
	status = serdev_device_write_buf(BluetoothSerial, "Bluetooth to serial INIT\n\r", sizeof("luetooth to serial INIT\n\r"));
	printk("Bluetooth - wrote: %d bytes.\n",status);
	printk("Comunication Serial Bluetooth to PC is OK\n");
	
	return 0;
	
	fail:
	  return err;

}

static void bluetooth_remove(struct serdev_device *serdev){
	printk("bluetooth removing driver!\n");
	serdev_device_close(serdev);
}

static int __init bluetooth_init(void){
	printk("Bluetooth load driver!.\n");

	BluetoothSerialPC_class = class_create("SerialBluetoothPC");
	
	if(IS_ERR(BluetoothSerialPC_class))
		return PTR_ERR(BluetoothSerialPC_class);
	
	if(serdev_device_driver_register(&bluetooth_driver)){
		printk("Bluetooth could not load driver!.\n");
		class_destroy(BluetoothSerialPC_class);
		return -1;
	}
	return 0;
}

static void __exit bluetooth_exit(void){
	kfree(datafromuser);
	kfree(datatopc);
	printk("bluetooth unload driver!.");
	serdev_device_driver_unregister(&bluetooth_driver);
}

module_init(bluetooth_init);
module_exit(bluetooth_exit);

