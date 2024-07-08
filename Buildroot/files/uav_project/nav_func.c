#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <nav_func.h>


Matrix euler2DCM(double *euler, Matrix DCM)
{

    double cPHI,sPHI,cTHE,sTHE,cPSI,sPSI;
    
    cPHI = cos(euler[0]); sPHI = sin(euler[0]);
    cTHE = cos(euler[1]); sTHE = sin(euler[1]);
    cPSI = cos(euler[2]); sPSI = sin(euler[2]); 

    DCM.data[0][0]= cTHE*cPSI; 			DCM.data[0][1]= cTHE*sPSI; 			DCM.data[2][0]= -sTHE;
    DCM.data[1][0]= sPHI*sTHE*cPSI-cPHI*sPSI;	DCM.data[1][1]= sPHI*sTHE*sPSI+cPHI*cPSI;	DCM.data[2][1]= sPHI*cTHE;
    DCM.data[2][0]= cPHI*sTHE*cPSI+sPHI*sPSI;	DCM.data[2][1]= cPHI*sTHE*sPSI-sPHI*cPSI;	DCM.data[2][2]= cPHI*cTHE;

    return DCM;
}


double DCM2quat(Matrix DCM, double *quat)
{
   double q1,q2,q3,q4;
   q1 = 0.5*sqrt(1+DCM.data[0][0]+DCM.data[1][1]+DCM.data[2][2]);
   q2 = (DCM.data[2][1]-DCM.data[1][2])/(4*q1);
   q3 = (DCM.data[0][2]-DCM.data[2][0])/(4*q1);
   q4 = (DCM.data[1][0]-DCM.data[0][1])/(4*q1);
   
   quat[0]=q1; quat[1]=q2; quat[2]=q3; quat[3]=q4;
   
   return (*quat);
}

double euler2quat(double *euler, double *quat)
{

    double roll, pitch, yaw;
    
    roll = euler[0]; pitch = euler[1]; yaw = euler[2];

    double cr = cos(roll * 0.5);
    double sr = sin(roll * 0.5);
    double cp = cos(pitch * 0.5);
    double sp = sin(pitch * 0.5);
    double cy = cos(yaw * 0.5);
    double sy = sin(yaw * 0.5);

    quat[0] = cr * cp * cy + sr * sp * sy;
    quat[1] = sr * cp * cy - cr * sp * sy;
    quat[2] = cr * sp * cy + sr * cp * sy;
    quat[3] = cr * cp * sy - sr * sp * cy;

    return (*quat);
}

Matrix quat2DCM(double *quat, Matrix DCM)
{
   
    double al,qx,qy,qz;
   
    al = quat[0]; qx = quat[1];  qy = quat[2];  qz = quat[3]; 
   
    DCM.data[0][0]= al*al+qx*qx-qy*qy-qz*qz;	DCM.data[0][1]= 2*(qx*qy+al*qz);		DCM.data[0][2]= 2*(qx*qz-qy*al);
    DCM.data[1][0]= 2*(qx*qy-qz*al);		DCM.data[1][1]= al*al-qx*qx+qy*qy-qz*qz;	DCM.data[1][2]= 2*(qy*qz+al*qx);
    DCM.data[2][0]= 2*(qx*qz+al*qy);		DCM.data[2][1]= 2*(qy*qz-al*qx);		DCM.data[2][2]= al*al-qx*qx-qy*qy+qz*qz;
   
   return (DCM);
}


double quat2euler(double *quat, double *euler)
{

    double roll, pitch, yaw;
    double q0,q1,q2,q3;
    double M23, M33, M13, M12, M11;
    
    q0 = quat[0]; q1 = quat[1];  q2 = quat[2];  q3 = quat[3];

/*    
    M23 = 2*q3*q4 + 2*q1*q2; 		//2*(q1*q3+q2*q4);
    M33 = 2*q1*q1 + 2*q4*q4 -1 ;	//q1*q1-q2*q2-q3*q3+q4*q4;
    M13 = 2*q2*q4 - 2*q1*q3;		//2*(q3*q4-q1*q2);
    M12 = 2*q2*q3 + 2*q1*q4;		//2*(q2*q3-q1*q4);
    M11 = 2*q1*q1 + 2*q2*q2-1;		//q1*q1+q2+q2-q3*q3-q4*q4;

*/


    M23 = 2*(q2*q3+q0*q1);
    M33 = q0*q0-q2*q2-q1*q1+q3*q3;
    M13 = 2*(q1*q3-q2*q0);
    M12 = 2*(q1*q2-q0*q3);
    M11 = q1*q1+q0*q0-q3*q3-q2*q2;

   roll=atan2(M23,M33);
   pitch = atan2(-M13,sqrt(1-M13*M13));
   yaw = atan2(M12,M11);

    euler[0]=roll; euler[1]=pitch; euler[2]=yaw;

    return (*euler);
}


Matrix DCM_Angular(double *gyro, Matrix qDCM)
{

    double Wx, Wy, Wz;

     Wx=gyro[0]*0.5; Wy= gyro[1]*0.5; Wz=gyro[2]*0.5;
     
     qDCM.data[0][0]=0.0; qDCM.data[0][1]=-Wx; qDCM.data[0][2]=-Wy; qDCM.data[0][3]=-Wz;
     qDCM.data[1][0]=Wx;  qDCM.data[1][1]=0.0; qDCM.data[1][2]=Wz;  qDCM.data[1][3]=-Wy;
     qDCM.data[2][0]=Wy;  qDCM.data[2][1]=-Wz; qDCM.data[2][2]=0.0; qDCM.data[2][3]=Wx;
     qDCM.data[3][0]=Wz;  qDCM.data[3][1]=Wy;  qDCM.data[3][2]=-Wx; qDCM.data[3][3]=0.0;
     
    return (qDCM);

}


Matrix DCM_AngularNED(double *nav, Matrix qDCM)
{

     double Wx, Wy, Wz, dot_psi,dot_lambda;
     
     dot_psi = nav[1]/((RE(nav[3])+nav[5])*cos(nav[3]));
     dot_lambda = -nav[0]/(RN(nav[3])+nav[5]);

     Wx=0.5*(omega+dot_psi)*cos(nav[3]); Wy= 0.5*dot_lambda ; Wz=0.5*(omega+dot_psi)*sin(nav[3]); 
     
     qDCM.data[0][0]=0.0;	qDCM.data[0][1]=Wx;  	qDCM.data[0][2]=Wy;  	qDCM.data[0][3]=Wz;
     qDCM.data[1][0]=-Wx;  	qDCM.data[1][1]=0.0; 	qDCM.data[1][2]=Wz;  	qDCM.data[1][3]=-Wy;
     qDCM.data[2][0]=-Wy;  	qDCM.data[2][1]=-Wz; 	qDCM.data[2][2]=0.0; 	qDCM.data[2][3]=Wx;
     qDCM.data[3][0]=-Wz;  	qDCM.data[3][1]=Wy;  	qDCM.data[3][2]=-Wx; 	qDCM.data[3][3]=0.0;
     
    return (qDCM);

}

double rungeKuttaAttitude(Matrix DCM, double *gyro, double *nav, double *quat, double delta, double *newquat)
{

   //printf("\n-------------\n");
   
   double antquat[4];
   
   Matrix qDCMb, qDCMned;
   
   qDCMb = create_mat(4,4); 
   qDCMned = create_mat(4,4);
 
   double k1[4], k2[4], k3[4], k4[4];
   double q1[4], q2[4], q3[4];
   double gyro1[3], gyro2[3];
   
   double normquat, normnewquat;
   double normk1, normk2, normk3, normk4;

   normquat = 1; //norm(quat, 4);
   antquat[0]=quat[0]/normquat; antquat[1]=quat[1]/normquat;
   antquat[2]=quat[2]/normquat; antquat[3]=quat[3]/normquat;

   
   DCM_Angular(gyro,qDCMb); 
   DCM_AngularNED(nav,qDCMned);
   mat_sum(qDCMb, qDCMned, DCM);
   
   // Q1  
   
   mat_dot_vector(DCM,antquat, k1);  
   normk1 = 1; //norm(k1, 4);
   
   for (int i=0; i<4; i++)
   {  
      k1[i] = delta*k1[i]/normk1;
      q1[i] = antquat[i]+0.5*k1[i];
   }
     
   // Q2  
 
   mat_dot_vector(DCM, q1, k2);
   normk2 = 1; //norm(k2, 4);
   
   for (int i=0; i<4; i++)
   {
      k2[i] = delta*k2[i]/normk2;
      q2[i] = antquat[i]+0.5*k2[i];
   }

   //Q3    
   mat_dot_vector(DCM,q2, k3);
   normk3 = 1; //norm(k3, 4);
   
   for (int i=0; i<4; i++)
   {
      k3[i] = delta*k3[i]/normk3;
      q3[i] = antquat[i]+k3[i];
   }
   
   //Q4
   
   mat_dot_vector(DCM,q3, k4);
   normk4 = 1; //norm(k4, 4);
    for (int i=0; i<4; i++)
   {
      newquat[i] = antquat[i]+(k1[i] + 2*k2[i]+ 2*k3[i] + delta*k4[i]/normk4)/6;
      //printf("%f ",newquat[i]);
   }

    normnewquat = 1; //norm(newquat,4);

    newquat[0]=newquat[0]/normnewquat; newquat[1]=newquat[1]/normnewquat;
    newquat[2]=newquat[2]/normnewquat; newquat[3]=newquat[3]/normnewquat;   

    return (*newquat);
   
}


double navEquations(double *accel_NED, double *nav,double *dot_nav)
{

   double Vn, Ve, Vd, Asn, Ase, Asd;
   double lat, lon, alt;
   Asn = accel_NED[1]; Ase = accel_NED[2]; Asd = accel_NED[3];
   Vn = nav[0]; Ve = nav[1]; Vd = nav[2];
   lat = nav[3]; lon = nav[4]; alt = nav[5];
   
   dot_nav[0] = Asn + Vn/(RN(lat)+alt)*Vd-(2*omega*sin(lat)+Ve/(RE(lat)+alt)*tan(lat))*Ve;
   dot_nav[1] = Ase + (2*omega*sin(lat)+Ve/(RE(lat)+alt)*tan(lat))*Vn+(2*omega*cos(lat)+Ve/(RE(lat)+alt)*tan(lat))*Vd;
   dot_nav[2] = Asd - (2*omega*cos(lat)+Ve/(RE(lat)+alt)*tan(lat))*Ve-Vn/(RN(lat)+alt)*Vn+go*(1+0.0052884*sin(lat)*sin(lat))*(1-2*alt/Re(lat));
   dot_nav[3] = Vn/(RN(lat)+alt);
   dot_nav[4] = Ve/((Re(lat)+alt)*cos(lat));
   dot_nav[5] = -Vd;

}

double rungeKuttaNavigation(double *accel, double *nav, double *quat, double delta, double *newnav)
{

   double accel_NED[4], accel_B[4], quat4[4];
   double k1[6], k2[6], k3[6], k4[6];
   double q1[6], q2[6], q3[6];
   
   double nav1[6], nav2[6];

   accel_B[0]=0.0; accel_B[1]= accel[0]; accel_B[2]= accel[1]; accel_B[3]=accel[2];

   divQuat(accel_B,quat,quat4);
   prodQuat(quat,quat4,accel_NED);
   
   
   for (int i=0; i<6; i++)
   {  
      nav1[i] = nav[i]+0.5*delta;
      nav2[i] = nav[i]+delta;
   }
   
   //printf("\n\r-----------ACCEL NED ------\n\r");

   // Q0
   
   navEquations(accel_NED,nav,k1);  
   
   for (int i=0; i<4; i++)
   {  
      k1[i] = delta*k1[i];
      q1[i] = nav[i]+0.5*k1[i];
   }
    
   // Q1    
   navEquations(accel_NED,nav,k2); 
   
   
   for (int i=0; i<4; i++)
   {
      k2[i] = delta*k2[i];
      q2[i] = nav[i]+0.5*k2[i];
   }

   //Q2
   navEquations(accel_NED,nav,k3); 

   
   for (int i=0; i<4; i++)
   {
      k3[i] = delta*k3[i];
      q3[i] = nav[i]+k3[i];
   }
   
   //Q3
   
   navEquations(accel_NED,nav,k4); 
 
    for (int i=0; i<6; i++)
   {
      newnav[i] = nav[i]+(k1[i] + 2*k2[i]+ 2*k3[i] + delta*k4[i])/6;
      //printf("%f ",newnav[i]);
   }

    //printf("\n\r-----------------\n\r");

    return (*newnav);
}


