#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <asm/page.h>

#define SIGETX 44
#define REG_CURRENT_TASK _IOW('a','a',int32_t*)

static int signum = 0;
int32_t value = 0;
static void *my_data = NULL;

dev_t dev = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

static struct task_struct *kthread_1;
static struct task_struct *kthread_2;
static struct task_struct *task = NULL;
wait_queue_head_t wait_queue_etx;

int wait_queue_flag = 0;

static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_write(struct file *filp, const char *buf,  size_t len, loff_t *off);
static long etx_ioctl(struct file *file, unsigned int cmd,  unsigned long arg);
static int etx_mmap(struct file *file, struct vm_area_struct *vma);

static struct file_operations fops = 
{
  .owner = THIS_MODULE,
  .write = etx_write,
  .open  = etx_open,
  .mmap  = etx_mmap,
  .unlocked_ioctl = etx_ioctl,
  .release = etx_release,
};

int thread_wait(void *unused)
{
  static uint8_t conSignal = 0;
  while (!kthread_should_stop())
  {
    pr_info("Waiting for event ...");
    wait_event_interruptible(wait_queue_etx, wait_queue_flag !=0);
    conSignal++;
    memset(my_data,conSignal,32);
    struct siginfo info;
    memset(&info, 0, sizeof(struct siginfo));
    info.si_signo = SIGETX;
    info.si_code = SI_QUEUE;
    info.si_int = conSignal;
    if(task != NULL)
    {
      pr_info("Sending signal to app\n");
      if(send_sig_info(SIGETX,(struct kernel_siginfo *)&info,task) < 0)
      {
        pr_info("Unable to send signal\n");
      }
    }
    pr_info("Send siganl OK ...");


    //mutex_lock()

    //mutex_ulock()
    wait_queue_flag = 0;
  }
  pr_info("Exit thread wait");
  return 0;
  
}

int thread_function(void *thread_arg)
{
  int i = 0;
  int t = *(int*)thread_arg;
  while(!kthread_should_stop())
  {
    i++;
    pr_info("%d In Thread kernel function %d\n",t, i);
    if(i%3 == 0){
        wait_queue_flag = 1;
        wake_up_interruptible(&wait_queue_etx);
    }
    msleep(5000);
  }
  pr_info("Exit thread function");
  return 0;
}

static int etx_open(struct inode *inode, struct file *file)
{
  pr_info("Device file opened ...\n");
  return 0;
}

static int etx_release(struct inode *inode, struct file *file)
{
  struct task_struct *ref_task = get_current();
  pr_info("Device file closed ...\n");

  if(ref_task == task ){
    task = NULL;
  }
  return 0;
}

static ssize_t etx_write(struct file *filp, const char *buf,  size_t len, loff_t *off)
{
   uint8_t rec_buf[10] = {0};
   pr_info("writing data\n");
   if(copy_from_user(rec_buf,buf,len) > 0)
   {
       pr_err("ERROR: cannot get data from user\n");
       return 0;
   }
   pr_info("Received rec_buf[0]=%d\n",rec_buf[0]);
  return len;
}

static long etx_ioctl(struct file *file, unsigned int cmd,  unsigned long arg)
{
  if(cmd == REG_CURRENT_TASK)
  {
    pr_info("REG_CURRENT_TASK");
    task = get_current();
    signum = SIGETX;
  }
  return 0;
}

static int etx_mmap(struct file *file, struct vm_area_struct *vma)
{
  int status;
  vma->vm_pgoff = virt_to_phys(my_data) >> PAGE_SHIFT;
  status = remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,vma->vm_end - vma->vm_start, vma->vm_page_prot);
  if(status)
  {
    pr_info("etx_mmap - Error rmap_pfn_range: %d",status);
    return -EAGAIN;
  }
  return 0;
}
int init_module(void){
  printk("Init kthread module\n");

  if((alloc_chrdev_region(&dev, 1, 0,"etx_Dev")) < 0)
  {
    pr_info("cannot allocat msjor number\n");
    return -1;
  }
  pr_info("Major= %d, Minor= %d\n",MAJOR(dev),MINOR(dev));

  cdev_init(&etx_cdev, &fops);

  if((cdev_add(&etx_cdev,dev,1)) < 0)
  {
    pr_info("Cannot add the device to the system\n");
    goto r_class;
  }

  if(IS_ERR(dev_class = class_create("etx_class")))
  {
    pr_info("Cannot create the struct class\n");
    goto r_class;
  }

  if(IS_ERR(device_create(dev_class,NULL,dev,NULL,"etx_device")))
  {
    pr_info("Cannot create the device\n");
    goto r_device;
  }

  my_data = kzalloc(PAGE_SIZE,GFP_DMA);
  if(!my_data)
  {
    pr_info("my_data cannot allocate memory\n");
    return -ENOMEM;
  }

  init_waitqueue_head(&wait_queue_etx);

  kthread_2 = kthread_run(thread_wait,NULL,"kthread_2");
  if(kthread_2 != NULL)
  {
    printk("kthread_2 was created\n");
  }else{
    printk("Kthread_2 could not be created\n");
    kthread_stop(kthread_2);
    return -1;
  }

  kthread_1 = kthread_create(thread_function,&wait_queue_flag,"kthread_1");
  if(kthread_1 != NULL)
  {
    kthread_bind(kthread_1,0);
    wake_up_process(kthread_1);
    printk("kthread_1 was created\n");
  }else{
  printk("Kthread_1 could not be created\n");
    kthread_stop(kthread_1);
    return -1;
  }

  return 0;

  r_device:
    class_destroy(dev_class);

  r_class:
    unregister_chrdev_region(dev,1);

  return -1;
}

void cleanup_module(void){
  kthread_stop(kthread_1);
  kthread_stop(kthread_2);
  device_destroy(dev_class,dev);
  class_destroy(dev_class);
  cdev_del(&etx_cdev);
  unregister_chrdev_region(dev,1);
  printk("Exit of kthread_1\n");
}

MODULE_LICENSE("GPL");
