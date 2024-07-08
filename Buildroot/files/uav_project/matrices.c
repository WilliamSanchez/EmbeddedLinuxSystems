#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>

#include <matrices.h>

Matrix create_mat(int row, int col)
{
    Matrix mat;
        
    mat.data = (double **)malloc(row*sizeof(double *));
    
    for(int j=0; j<row; j++)
    	*((double **)(mat.data+j)) = (double *)malloc(col * sizeof(double ));
   
    mat.row = row;
    mat.col = col;

    return mat;

}


Matrix mat_cross(Matrix A, Matrix B, Matrix C)
{
    C.data[0][0]= A.data[1][0]*B.data[2][0] - A.data[2][0]*B.data[1][0];
    C.data[1][0]= A.data[2][0]*B.data[0][0] - A.data[0][0]*B.data[2][0];
    C.data[2][0]= A.data[0][0]*B.data[1][0] - A.data[1][0]*B.data[0][0];
    
    return (C);
}

Matrix mat_sum(Matrix A, Matrix B, Matrix C)
{

  int i,j,k;
  
  for(i=0;i<A.row;i++)
  for(j=0;j<A.col;j++)
      C.data[i][j] += A.data[i][j] + B.data[i][j];

   return (C);
}

Matrix mat_dot(Matrix A, Matrix B, Matrix C)
{

  int i,j,k;
  
  for(i=0;i<A.row;i++)
  for(j=0;j<B.col;j++)
  for(k=0, C.data[i][j]=0.0; k<A.col; k++)
      C.data[i][j] += A.data[i][k] * B.data[k][j];

   return (C);
}

double mat_dot_vector(Matrix A, double *B, double *C)
{
  int i,j,k;
  
  for(i=0;i<A.row;i++)
  for(k=0, C[i]=0.0; k<A.col; k++)
      C[i] += A.data[i][k] * B[k];

   return (*C);

}

double norm(double *mat, int size)
{

   double mag = 0.0;
   for(int j=0; j< size; j++)
   	mag += mat[j]*mat[j];
   	
  return sqrt(mag);
}

double sumQuat(double  *quat1, double *quat2, double *quat3)
{
   
   quat3[0] = quat1[0] + quat2[0]; 
   quat3[1] = quat1[1] + quat2[1];
   quat3[2] = quat1[2] + quat2[2];
   quat3[3] = quat1[3] + quat2[3];
    
   return (*quat3);

}

double prodQuat(double  *quat1, double *quat2, double *quat3)
{

   quat3[0] = quat1[0]*quat2[0]-quat1[1]*quat2[1]-quat1[2]*quat2[2]-quat1[3]*quat2[3]; 
   quat3[1] = quat1[0]*quat2[1]+quat1[1]*quat2[0]+quat1[2]*quat2[3]-quat1[3]*quat2[2];
   quat3[2] = quat1[0]*quat2[2]-quat1[1]*quat2[3]+quat1[2]*quat2[0]+quat1[3]*quat2[1];
   quat3[3] = quat1[0]*quat2[3]+quat1[1]*quat2[2]-quat1[2]*quat2[1]+quat1[3]*quat2[0];;
    
   return (*quat3);
   
}

double divQuat(double  *quat1, double *quat2, double *quat3)
{
    double *invQ2 = (double*)malloc(4*sizeof(double));
    invQuat(quat2, invQ2);
    prodQuat(quat1,invQ2,quat3);    
    free(invQ2);
    
    return (*quat3);

}

double invQuat(double  *quat1, double *quat2)
{

   double mag;
   
   mag = quat1[0]*quat1[0]+quat1[1]*quat1[1]+quat1[2]*quat1[2]+quat1[3]*quat1[3]; 

   quat2[0] = quat1[0]/mag; 
   quat2[1] = -quat1[1]/mag;
   quat2[2] = -quat1[2]/mag;
   quat2[3] = -quat1[3]/mag;
    
   return (*quat2);

}

Matrix fill_matrix(Matrix mat, int type)
{
	
    switch(type)
    {
    	case 0:
    		for(int j=0; j< mat.row; j++)
      		  for(int i=0; i< mat.col; i++)
      		     *(*(mat.data+j)+i) = 0.0; // mat.data[j][i]
		break;
    	case 1:    	
    	    	for(int j=0; j< mat.row; j++)
      		  for(int i=0; i< mat.col; i++)
      		    *(*(mat.data+j)+i) = 1.0;  // mat.data[j][i]
		break;
    	case 2:
    	    	for(int j=0; j< mat.row; j++)
    	    	{
      		  for(int i=0; i< mat.col; i++)
      		  {
      		     if(i==j)
      	 	     	*(*(mat.data+j)+i) = 1.0;
      	 	     else
      	 	     	*(*(mat.data+j)+i) = 0.0;
      	 	   }
      	 	}
		break;
    	
    	default:
    		for(int j=0; j< mat.row; j++)
      		  for(int i=0; i< mat.col; i++)
      		    *(*(mat.data+j)+i) = (j+1)*10+i+1;
		break;
    
    }
    
    return (mat);
}


int matrix_free(Matrix mat)
{
    if (mat.data == NULL)
       return 0;
       
    for(int j=0; j < mat.col; j++)
        free(*(mat.data+j));
    free(mat.data);
    
    return 1;

}


void print_Matrix(Matrix *_matrix)
{
    for(int i=0; i<_matrix->row; i++)
    { 
 	for(int j=0; j<_matrix->col; j++)
 	{
 		printf("%f ",_matrix->data[i][j]);
 	}
 	printf("\n\r");
    } 
 }
