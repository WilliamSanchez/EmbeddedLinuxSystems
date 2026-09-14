#include <linux/module.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/completion.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/gpio/consumer.h>
#include <linux/of_gpio.h>
#include <linux/of.h>
#include <linux/mm.h>
#include <asm/page.h>
#include <linux/wait.h>
#include <linux/ioctl.h>

#define DEVICE_NAME         "IMAGE_IR"
#define DEVICE_ID           0x2407
#define START_PARAMETERS    0x2400   
#define SIGGETFRAME          44

struct mlx90640_device_id_parameter {
    uint16_t id[3];
    uint16_t status_register;
    uint16_t control_register;
    uint16_t configuration_register;
    uint16_t calibration_parameters[832];
};

#define RD_DEVICE_ID _IOR('a','b', struct mlx90640_device_id_parameter *)
#define RD_DEVICE_PARAMETERS _IOR('a','c', struct mlx90640_device_id_parameter *)
#define RD_DEVICE_FRAME _IOW('a','a',int32_t*)

struct mlx90640_dev {
    struct i2c_client *client;
    struct work_struct work;
    struct mutex lock;
    struct cdev mlx90640_cdev;
    dev_t devt;
};

static struct mlx90640_device_id_parameter mlx90640_parameter_ID;

struct mlx90640_dev *mlx90640_dev_t = NULL;
struct device *device;
static struct class *mlx90640_class = NULL;
static int major;

struct completion read_done;
static struct task_struct *read_frame;
static struct task_struct *new_frame;
static struct task_struct *send_signal = NULL;
wait_queue_head_t wait_queue_new_frame;

int wait_queue_newData_flag = 0;
static uint16_t *mlx90640_frameData = NULL;
static int signum = 0;

static ssize_t readReg_mlx90640(uint16_t reg, size_t len, uint16_t *buf);

int thread_wait_frame_data(void *unused)
{
    static uint8_t conSignal = 0;
    while(!kthread_should_stop())
    {
        uint16_t reg_ID;
        conSignal++;
        pr_info("Waiting for event ...");
        wait_event_interruptible(wait_queue_new_frame, wait_queue_newData_flag !=0);
        for (int i = 0; i < 832; i = (i == 0) ? 15 : i + 16)
        {
            reg_ID = 0x0400+i;
            if(readReg_mlx90640(reg_ID, 16, &mlx90640_frameData[i]) < 0)
            {
                pr_err("Not read device ID ..\n");
                break;
            }
        }      
        struct siginfo info;
        memset(&info, 0, sizeof(struct siginfo));
        info.si_signo = SIGGETFRAME;
        info.si_code = SI_QUEUE;
        info.si_int = 1;
        if(send_signal != NULL)
        {
          pr_info("Sending signal to app\n");
          if(send_sig_info(SIGGETFRAME,(struct kernel_siginfo *)&info,send_signal) < 0)
          {
            pr_info("Unable to send signal\n");
          }
        }
        pr_info("Send signal OK ...");
        wait_queue_newData_flag = 0;
    }
    return 0;
}

int thread_get_frame(void *thread_arg)
{
  uint8_t  i=0;
  int t = *(int*)thread_arg;
  uint16_t status_register;
  while(!kthread_should_stop())
  {
   
    readReg_mlx90640(0x8000, 1, &status_register);
    i++;
    pr_info("%d In Thread kernel function status register %x\n", t, status_register);
    if((status_register&0x0004) != 0){
        wait_queue_newData_flag = 1;
        wake_up_interruptible(&wait_queue_new_frame);
    }
    msleep(1000);
  }
  pr_info("Exit thread function");
  return 0;
}

static int mlx90640_open(struct inode *inode, struct file *file)
{
    pr_info("Device file opened return ok if the IR sensor is OK after initialization ..\n");
    return 0;    
}

static int mlx90640_release(struct inode *inode, struct file *file)
{
    pr_info("Device file closed ..\n");
    return 0;    
}

ssize_t mlx90640_write(struct file *file, const char __user *buf, size_t count, loff_t *off)
{
   uint8_t rec_buf[10] = {0};
   pr_info("writing data\n");
   if(copy_from_user(rec_buf,buf,count) > 0)
   {
       pr_err("ERROR: cannot get data from user\n");
       return 0;
   }
   pr_info("Received rec_buf[0]=%d\n",rec_buf[0]);
  return count;
}

static long mlx90640_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
uint16_t reg_ID;
  switch(cmd)
  {
    case RD_DEVICE_ID:        
        for(int i=0; i<3; i++)
        {
            reg_ID = DEVICE_ID+i;
            if(readReg_mlx90640(reg_ID, 1, &mlx90640_parameter_ID.id[i]) < 0)
            {
                pr_err("Not read device ID ..\n");
                break;
            }
        }
        if(copy_to_user((void __user *)arg, &mlx90640_parameter_ID, sizeof(mlx90640_parameter_ID)))
        {
            pr_err("Error get ID device\n");
        }
        break;
    
    case RD_DEVICE_PARAMETERS:
        for (int i = 0; i < 832; i = (i == 0) ? 15 : i + 16)
        {
            reg_ID = START_PARAMETERS+i;
            if(readReg_mlx90640(reg_ID, 16, &mlx90640_parameter_ID.calibration_parameters[i]) < 0)
            {
                pr_err("Not read device ID ..\n");
                break;
            }
        }
        if(copy_to_user((void __user *)arg, &mlx90640_parameter_ID, sizeof(mlx90640_parameter_ID)))
        {
            pr_err("Error get parameter device\n");
        }
        break;
    case RD_DEVICE_FRAME:
        pr_info("RD_DEVICE_FRAME");
        send_signal = get_current();
        signum = SIGGETFRAME;
        break;
    default: 
        break;
  }
  return 0;
}

static int mlx90640_mmap(struct file *file, struct vm_area_struct *vma)
{
  int status;
  vma->vm_pgoff = virt_to_phys(mlx90640_frameData) >> PAGE_SHIFT;
  status = remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, vma->vm_end - vma->vm_start, vma->vm_page_prot);
  if(status)
  {
    pr_info("etx_mmap - Error rmap_pfn_range: %d",status);
    return -EAGAIN;
  }
  return 0;
}

static struct file_operations mlx90640_fops = 
{
    .owner          = THIS_MODULE,
    .open           = mlx90640_open,
    .unlocked_ioctl = mlx90640_ioctl,
    .mmap           = mlx90640_mmap,
    .write          = mlx90640_write,
    .release        = mlx90640_release,
};

static ssize_t readReg_mlx90640(uint16_t reg, size_t len, uint16_t *buf)
{
    u8 reg_init[2];
    struct i2c_msg msg_init[2];
    unsigned char *data;
    data = kmalloc(2 * len, GFP_KERNEL);
    if (!data)
        return -ENOMEM;
        
    reg_init[0] = (u8)(reg >> 8);
    reg_init[1] = (u8)(reg);

    msg_init[0].addr = mlx90640_dev_t->client->addr;
    msg_init[0].flags = 0;
    msg_init[0].len = 2;
    msg_init[0].buf = reg_init;

    msg_init[1].addr = mlx90640_dev_t->client->addr;
    msg_init[1].flags = I2C_M_RD;
    msg_init[1].len = 2*len;
    msg_init[1].buf = data;

    pr_info("Reg %x|%x, num of uint16_t bytes %d\n",reg_init[0], reg_init[1],len);

    if(i2c_transfer(mlx90640_dev_t->client->adapter, msg_init, 2) < 0)
    {
    	pr_err("mlx90640: i2c transfer failed\n");
    	return -1;
    }   

    for(int i=0; i<len; i++)
    {
        buf[i] = data[2*i]<<8 | data[2*i+1];
    }    
    kfree(data);
    return len;
}

static ssize_t readReg_mlx90640(uint16_t reg, size_t len, uint16_t *buf)
{
    u8 reg_init[2];
    struct i2c_msg msg_init[2];
    unsigned char *data;
    data = kmalloc(2 * len, GFP_KERNEL);
    if (!data)
        return -ENOMEM;
    /*    
    reg_init[0] = (u8)(reg >> 8);
    reg_init[1] = (u8)(reg);

    msg_init[0].addr = mlx90640_dev_t->client->addr;
    msg_init[0].flags = 0;
    msg_init[0].len = 2+data;
    msg_init[0].buf = reg_init+data;

    if(i2c_transfer(mlx90640_dev_t->client->adapter, msg_init, 2) < 0)
    {
    	pr_err("mlx90640: i2c transfer failed\n");
    	return -1;
    }   
    */ 
    kfree(data);
    return len;
}

static int mlx90640_probe(struct i2c_client *client)
{
    pr_info("Initializing sensor image mlx90640");    

    int err;
    mlx90640_dev_t = devm_kzalloc(&client->dev,sizeof(*mlx90640_dev_t),GFP_KERNEL);
    if(!mlx90640_dev_t)
        return -ENOMEM;

    mlx90640_dev_t->client = client;
    if(!i2c_check_functionality(mlx90640_dev_t->client->adapter, I2C_FUNC_SMBUS_BYTE_DATA))
        return -EIO;

    mdelay(1000);

    uint16_t id_device = 0x0000;
    uint16_t reg_ID = 0x2407;
    if(readReg_mlx90640(reg_ID, 1, &id_device) < 0)
    {
        pr_err("Not read info from function ..\n");
        return -1;
    }
    pr_info("mlx90640 ID %x : %x\n",reg_ID,id_device);
    reg_ID++;
    if(readReg_mlx90640(reg_ID, 1, &id_device) < 0)
    {
        pr_err("Not read info from function ..\n");
        return -1;
    }
    pr_info("mlx90640 ID %x : %x\n",reg_ID,id_device);
    reg_ID++;
    if(readReg_mlx90640(reg_ID, 1, &id_device) < 0)
    {
        pr_err("Not read info from function ..\n");
        return -1;
    }  
    pr_info("mlx90640 ID %x : %x\n",reg_ID,id_device);

    major = register_chrdev(0, "mlx90640", &mlx90640_fops);
    if(major < 0)
    {
    	pr_err("[target] register chrdv() failed\n");
    	return major;
    }

    mutex_init(&mlx90640_dev_t->lock);

    device = device_create(mlx90640_class,NULL,MKDEV(major,0),NULL,DEVICE_NAME);
    if(IS_ERR(device)){
        err = PTR_ERR(device);
        pr_err("Error creatind device create");  
        goto fail;     
    }
 
    mutex_lock(&mlx90640_dev_t->lock);
    i2c_set_clientdata(mlx90640_dev_t->client, mlx90640_dev_t);
    mutex_unlock(&mlx90640_dev_t->lock);

    //PAGE_SIZE =  the size of one memory page. It is architecture-dependent, commonly 4096 bytes (4 KiB).
    //GFP_DMA   =   a memory-allocation flag requesting memory from a region suitable for DMA on systems where DMA has address limitations. 
    mlx90640_frameData = kzalloc(PAGE_SIZE,GFP_DMA); // 2*832 = 1664 < PAGE_SIZE
    if(!mlx90640_frameData){    
        pr_info("my_data cannot allocate memory\n");
        return -ENOMEM;
    }

    init_waitqueue_head(&wait_queue_new_frame);

    new_frame = kthread_run(thread_wait_frame_data,NULL,"new_frame");
    if(new_frame != NULL)
    {
      printk("new_frame was created\n");
    }else{
      printk("new_frame could not be created\n");
      kthread_stop(new_frame);
      return -1;
    }
    
    read_frame = kthread_create(thread_get_frame,&wait_queue_newData_flag,"read_frame");
    if(read_frame != NULL)
    {
      kthread_bind(read_frame,0);
      wake_up_process(read_frame);
      printk("read_frame was created\n");
    }else{
    printk("read_frame could not be created\n");
      kthread_stop(read_frame);
      return -1;
    }

    pr_info("Finish initializing set sensor image mlx90640");

    return 0;

    fail:
        if(mlx90640_dev_t)
            kfree(mlx90640_dev_t);
        return err;
}

static void mlx90640_remove(struct i2c_client *client)
{
    kthread_stop(read_frame);
    kfree(mlx90640_dev_t);
    pr_info("Removing device driver\n");
}

static const struct i2c_device_id i2c_mlx90640_id[] ={
    {"mlx90640",0},
    {},
};

MODULE_DEVICE_TABLE(i2c,i2c_mlx90640_id);

static const struct of_device_id mlx90640_id[]={
    {.compatible="mlx90640,infrared_image"},
    {},
};

MODULE_DEVICE_TABLE(of,mlx90640_id);

static struct i2c_driver mlx90640_driver = {
    .driver={
        .owner          =THIS_MODULE,
        .name           ="mlx90640",
        .of_match_table =of_match_ptr(mlx90640_id),
    },
    .probe      = mlx90640_probe,
    .remove     = mlx90640_remove,
    .id_table   = i2c_mlx90640_id,
};

static int __init mlx90640_init(void)
{
    int status;
    mlx90640_class = class_create("mlx90640");
    if(IS_ERR(mlx90640_class))
        return PTR_ERR(mlx90640_class);

    status = i2c_register_driver(THIS_MODULE,&mlx90640_driver);
    if(status < 0)
        class_destroy(mlx90640_class);

    return status;
}

static void __exit mlx90640_exit(void)
{
    if(mlx90640_frameData)
		kfree(mlx90640_frameData);
    kthread_stop(read_frame);
    kthread_stop(new_frame);
    kfree(mlx90640_dev_t);    
    i2c_del_driver(&mlx90640_driver);
    class_destroy(mlx90640_class);
    pr_info("Exit of device driver\n");
}

module_init(mlx90640_init);
module_exit(mlx90640_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("William Sanchez");
MODULE_DESCRIPTION("mlx90640 device driver ");

































