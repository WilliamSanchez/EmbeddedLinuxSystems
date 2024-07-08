#ifndef _DQDATA_H_
#define _DQDATA_H_

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
11> yag rate

*/

typedef struct dqData
{
	float latitude;
	float longitude;
	float altitude;
	float airspeed;
	float heading;
	float x_accel;
	float y_accel;
	float z_accel;
	float roll_rate;
	float pitch_rate;
	float yaw_rate; 
}dqData;



typedef struct attitude
{
	float roll;
	float pitch;
	float yaw; 
}attitude;

#endif
