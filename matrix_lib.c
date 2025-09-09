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

    for(unsigned long int i = 0; i < matrixA->height; i++){
        for(unsigned long int k = 0; k < matrixA->width; k++){
            
            float aik = matrixA->rows[i * matrixA->width + k]; // Escalar para reuso
            
            for (unsigned long int j = 0; j < matrixB->width; j++){
                // 3. Acesso por LINHA em B (eficiente para o cache)
                matrixC->rows[i * matrixC->width + j] += aik * matrixB->rows[k * matrixB->width + j];
            }
        }
    }
    
    return 1;
}