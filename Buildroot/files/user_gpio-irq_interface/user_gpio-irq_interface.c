#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>


#define DEVICE_NAME 	"w_gpio_irq"
#define SIGETX 44

#define REGISTER_UAPP _IO('R', 'g')
#define WR_VALUE _IOW('a', 'a', int32_t *)
#define RD_VALUE _IOR('a', 'b', int32_t *)

void signalhandler(int sig) {
	printf("Button was pressed!\n");
}

int main()
{
   int fd;
   printf("Starting program signal %d\n",SIGETX);
   if(signal(SIGETX, signalhandler) == SIG_ERR)
   {
   	printf("Error signal\n");
   	return -1;
   }
   
   
   fd = open("/dev/w_gpio_irq",O_RDWR);   
   if(fd < 0)
   {
   	 printf("Cannot open device file...\n");
         return -1;
   }
   
   if(ioctl(fd, REGISTER_UAPP, NULL))
   {
   	perror("Error registering task\n");
   	close(fd);
   	return -1;
   }
   
   printf("Wait for signal...\n");
   while(1){
   	sleep(1);
   }
   close(fd);
   

   return 0;
}
