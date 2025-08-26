#include "matrix_lib.h"

int scalar_matrix_mult(float scalar_value, struct matrix* matrix){
    unsigned long int size = matrix->height * matrix->width;
    for (unsigned long int i = 0; i < size; i++){
        matrix->rows[i] *= scalar_value;
    }

    return 1;
}

int matrix_matrix_mult(struct matrix* matrixA, struct matrix* matrixB, struct matrix* matrixC){
    if (matrixA->width != matrixB->height || matrixC->height != matrixA->height || matrixC->width != matrixB->width) 
        return 0;

    for(unsigned long int i = 0; i < matrixC->height; i++){
        for(unsigned long int j = 0; j< matrixC->width; j++){
            float sum = 0.0f;
            for (unsigned long int k = 0; k < matrixA->width; k++){
                sum += matrixA->rows[i * matrixA->width + k] * matrixB->rows[k * matrixB->width + j];
            }
            matrixC->rows[i * matrixC->width + j] = sum;
        }

    }
    
    return 1;
}
