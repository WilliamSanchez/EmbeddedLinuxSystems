#ifndef _NAV_FUNC_H_
#define _NAV_FUNC_H_

#include <matrices.h>

//#define PI 3.14159265358979	

#define e			0.00335289186		// 1/298.25 
#define Ro			6.378138E6		// [m]
#define omega			7.292115E-5		// rad/s
#define go			9.780327		// m/s²	


#define RE(x)(Ro*(1+e*sin(x)*sin(x))) 
#define RN(x)(Ro*(1-e*(2-3*sin(x)*sin(x))))
#define Re(x)(Ro*(1-e*sin(x)*sin(x))) 
#define foot2meter(x)(x*0.3048)

#define zeros		0
#define ones 		1
#define identity	2
#define undefined	3

Matrix euler2DCM(double *euler, Matrix DCM);
Matrix quat2DCM(double *quat, Matrix DCM);
Matrix DCM_Angular(double *gyro, Matrix qDCM);
Matrix DCM_AngularNED(double *nav, Matrix qDCM);

double DCM2quat(Matrix DCM, double *quat);
double euler2quat(double *euler, double *quat);
double quat2euler(double *quat, double *euler);

double rungeKuttaAttitude(Matrix DCM, double *gyro, double *nav, double *quat, double delta, double *newquat);
double rungeKuttaNavigation(double *accel, double *nav, double *quat, double delta, double *newnav);

double navEquations(double *accel_NED, double *nav,double *dot_nav);

#endif
