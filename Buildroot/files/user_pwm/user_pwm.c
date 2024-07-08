#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>

#define PWM_MOTOR_ON _IOW('a','a',int32_t*)
#define PWM_MOTOR_OFF _IOW('a','a',int32_t*)

#define SERVO_PWM_PERIOD  	20000000
#define SERVO_MAX_DUTY  	 2500000
#define SERVO_MIN_DUTY		  550000

uint8_t read_buf[1024];

int main()
{
   int fd, cont =0;
   
   fd = open("/dev/w_pwm_motor",O_RDWR);
   
   if(fd < 0)
   {
   	 printf("Cannot open device file...\n");
         return -1;
   }
   
   unsigned long duty_cycle = (SERVO_MAX_DUTY-SERVO_MIN_DUTY)/2;
   
   ioctl(fd, PWM_MOTOR_ON, (int32_t *)&duty_cycle);
   
   while(1){
   /*
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
 	write(fd, 'a', 1);*/
   	sleep(5);
   }
   close(fd);

   return 0;
}
