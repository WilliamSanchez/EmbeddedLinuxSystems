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
#define PWM_MOTOR_OFF _IOW('a','b',int32_t*)
#define PWM_MOTOR_SET_DUTYCYCLE _IOW('a','c',int32_t*)

#define SERVO_PWM_PERIOD  	20000000
#define SERVO_MAX_DUTY  	 2550000
#define SERVO_MIN_DUTY		  500000

uint8_t read_buf[1024];

int main()
{
   int fd;
   
   fd = open("/dev/w_pwm_motor",O_RDWR);
   
   if(fd < 0)
   {
   	 printf("Cannot open device file...\n");
         return -1;
   }
   
   unsigned long duty_cycle = (SERVO_MAX_DUTY-SERVO_MIN_DUTY)/2;
   
   ioctl(fd, PWM_MOTOR_ON, (int32_t *)&duty_cycle);
   
   while(1){
        
        sleep(1);duty_cycle = SERVO_MAX_DUTY;
    	ioctl(fd, PWM_MOTOR_SET_DUTYCYCLE, (int32_t *)&duty_cycle);
   	sleep(1); duty_cycle = (SERVO_MAX_DUTY-SERVO_MIN_DUTY)/2;
 	ioctl(fd, PWM_MOTOR_SET_DUTYCYCLE, (int32_t *)&duty_cycle);
   	sleep(1);duty_cycle = SERVO_MIN_DUTY;
   	ioctl(fd, PWM_MOTOR_SET_DUTYCYCLE, (int32_t *)&duty_cycle);
   	sleep(1);duty_cycle = (SERVO_MAX_DUTY-SERVO_MIN_DUTY)/2;
 	ioctl(fd, PWM_MOTOR_SET_DUTYCYCLE, (int32_t *)&duty_cycle);
   }
   close(fd);

   return 0;
}
