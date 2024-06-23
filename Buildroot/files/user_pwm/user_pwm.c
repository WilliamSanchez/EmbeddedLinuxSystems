#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

uint8_t read_buf[1024];

int main()
{
   int fd, cont =0;
   
   fd = open("/dev/pwd_device_driver",O_RDWR);
   
   if(fd < 0)
   {
   	 printf("Cannot open device file...\n");
         return -1;
   }
   
   while(1){
    	write(fd, 'j', 1);
   	sleep(5);
 	write(fd, 'b', 1);
   	sleep(5);
 	write(fd, 'i', 1);
   	sleep(10);
   	write(fd, 'c', 1);
   	sleep(15);
 	write(fd, 'h', 1);
   	sleep(20);
   	write(fd, 'd', 1);
   	sleep(15);
 	write(fd, 'g', 1);
   	sleep(10);
   	write(fd, 'e', 1);
   	sleep(5);
 	write(fd, 'f', 1);
   	sleep(10);
 	write(fd, 'a', 1);
   	sleep(5);
   }
   close(fd);

   return 0;
}
