#include <linux/module.h>
#include <linux/init.h>
#include <linux/serdev.h>
#include <linux/mod_devicetable.h>
#include <linux/property.h>
#include <linux/of_device.h>

#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME  "serial_micro_to_pc"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("William Sanchez");
MODULE_DESCRIPTION("driver to reads LIDAR sensor");

static struct class *serialPC_class = NULL;

static int lidar_probe(struct serdev_device *serdev);
static void lidar_remove(struct serdev_device *serdev);

static char datatopc[1024];

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

ssize_t serialpc_read(struct file *filp, char __user *buffer, size_t length, loff_t *loff)
{
       if(copy_to_user(buffer, datatopc, sizeof(datatopc))!=0){
    	pr_err("lidar: transfer data tu user failed\n");
	goto end_read;
     }
     
     return 0;
     
     end_read:
     	return -1;
}


ssize_t serialpc_write(struct file *file, const char __user *buf, size_t count, loff_t *d_ops)
{
/*
     char datafromuser[128]; 
     if(copy_from_user(datafromuser, buf, count)!=0){
    	pr_err("lidar: transfer data tu user failed\n");
	goto end_write;
     }
*/   //  pr_info("The user wrote %s",datafromuser);
     
     return 0; //scount;
     
//     end_write:
//     	return -1;
}
static struct file_operations fops = 
{
   .owner 	= THIS_MODULE,
   .read	= serialpc_read,
   .write	= serialpc_write,
   .open	= serialpc_open,
   .release	= serialpc_release,

}; 

static struct of_device_id serdev_lidar_id[] = {
	{
		.compatible = "serialuc,microcontroller",
	},
	{},
};
MODULE_DEVICE_TABLE(of,serdev_lidar_id);

static struct serdev_device_driver lidar_driver = {
	.probe = lidar_probe,
	.remove = lidar_remove,
	.driver = {
		.name = "LIDAR Drive",
		.of_match_table = serdev_lidar_id,
	},
};

static int serdev_lidar_recv(struct serdev_device *serdev, const unsigned char *buffer, size_t size){
	//printk("LIDAR echo - Received %d bytes with %s",size, buffer);
	memcpy(datatopc,buffer,size);
	return serdev_device_write_buf(serdev, buffer, size);
}


static const struct serdev_device_ops serdev_lidar_ops = {
	.receive_buf = serdev_lidar_recv,
};

static int lidar_probe(struct serdev_device *serdev){
	int status;
	int major;
	int err;
	struct device *device = NULL;
	printk("LIDAR Now i am in the ŕobe function");
	
	major = register_chrdev(0,"serial_micro_pc", &fops);
	if(major < 0)
	{
	   pr_err("Error to register char dev\n");
	   return major;
	}
	
	device = device_create(serialPC_class,NULL,MKDEV(major,0),NULL,DEVICE_NAME);	
	if(IS_ERR(device))
	{
	   err = PTR_ERR(device);
	   pr_err("Not create the device erro %d",err);
	   goto fail;
	}
		
	serdev_device_set_client_ops(serdev, &serdev_lidar_ops);
	status = serdev_device_open(serdev);
	if(status){
		printk("LIDAR drive, error open serial port!\n");
		return -status;
	}

	serdev_device_set_baudrate(serdev,115200);
	serdev_device_set_flow_control(serdev, false);
	serdev_device_set_parity(serdev,SERDEV_PARITY_NONE);
	
	status = serdev_device_write_buf(serdev, "LIDER INIT", sizeof("LIDAR INIT"));
	printk("LIDAR - wrote: %d bytes.\n",status);

        printk("Comunication Serial Microcontroller to PC is OK\n");
	return 0;
	
	fail:
	  return err;

}

static void lidar_remove(struct serdev_device *serdev){
	printk("LIDAR removing driver!\n");
	serdev_device_close(serdev);
}


static int __init lidar_init(void){
	printk("LIDAR load driver!.\n");

	serialPC_class = class_create("SerialPC");
	
	if(IS_ERR(serialPC_class))
		return PTR_ERR(serialPC_class);
	
	if(serdev_device_driver_register(&lidar_driver)){
		printk("LIDAR could not load driver!.\n");
		class_destroy(serialPC_class);
		return -1;
	}
	return 0;
}


static void __exit lidar_exit(void){
	printk("LIDAR unload driver!.");
	serdev_device_driver_unregister(&lidar_driver);
}

module_init(lidar_init);
module_exit(lidar_exit);

