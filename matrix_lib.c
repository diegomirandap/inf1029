//INF-1029 - Trabalho 2
//Diego Miranda - 2210996
//Felipe Cancella - 2210487
#include "matrix_lib.h"
#include <immintrin.h>

int scalar_matrix_mult(float scalar_value, struct matrix* matrix){
    if (!matrix || !matrix->rows) return 0;

    unsigned long int total_size = matrix->height * matrix->width;
    // Cria um vetor AVX onde todas as 8 posições contêm o valor escalar
    __m256 scalar_vec = _mm256_set1_ps(scalar_value);

    unsigned long int i = 0;
    // Processa 8 floats de cada vez
    for (i = 0; i <= total_size - 8; i += 8) {
        // 1. Carrega 8 floats da matriz para um vetor AVX
        __m256 matrix_vec = _mm256_loadu_ps(&matrix->rows[i]);
        // 2. Multiplica os dois vetores
        __m256 result_vec = _mm256_mul_ps(matrix_vec, scalar_vec);
        // 3. Armazena o resultado de volta na matriz
        _mm256_storeu_ps(&matrix->rows[i], result_vec);
    }
    return 1;
}

int matrix_matrix_mult(struct matrix* matrixA, struct matrix* matrixB, struct matrix* matrixC){
    if (!matrixA || !matrixB || !matrixC || !matrixA->rows || !matrixB->rows || !matrixC->rows) return 0;
    if (matrixA->width != matrixB->height || matrixC->height != matrixA->height || matrixC->width != matrixB->width) return 0;
    
    // IMPORTANTE: Esta função assume que a matrixC já foi devidamente inicializada com zeros
    // pelo programa que a chamou.
    
    for (unsigned int i = 0; i < matrixA->height; i++) {
        for (unsigned int k = 0; k < matrixA->width; k++) {
            // Pega o elemento A[i][k] e o replica 8 vezes em um vetor AVX
            __m256 a_vec = _mm256_set1_ps(matrixA->rows[i * matrixA->width + k]);
            
            unsigned int j = 0;
            // Loop interno vetorizado: processa 8 floats da linha de B e C por vez
            for (j = 0; j <= matrixB->width - 8; j += 8) {
                // Carrega 8 floats da linha k de B
                __m256 b_vec = _mm256_loadu_ps(&matrixB->rows[k * matrixB->width + j]);
                // Carrega 8 floats da linha i de C
                __m256 c_vec = _mm256_loadu_ps(&matrixC->rows[i * matrixC->width + j]);
                
                // Operação Fused Multiply-Add: c = (a * b) + c
                c_vec = _mm256_fmadd_ps(a_vec, b_vec, c_vec);
                
                // Armazena o resultado de volta em C
                _mm256_storeu_ps(&matrixC->rows[i * matrixC->width + j], c_vec);
            }
        }
    }
    
    return 1;
}