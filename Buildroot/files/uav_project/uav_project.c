#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <sched.h>
#include <time.h>
#include <semaphore.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <syslog.h>

#include "dqData.h"


#define PWM_MOTOR_ON _IOW('a','a',int32_t*)
#define PWM_MOTOR_OFF _IOW('a','b',int32_t*)
#define PWM_MOTOR_SET_DUTYCYCLE _IOW('a','c',int32_t*)

#define SERVO_PWM_PERIOD  	20000000
#define SERVO_MAX_DUTY  	 2550000
#define SERVO_MIN_DUTY		  500000
#define SERVO_DEGREE ((SERVO_MAX_DUTY-SERVO_MIN_DUTY)/180)
#define DEGREE_90 ((SERVO_MAX_DUTY+SERVO_MIN_DUTY)/2)

#define NUM_THREADS (3)
#define USER_PER_MSEC (1000)

#define LATITUDE		0
#define LONGITUDE		1
#define ALTITUDE		2
#define AIRSPEED		3
#define HEADING			4
#define X_ACCEL			6
#define Y_ACCEL			7
#define Z_ACCEL			8
#define PITCH_RATE		9
#define ROLL_RATE		10
#define YAW_RATE		11

#define BUF_SIZE 10
#define PORT_NUM_QGC 4500
#define PORT_NUM_FG_OUT  5500
#define PORT_NUM_FG_IN  5501

struct dqData _getData;

char buf[BUF_SIZE] = "Hola\n";
 
int abortTest = 0;
double start_time;
double get_time_NAV;

sem_t semCTL, semNAV;

typedef struct{
    int threadidx;
    int majorPeriods;
}threadParams_t; 


double getTimeMsec();
char rcv_data[1024];
char txr_data[1024];
int sock_in, sock_out;
float fdmData[100];
int addr_len, bytes_read;
int send_len;
   
struct sockaddr_in server_addr_in, client_addr_in;
struct sockaddr_in server_addr_out, client_addr_out;
struct sockaddr_in fgAddr;
struct hostent *host_out;

int numBytes_out;
static unsigned long duty_cycle = 0; 
static int fd_pwm, fd_uc;

struct controlFG {
    double rudder;
    double elevator;
    double aileron;
    double throttle;
}__attribute__((packed));

/*********************************************
*********************************************/

union temp64{
  int64_t ll;
  int32_t l[2];
};

void swap64(void *p)
{
     union temp64 *f, t;
     f = (union temp64 *)p;
     t.l[0] = htonl(f->l[1]);
     t.l[1] = htonl(f->l[0]);
     
     f->ll = t.ll;
}
/*********************************************
*********************************************/

int config_servo_pwm()
{

   fd_pwm = open("/dev/w_pwm_motor",O_RDWR);   
   if(fd_pwm < 0)
   {
   	 printf("Cannot open device file w_pwm_moto...\n");
         return -1;
   } 
   duty_cycle = DEGREE_90; 
   ioctl(fd_pwm, PWM_MOTOR_ON, (int32_t *)&duty_cycle);
   return 0;
}

int config_microcontroller()
{

   fd_uc = open("/dev/serial_micro_to_pc",O_RDWR);   
   if(fd_uc < 0)
   {
   	 printf("Cannot open device file serial_micro_to_pc...\n");
         return -1;
   } 
   return 0;
}

/*********************************************
*********************************************/

int set_servo_pwm(float angle)
{

   duty_cycle = (DEGREE_90+SERVO_DEGREE*angle); 
   ioctl(fd_pwm, PWM_MOTOR_SET_DUTYCYCLE, (int32_t *)&duty_cycle);
   
   return 0;
}

/*********************************************
*********************************************/

/*********************************************
*********************************************/

void init_sendData()
{
      
//   host_out = (struct hostnet*)gethostname((char*)"192.168.7.2");   
   if((sock_out = socket(AF_INET,SOCK_DGRAM,0)) == -1)
   {
       perror("socket");
       exit(1);
   }
   
   memset(&server_addr_out, 0, sizeof(struct sockaddr_in));
   server_addr_out.sin_family = AF_INET;
   server_addr_out.sin_port = htons(PORT_NUM_QGC);
   server_addr_out.sin_addr.s_addr = INADDR_ANY;//inet_addr("192.168.1.1");//
//   bzero(&(server_addr_out.sin_zero),8);
   
   if(bind(sock_out,(struct sockaddr *)&server_addr_out, sizeof(struct sockaddr_in)) == -1)
   {
   	perror("Bind");
   	//exit(1);
   }
   
   printf("\nSend data in the port %d\n",PORT_NUM_QGC);
   
//   fflush(stdout);
   numBytes_out = strlen(buf);
   char resp[100];
   int sock_len = sizeof(struct sockaddr_in);
   int numBytes = recvfrom(sock_out,resp,sizeof(resp),0,(struct sockaddr*)&client_addr_out, &sock_len);
   if(numBytes == -1)
   {
        perror("recvfrom");
   }

       printf("\nQGCS (%s,%d)said:",inet_ntoa(client_addr_out.sin_addr),ntohs(client_addr_out.sin_port)); 
       printf("Response: %.*s\n", (int)numBytes, resp);

}

/*********************************************
*********************************************/

/*********************************************
*********************************************/

void init_getData()
{
   
   if((sock_in = socket(AF_INET,SOCK_DGRAM,0)) < 0)
   {
       perror("socket");
       exit(1);
   }
   
   server_addr_in.sin_family = AF_INET;
   server_addr_in.sin_port = htons(PORT_NUM_FG_OUT);
   server_addr_in.sin_addr.s_addr = INADDR_ANY; //inet_addr("192.168.7.2");//
   bzero(&(server_addr_in.sin_zero),8);
   
   fgAddr.sin_family = AF_INET;
   fgAddr.sin_port = htons(PORT_NUM_FG_IN);
   fgAddr.sin_addr.s_addr = inet_addr("192.168.1.1");//INADDR_ANY; //
   //bzero(&(fgAddr.sin_zero),8);
   
   if(bind(sock_in,(struct sockaddr *)&server_addr_in, sizeof(struct sockaddr)) == -1)
   {
   	perror("Bind");
   	exit(1);
   }
   
   addr_len = sizeof(struct sockaddr);
   printf("\nUDPserver waiting for client to port 5500");
   
   fflush(stdout);
   
   bytes_read = recvfrom(sock_in,rcv_data,1024,0,(struct sockaddr*)&client_addr_in,&addr_len);
   rcv_data[bytes_read] = '\0';
   
       // print where it got the UDP data from and the raw data
    printf("\nFG (%s,%d)said:",inet_ntoa(client_addr_in.sin_addr),ntohs(client_addr_in.sin_port));
    printf("DATA: \n\r%s",rcv_data);
    
        sscanf(rcv_data,"%f%f%f%f%f%f%f%f%f%f%f",
	&_getData.latitude,&fdmData[LONGITUDE],&fdmData[ALTITUDE],
	&fdmData[AIRSPEED],&fdmData[HEADING],
	&fdmData[X_ACCEL],&fdmData[Y_ACCEL],&fdmData[Z_ACCEL],
	&fdmData[ROLL_RATE],&fdmData[PITCH_RATE],&fdmData[YAW_RATE]);

    //print data to screen for monitoring purpose
    sprintf(txr_data,"\nSensor Data\nLAT %f\nLON %f\nALT %f\nAIRSPEED %f\nHEAD %f\nX_accl%f\nY_accel%f\nZ_accel%f\nP_rate%f\nR_rate%f\nY_rate%f\n",
	_getData.latitude,fdmData[LONGITUDE],fdmData[ALTITUDE],
	fdmData[AIRSPEED],fdmData[HEADING],
	fdmData[X_ACCEL],fdmData[Y_ACCEL],fdmData[Z_ACCEL],
	fdmData[ROLL_RATE],fdmData[PITCH_RATE],fdmData[YAW_RATE]);
}

/*********************************************
*********************************************/
/*
	GPS	
1-> latitude
2-> longitude
3-> altitude
	
	SPEED
4-> air speed

	HEADING
5-> heading
 
	IMU
6-> x accel
7-> y accel
8-> z accel
9-> roll rate
10-> pitch rate
11> yaw rate
	
*/

void *funcNAV(void *threadp)
{
    double event_time, run_time=0.0;
    int limit=0, release=0, cpucore, i;
    threadParams_t *threadParams=(threadParams_t*)threadp;
    unsigned int requred_test_cycles = 200;
    numBytes_out = strlen(buf);
    
    while(!abortTest)
    {
       sem_wait(&semNAV);
       
       if(abortTest)
         break;
       else
       	release++;
       	
       	cpucore = sched_getcpu();get_time_NAV = getTimeMsec();
       	printf("funcNAV start %d @ %lf on core %d\n",release,(event_time=getTimeMsec()-start_time),limit);
       	
       	
       	do
       	{
              bytes_read = recvfrom(sock_in,rcv_data,1024,0,(struct sockaddr*)&client_addr_in,&addr_len);
              rcv_data[bytes_read] = '\0';
   
              // print where it got the UDP data from and the raw data
              //printf("\n(%s,%d)said:",inet_ntoa(client_addr_in.sin_addr),ntohs(client_addr_in.sin_port));
              syslog(LOG_INFO,"\n(%s,%d)said:",inet_ntoa(client_addr_in.sin_addr),ntohs(client_addr_in.sin_port));
//              printf("DATA: %s/n/r",rcv_data);

    //parse UDP data and store into float array
    sscanf(rcv_data,"%f%f%f%f%f%f%f%f%f%f%f",
	&_getData.latitude,&_getData.longitude,&_getData.altitude,
	&_getData.airspeed,&_getData.heading,
	&_getData.x_accel,&_getData.y_accel,&_getData.z_accel,
	&_getData.roll_rate,&_getData.pitch_rate,&_getData.yaw_rate);
	
    //print data to screen for monitoring purpose
/*
    sprintf(txr_data,"\nSensor Data\nLAT %f\nLON %f\nALT %f\nAIRSPEED %f\nHEAD %f\nX_accl%f\nY_accel%f\nZ_accel%f\nP_rate%f\nR_rate%f\nY_rate%f\n",
	_getData.latitude,_getData.longitude,_getData.altitude,
	_getData.airspeed,_getData.heading,
	_getData.x_accel,_getData.y_accel,_getData.z_accel,
	_getData.roll_rate,_getData.pitch_rate,_getData.yaw_rate);
*/

    sprintf(txr_data,"%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n",
	_getData.latitude,_getData.longitude,_getData.altitude,
	_getData.airspeed,_getData.heading,
	_getData.x_accel,_getData.y_accel,_getData.z_accel,
	_getData.roll_rate,_getData.pitch_rate,_getData.yaw_rate);
	
	
        if(sendto(sock_out,txr_data,strlen(txr_data),0,(struct sockaddr*)&client_addr_out,sizeof(struct sockaddr_in))!=numBytes_out)
              perror("sendto");
        else
             syslog(LOG_INFO,"Send data Success\n");//printf("Send data Success\n");
                  
        set_servo_pwm(_getData.pitch_rate);
        syslog(LOG_INFO,"Apply data OK\n");//printf("Apply data OK\n");
          

       	}while(limit != 0);
       	
        printf("funcNAV complete %d delta time: %lf, %d loops\n",release,(event_time=getTimeMsec()-get_time_NAV),limit);
        limit=0;    
    }
    
    pthread_exit((void*)0);
}


/*********************************************
*********************************************/

void *funcCTL(void *threadp)
{
    double event_time, run_time=0.0;
    double cont = 0.0;
    struct controlFG cdata;
    int limit=0, release=0, cpucore, i;
    threadParams_t *threadParams=(threadParams_t*)threadp;
    unsigned int _elevator = 0;
    unsigned int _aileron = 0;
    unsigned int _throttle = 0;
    unsigned int _rudder = 0;
    
    char buf_send[10] = "Hola\n";
//      numBytes_out = strlen(buf);
      char data_to_uc[1024]; 
    
    while(!abortTest)
    {
       sem_wait(&semCTL);
       
       if(abortTest)
         break;
       else
       	release++;
       	
       	cpucore = sched_getcpu();
       	syslog(LOG_INFO,"funcCTL start %d @ %lf on core %d\n",release,(event_time=getTimeMsec()-start_time),limit); 	
       	do
       	{
       	
       	    read(fd_uc, data_to_uc, 32);
       	    printf("Data from microcontroller %s\n\r",data_to_uc);
       // 	ELEVATOR 	\ 	AILERON 	\ 	RUDDER 	\ 	THROTTLE
      sscanf(data_to_uc,"%d %d %d %d",&_elevator,&_aileron,&_rudder,&_throttle);
       	
      printf("Elevator %d, Aileron %d, Rudder %d,Throttle %d\n\t", _elevator, _aileron, _rudder, _throttle);
       	    
      cdata.aileron = 2*(3300 - (double)_aileron)/3300-1;
      cdata.rudder = 2*(3300 - (double)_rudder)/3300-1;
      cdata.throttle = (double)_throttle/255;
      cdata.elevator = 2*(3300 - (double)_elevator)/3300-1;
      
      printf("Send data control Elevator %03f, Aileron %03f, Rudder %03f, Throttle %03f\n\t",cdata.elevator, cdata.aileron, cdata.rudder, cdata.throttle);
      
      swap64(&cdata.aileron);
      swap64(&cdata.rudder);
      swap64(&cdata.throttle);
      swap64(&cdata.elevator);
       	    
       	    if(sendto(sock_in,&cdata,sizeof(cdata),0,(struct sockaddr*)&fgAddr,sizeof(struct sockaddr_in)) < 0)
       	          printf("Erro to send control data\n");
              
             syslog(LOG_INFO,"Send data control Success\n");//printf("Send data Success\n");
       	    cont++;
       	    if (cont >=200)
       	         cont = 0;
       	         
       	    printf("Apply data OK %lu\n", sizeof(cdata));
          
       	}while(limit != 0);
       	
        syslog(LOG_INFO,"funcCTL complete %d @ %lf, %d loops\n",release,(event_time=getTimeMsec()-start_time),limit);
        limit=0;    
    }
    
    pthread_exit((void*)0);
}


/*********************************************
*********************************************/

//		Sequencer

void *sequencer(void *threadp)
{

  int i;
  int MajorPeriodCnt = 0;
  double event_time;
  
  threadParams_t *threadParams = (threadParams_t *)threadp;
  
  printf("Starting sequencer:[S1, T1=20, C1=10],[S2, T2=50, C2=20],u=0.9,LCM=100\n");
  start_time = getTimeMsec();
  
  do
  {
      printf("\n****CI t=%lf\n",event_time=getTimeMsec()-start_time);
      sem_post(&semNAV);sem_post(&semCTL);
      
      usleep(20*USER_PER_MSEC);sem_post(&semCTL);
      printf("t=%lf\n",event_time=getTimeMsec()-start_time);
      
//      usleep(20*USER_PER_MSEC);sem_post(&semCTL);
//      printf("t=%lf\n",event_time=getTimeMsec()-start_time);
      
      usleep(20*USER_PER_MSEC);sem_post(&semNAV);
      printf("t=%lf\n",event_time=getTimeMsec()-start_time);
      
//      usleep(20*USER_PER_MSEC);sem_post(&semCTL);
//      printf("t=%lf\n",event_time=getTimeMsec()-start_time);
      
      usleep(20*USER_PER_MSEC);
      MajorPeriodCnt ++;
      
  }while(1);
  //while(MajorPeriodCnt < threadParams->majorPeriods);
  
  abortTest = 1;
  sem_post(&semNAV);sem_post(&semCTL);
}

/*********************************************
*********************************************/

//	get time 
double getTimeMsec()
{
  struct timespec event_ts = {0,0};
  
  clock_gettime(CLOCK_MONOTONIC, &event_ts);
  return ((event_ts.tv_sec)*1000.0)+((event_ts.tv_nsec)/10000000.0);

}

//	print scheduler
void print_scheduler()
{
   int schedType;
   
   schedType = sched_getscheduler(getpid());
   
   switch(schedType)
   {
       case SCHED_FIFO:
       	    printf("pthread policy is SCHED_FIFO\n");
       	    break;
       case SCHED_OTHER:
       	    printf("pthread policy is SCHED_OTHER\n");
       	    break;
       case SCHED_RR:
       	    printf("pthread policy is SCHED_RR\n");
       	    break;
       default:
            printf("Pthread policy UNKNOWN\n");
   }
}

int main()
{

  system("clear");  
  
  printf("\n\n\n");
  
  int i, rc, scope;
  cpu_set_t threadcpu;
  pthread_t threads[NUM_THREADS];
  threadParams_t threadParams[NUM_THREADS]; 
  pthread_attr_t rt_sched_attr[NUM_THREADS];
  pthread_attr_t main_attr;
  int rt_max_prio, rt_min_prio;
  pid_t mainpid;
  cpu_set_t allcpuset;
  
  abortTest = 0; 
  
  struct sched_param rt_param[NUM_THREADS];
  struct sched_param main_param; 
  
  printf("system has %d processors configured and %d available.\n", get_nprocs_conf(),get_nprocs());
  
  CPU_ZERO(&allcpuset);
  for(i=0;i<NUM_THREADS;i++)
  	CPU_SET(i,&allcpuset);
  
  if(&semCTL,0,0){printf("Failed o initializate semCTL semaphore\n");exit(-1);}
  if(&semNAV,0,0){printf("Failed o initializate semNAV semaphore\n");exit(-1);}
  
  mainpid = getpid();
  rt_max_prio = sched_get_priority_max(SCHED_FIFO);
  rt_min_prio = sched_get_priority_min(SCHED_FIFO);
  
  rc=sched_getparam(mainpid,&main_param);
  main_param.sched_priority = rt_max_prio;
  rc = sched_setscheduler(getpid(),SCHED_FIFO,&main_param);
  if(rc<0)perror("main_param");
  
  print_scheduler();
  
  pthread_attr_getscope(&main_attr,&scope);
  
  if(scope == PTHREAD_SCOPE_SYSTEM)
  	printf("PTHREAD_SCOPE_SYSTEM\n");
  else if(scope == PTHREAD_SCOPE_PROCESS)
  	printf("PTHREAD_SCOPE_PROCESS");
  else
	printf("PTHREAD_SCOPE_UNKNOWN");

  printf("rt_max_prio: %d\n",rt_max_prio);
  printf("rt_min_prio: %d\n",rt_min_prio);
  
  for(i=0; i<NUM_THREADS; i++)
  {
  	CPU_ZERO(&threadcpu);
  	CPU_SET(0,&threadcpu);
  	
  	rc = pthread_attr_init(&rt_sched_attr[i]);
  	rc = pthread_attr_setinheritsched(&rt_sched_attr[i],PTHREAD_EXPLICIT_SCHED);
  	rc = pthread_attr_setschedpolicy(&rt_sched_attr[i],SCHED_FIFO); 
  	rc = pthread_attr_setaffinity_np(&rt_sched_attr[i],sizeof(cpu_set_t),&threadcpu);
  	
  	rt_param[i].sched_priority=rt_max_prio-i;
  	pthread_attr_setschedparam(&rt_sched_attr[i],&rt_param[i]);
  	
  	threadParams[i].threadidx=i;
  }
  
  printf("Service threads will run on %d CPU cores\n",CPU_COUNT(&threadcpu));
    
  init_getData();
  usleep(300000); 
  init_sendData();

  printf("\n\n---------------------------------\n");
  printf("\t\t\t\t\nINIT CONTOL SYSTEm uav\n");
  printf("\n---------------------------------\n\n");
  
  usleep(300000);
  printf("INIT .... \n\n");
  
//  threadParams[0].majorPeriods=300;
threadParams[0].majorPeriods=0;
  config_servo_pwm();
  config_microcontroller();
  
  rc = pthread_create(&threads[1],&rt_sched_attr[1]/*(void*)0*/,funcCTL,(void*)&(threadParams[1]));
  rc = pthread_create(&threads[2],&rt_sched_attr[2]/*(void*)0*/,funcNAV,(void*)&(threadParams[2]));
  
  usleep(300000);  
  
  rc = pthread_create(&threads[0],&rt_sched_attr[0]/*(void*)0*/,sequencer,(void*)&(threadParams[0]));
  
//  pthread_join(threads[0],NULL);
//  pthread_join(threads[2],NULL);

  for(i=0; i<NUM_THREADS;i++)
  {
     pthread_join(threads[i],NULL);
  }
  
  printf("\nEND .... \n\n");
  return 0;
}
