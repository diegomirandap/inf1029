//INF-1029 - Trabalho 2
//Diego Miranda - 2210996
//Felipe Cancella - 2210487
#include "matrix_lib.h"
#include <immintrin.h>

int scalar_matrix_mult(float scalar_value, struct matrix* matrix){
    if (!matrix || !matrix->rows) return 0;

    unsigned long int total_size = matrix->height * matrix->width;
    // Cria o vetor avx já com o escalar
    __m256 scalar_vec = _mm256_set1_ps(scalar_value);               

    for (unsigned long int i = 0; i <= total_size - 8; i += 8) {
        // Carrega os valores para o vetor avx
        __m256 matrix_vec = _mm256_loadu_ps(&matrix->rows[i]);      
        // Multiplicação dos valores
        __m256 result_vec = _mm256_mul_ps(matrix_vec, scalar_vec);  
        // Atualiza o valor na matrix
        _mm256_storeu_ps(&matrix->rows[i], result_vec);             
    }
    return 1;
}

int matrix_matrix_mult(struct matrix* matrixA, struct matrix* matrixB, struct matrix* matrixC){
    if (!matrixA || !matrixB || !matrixC || !matrixA->rows || !matrixB->rows || !matrixC->rows) return 0;
    if (matrixA->width != matrixB->height || matrixC->height != matrixA->height || matrixC->width != matrixB->width) return 0;
    
    for (unsigned long int i = 0; i < matrixA->height; i++) {
        for (unsigned long int k = 0; k < matrixA->width; k++) {
            // Cria o vetor avx como a base da operação (Aik) em todas as posições do vetor
            __m256 a_vec = _mm256_set1_ps(matrixA->rows[i * matrixA->width + k]); 
            
            for (unsigned long j = 0; j <= matrixB->width - 8; j += 8) {
                // Carrega os valores para o vetor avx
                __m256 b_vec = _mm256_loadu_ps(&matrixB->rows[k * matrixB->width + j]);
                __m256 c_vec = _mm256_loadu_ps(&matrixC->rows[i * matrixC->width + j]);
                
                // Operação de multiplicação e adição unificada de A e B, armazenada em C
                c_vec = _mm256_fmadd_ps(a_vec, b_vec, c_vec); 
                
                // Atualiza o valor na matrix
                _mm256_storeu_ps(&matrixC->rows[i * matrixC->width + j], c_vec); 
            }
        }
    }
    
    return 1;
}