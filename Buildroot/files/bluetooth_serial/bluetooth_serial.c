#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

uint8_t read_buf[128];
uint8_t write_buf[32];
size_t count = 0;

int main()
{
   int fd;

   printf("\nINIT Bluetooth Serial program\n");
   
   fd = open("/dev/bluetooth_serial",O_RDWR);      
   //fd = open("/dev/serial_micro_to_pc",O_RDWR); 
   if(fd < 0)
   {
   	 printf("Cannot open device file...\n");
         return -1;
   }
   
   strcpy(write_buf,"Conecting OK\t\n");   
//   while(1){
    	if (write(fd,write_buf,strlen(write_buf)) < 0)
   	{
   	   	printf("Not send data\n");
         	return -1;
   	}
   	printf("DONE send: %s len:%d !\n",write_buf,strlen(write_buf)+1);
//   	read(fd, read_buf, 128);
//   	if(strstr(read_buf,"OK") != NULL)
//   		break;
   	sleep(2);
//   }
  
   while(1){
 	count = read(fd, read_buf, 32);
 	//if(count > 0)
 	//{
 		printf("Data: %s len: %d\n",read_buf, count);
 	//}
	//memset(read_buf,0x00,128);
	//sleep(1);
	usleep(50000); //50ms
   }
   close(fd);

   return 0;
}
