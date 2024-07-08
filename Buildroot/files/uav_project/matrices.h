#ifndef _MATRICES_H_
#define _MATRICES_H_

#include <stdio.h>

typedef struct{
    int row;
    int col;
    double **data;
}Matrix;

Matrix create_mat(int row, int col);
Matrix fill_matrix(Matrix mat, int type);


double norm(double *mat, int size);
Matrix mat_sum(Matrix A, Matrix B, Matrix C);
Matrix mat_dot(Matrix A, Matrix B, Matrix C);
Matrix mat_cross(Matrix A, Matrix B, Matrix C);
double mat_dot_vector(Matrix A, double *B, double *C);

double sumQuat(double  *quat1, double *quat2, double *quat3);
double prodQuat(double  *quat1, double *quat2, double *quat3);
double divQuat(double  *quat1, double *quat2, double *quat3);
double invQuat(double  *quat1, double *quat2);

int matrix_free(Matrix mat);
void print_Matrix(Matrix *_matrix);


#endif
