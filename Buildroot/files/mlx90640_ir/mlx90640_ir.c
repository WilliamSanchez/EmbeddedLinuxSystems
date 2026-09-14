#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <signal.h>

#include "MLX90640_API.h"

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

paramsMLX90640 ext_paramsMLX90640;
struct mlx90640_device_id_parameter mlx90640_dataID; 

static int done=0;
int check = 0;
void *ptr;
int fd;
float image_temp[768];

uint16_t frame_data[832];

void sig_event_handler(int n, siginfo_t *info, void *unusde)
{
   if(n==SIGGETFRAME)
   {
      char send[]={0x03, 0x05};
      printf("Received mlx90640 status register: value = %x\n",mlx90640_dataID.status_register);
      check = info->si_int;
      printf("Received signal and data from kernel: value = %u\n",check);
      ptr = mmap(NULL,832*sizeof(uint16_t),PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
      if(ptr != MAP_FAILED)
      {
         memset(frame_data,0,832*sizeof(uint16_t));    
         memcpy(frame_data,ptr,832*sizeof(uint16_t));
         printf("\n**************************   FRAME DATA  ********************************************************************\n");
         printf("\t0\t1\t2\t3\t4\t5\t6\t7\t8\t9\tA\tB\tC\tD\tE\tF\n");
         for(int i=0; i<52;i++)
         {  printf("%x",0x0400+i*16);
            for(int j=0; j<16; j++)
            {
               printf("\t%x",frame_data[i*16+j]);
            }
            printf("\n");
         }
         printf("\n************************************************************************************************************\n");
/*
         MLX90640_GetImage(frame_data, &ext_paramsMLX90640, image_temp);
         printf("\n**************************   FRAME DATA  ********************************************************************\n");
         printf("\t0\t1\t2\t3\t4\t5\t6\t7\t8\t9\t11\n");
         for(int i=0; i<32*2;i++)
         {  printf("%x",i*11);
            for(int j=0; j<11; j++)
            {
               printf("\t%.2f",image_temp[i*11+j]);
            }
            printf("\n");
         }
         printf("\n************************************************************************************************************\n");
*/
         munmap(ptr,832);
      }
      write(fd,send,2);
   }
}

int main()
{

   int32_t number;
   struct sigaction act;

   sigemptyset(&act.sa_mask);
   act.sa_flags = (SA_SIGINFO | SA_RESTART);
   act.sa_sigaction = sig_event_handler;
   sigaction(SIGGETFRAME,&act,NULL);

   printf("Installed signal handler for SIGGETFRAME=%d\n",SIGGETFRAME);

   fd = open("/dev/IMAGE_IR",O_RDWR);   
   if(fd < 0)
   {
   	 printf("Cannot open device file...\n");
         return -1;
   }

   if(ioctl(fd, RD_DEVICE_ID,&mlx90640_dataID) < 0)
   {
      printf("error read device ID");
      return -1;
   }
   printf("Device id : %x, %x, %x\n",mlx90640_dataID.id[0], mlx90640_dataID.id[1], mlx90640_dataID.id[2]);

   if(ioctl(fd, RD_DEVICE_PARAMETERS,&mlx90640_dataID) < 0)
   {
      printf("error read device parameters\n");
      return -1;
   }

   printf("\n**********************************************************************************************\n");
   printf("\t0\t1\t2\t3\t4\t5\t6\t7\t8\t9\tA\tB\tC\tD\tE\tF\n");
   for(int i=0; i<52;i++)
   {  printf("%x",0x2400+i*16);
      for(int j=0; j<16; j++)
      {
         printf("\t%x",mlx90640_dataID.calibration_parameters[i*16+j]);
      }
      printf("\n");
   }
   printf("\n**********************************************************************************************\n");

   if(MLX90640_ExtractParameters(&mlx90640_dataID.calibration_parameters[0], &ext_paramsMLX90640) < 0)
   {
         printf("Cannot extract parameters mlx90640...\n");
         return -1;
   }

   if(ioctl(fd,RD_DEVICE_FRAME,(int32_t*)&number))
   {  
      printf("failed");
      close(fd);
      exit(1);
   }

   while(1){
   	printf("Waiting for signal..\n");
      while(!done && !check){};
   	check =0;
   }
   close(fd);
   

   return 0;
}
