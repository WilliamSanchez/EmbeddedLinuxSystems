#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int datos = 0;

//MODULE_AUTHOR("William Sanchez");
//MODULE_DESCRIPTION("Modulo ejemplo Hello world");
//MODULE_PARM(datos,"i");


int init_module(void){
  printk("Hello world, parametro %d\n",datos);
  return 0;
}

void cleanup_module(void){
  printk("bye world\n");
}

MODULE_LICENSE("GPL");
