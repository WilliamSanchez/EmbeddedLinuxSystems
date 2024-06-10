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
   
   fd = open("/dev/w_lidar_i2c",O_RDWR);
   
   if(fd < 0)
   {
   	 printf("Cannot open device file...\n");
         return -1;
   }
   
   while(1){
   	read(fd, read_buf, 5);
   	for(int i = 0; i< 5; i++)
    		printf("lidar: %x \n", read_buf[i]);
   	printf("%d Data = %d%dcm\n", cont++,read_buf[1],read_buf[0]);
   	sleep(1);
   }
   close(fd);

   return 0;
}
