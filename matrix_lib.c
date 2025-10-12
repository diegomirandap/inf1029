//INF-1029 - Trabalho 3
//Diego Miranda - 2210996
//Felipe Cancella - 2210487
#include "matrix_lib.h"
#include <immintrin.h>
#include <pthread.h>
#include <stdio.h>

static int numThreads = 1;

// Structs para uso interno das threads
typedef struct {
    float scalar_value;
    matrix* matrix;
    int start_row;
    int end_row;
} scalar_mult_args;

typedef struct {
    matrix *matrixA;
    matrix *matrixB;
    matrix *matrixC;
    int start_row;
    int end_row;
} matrix_mult_args;

// Função para multiplicação escalar interna da thread
void* scalar_mult_worker(void* args) {
    scalar_mult_args* thread_args = (scalar_mult_args*) args;

    // Cria o vetor avx já com o escalar
    __m256 scalar_vec = _mm256_set1_ps(thread_args->scalar_value);

    for (unsigned long int i = thread_args->start_row; i < thread_args->end_row; i++) {
        // Calcula a área da respectiva thread
        unsigned long int row_offset = i * thread_args->matrix->width;
        for (unsigned int j = 0; j <= thread_args->matrix->width - 8; j += 8) {
            // Carrega os valores para o vetor avx
            __m256 matrix_vec = _mm256_loadu_ps(&thread_args->matrix->rows[row_offset + j]);
            // Multiplicação dos valores
            __m256 result_vec = _mm256_mul_ps(matrix_vec, scalar_vec);
             // Atualiza o valor na matrix
            _mm256_storeu_ps(&thread_args->matrix->rows[row_offset + j], result_vec);
        }
    }
    pthread_exit(NULL);
}

// Função multiplicação de matrizes interna da thread
void* matrix_mult_worker(void* args) {
    
    matrix_mult_args* thread_args = (matrix_mult_args*) args;

    for (unsigned int i = thread_args->start_row; i < thread_args->end_row; i++) {
        for (unsigned int k = 0; k < thread_args->matrixA->width; k++) {
            // Cria o vetor avx como a base da operação (Aik) em todas as posições do vetor
            __m256 a_vec = _mm256_set1_ps(thread_args->matrixA->rows[i * thread_args->matrixA->width + k]);
            
            for (unsigned int j = 0; j <= thread_args->matrixB->width - 8; j += 8) {
                // Carrega os valores para o vetor avx
                __m256 b_vec = _mm256_loadu_ps(&thread_args->matrixB->rows[k * thread_args->matrixB->width + j]);
                __m256 c_vec = _mm256_loadu_ps(&thread_args->matrixC->rows[i * thread_args->matrixC->width + j]);

                // Operação de multiplicação e adição unificada de A e B, armazenada em C
                c_vec = _mm256_fmadd_ps(a_vec, b_vec, c_vec);

                // Atualiza o valor na matrix
                _mm256_storeu_ps(&thread_args->matrixC->rows[i * thread_args->matrixC->width + j], c_vec);
            }
        }
    }
    pthread_exit(NULL);
}


int scalar_matrix_mult(float scalar_value, struct matrix* matrix){
    if (!matrix || !matrix->rows) return 0;
    
    pthread_t threads[numThreads];
    scalar_mult_args args[numThreads];
    int rows_per_thread = matrix->height / numThreads;

    for (int i = 0; i < numThreads; i++) {
        // Adiciona os argumentos para cada thread
        args[i].scalar_value = scalar_value;
        args[i].matrix = matrix;
        args[i].start_row = i * rows_per_thread;
        args[i].end_row = (i == numThreads - 1) ? matrix->height : (i + 1) * rows_per_thread;

        // Cria a thread
        pthread_create(&threads[i], NULL, scalar_mult_worker, &args[i]);
    }

    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    return 1;
}

int matrix_matrix_mult(struct matrix* matrixA, struct matrix* matrixB, struct matrix* matrixC){
    if (!matrixA || !matrixB || !matrixC || !matrixA->rows || !matrixB->rows || !matrixC->rows) return 0;
    if (matrixA->width != matrixB->height || matrixC->height != matrixA->height || matrixC->width != matrixB->width) return 0;
    
    pthread_t threads[numThreads];
    matrix_mult_args args[numThreads];
    int rows_per_thread = matrixC->height / numThreads;

    for (int i = 0; i < numThreads; i++) {
        // Adiciona os argumentos para cada thread
        args[i].matrixA = matrixA;
        args[i].matrixB = matrixB;
        args[i].matrixC = matrixC;
        args[i].start_row = i * rows_per_thread;
        args[i].end_row = (i == numThreads - 1) ? matrixC->height : (i + 1) * rows_per_thread;

        // Cria a thread
        pthread_create(&threads[i], NULL, matrix_mult_worker, &args[i]);
    }
    
    for (int i = 0; i < numThreads; i++) {
        pthread_join(threads[i], NULL);
    }
      
    return 1;
}

void set_number_threads(int num_threads){
    if (num_threads > 1) {
        numThreads = num_threads;
    } 
}