#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/delay.h>

static struct task_struct *kthread_1;
static int dado = 5;


int thread_function(void *thread_arg)
{
  int i = 0;
  int t = *(int*)thread_arg;
  while(!kthread_should_stop())
  {
    pr_info("%d In Thread kernel function %d\n",t, i++);
    msleep(1000);
  }
  return 0;
}

int init_module(void){
  printk("Init kthread module\n");

  kthread_1 = kthread_create(thread_function,&dado,"kthread_1");
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
}

void cleanup_module(void){
  kthread_stop(kthread_1);
  printk("Exit of kthread_1\n");
}

MODULE_LICENSE("GPL");
