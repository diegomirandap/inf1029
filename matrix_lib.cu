//INF-1029 - Trabalho 4
//Diego Miranda - 2210996
//Felipe Cancella - 2210487
#include <stdio.h>
#include <cuda_runtime.h>
#include "matrix_lib.h"

static int g_threads_per_block = 256;
static int g_max_blocks_per_grid = 4096;

static int check_cuda_error(cudaError_t err, const char *msg) {
    if (err != cudaSuccess) {
        fprintf(stderr, "Erro CUDA (%s): %s\n", msg, cudaGetErrorString(err));
        return 0;
    }
    return 1;
}


__global__ void scalar_mult_kernel(float scalar, float *d_data, unsigned long int total_elements) {
    //Kernel para scalar_mult FULL e PARTIAL
    unsigned long int idx = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned long int stride = gridDim.x * blockDim.x;

    for (unsigned long int i = idx; i < total_elements; i += stride) {
        d_data[i] = scalar * d_data[i];
    }
}

__global__ void matrix_mult_kernel_full(const float *d_A, const float *d_B, float *d_C, 
    unsigned long int A_width, unsigned long int C_width, 
    unsigned long int C_total_elements) {
    //Kernel para matrix_mult (Modo FULL_ALLOC)
        
    unsigned long int idx = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned long int stride = gridDim.x * blockDim.x;

    for (unsigned long int i = idx; i < C_total_elements; i += stride) {
        int row = i / C_width;
        int col = i % C_width;
        
        float sum = 0.0f;
        for (unsigned long int k = 0; k < A_width; k++) {
            sum += d_A[row * A_width + k] * d_B[k * C_width + col];
        }
        d_C[i] = sum;
    }
}

__global__ void matrix_mult_kernel_partial(const float *d_A_row, const float *d_B_full, float *d_C_row, 
    unsigned long int A_width, unsigned long int C_width) {
    //Kernel para matrix_mult (Modo PARTIAL_ALLOC)
        
    unsigned long int col_idx = blockIdx.x * blockDim.x + threadIdx.x;
    unsigned long int stride = gridDim.x * blockDim.x;

    // Cada thread calcula uma ou mais colunas da linha de resultado
    for (unsigned long int j = col_idx; j < C_width; j += stride) {
        float sum = 0.0f;
        // Produto escalar da linha 'd_A_row' pela coluna 'j' de 'd_B_full'
        for (unsigned long int k = 0; k < A_width; k++) {
            sum += d_A_row[k] * d_B_full[k * C_width + j];
        }
        d_C_row[j] = sum;
    }
}

int set_grid_size(int threads_per_block, int max_blocks_per_grid) {
    const int MAX_THREADS_LIMIT = 1024;
    const int MAX_BLOCKS_LIMIT = 2147483647;

    if (threads_per_block <= 0 || threads_per_block > MAX_THREADS_LIMIT || 
        max_blocks_per_grid <= 0 || max_blocks_per_grid > MAX_BLOCKS_LIMIT) {
        g_threads_per_block = 256;
        g_max_blocks_per_grid = 4096;
        return 0;
    }
    g_threads_per_block = threads_per_block;
    g_max_blocks_per_grid = max_blocks_per_grid;
    return 1;
}

int scalar_matrix_mult(float scalar_value, matrix *matrix) {
    if (!matrix || !matrix->d_rows || !matrix->h_rows) return 0;

    dim3 block(g_threads_per_block);
    
    if (matrix->alloc_mode == FULL_ALLOC) {
        unsigned long int total_elements = matrix->height * matrix->width;
        unsigned int num_blocks = (total_elements + block.x - 1) / block.x;
        dim3 grid(num_blocks > g_max_blocks_per_grid ? g_max_blocks_per_grid : num_blocks);
        
        scalar_mult_kernel<<<grid, block>>>(scalar_value, matrix->d_rows, total_elements);
        
        check_cuda_error(cudaGetLastError(), "Lançamento do kernel scalar_mult (Full)");
    
    } else if (matrix->alloc_mode == PARTIAL_ALLOC) {
        unsigned long int row_bytes = matrix->width * sizeof(float);
        unsigned long int num_elements_per_row = matrix->width;
        
        unsigned int num_blocks = (num_elements_per_row + block.x - 1) / block.x;
        dim3 grid(num_blocks > g_max_blocks_per_grid ? g_max_blocks_per_grid : num_blocks);
        
        // Processa uma linha de cada vez
        for (unsigned int i = 0; i < matrix->height; i++) {
            // 1. Copia a linha do host para o buffer do device
            check_cuda_error(cudaMemcpy(matrix->d_rows, &matrix->h_rows[i * matrix->width], row_bytes, cudaMemcpyHostToDevice), "scalar H->D (Partial)");
            
            // 2. Lança o kernel para processar apenas essa linha
            scalar_mult_kernel<<<grid, block>>>(scalar_value, matrix->d_rows, num_elements_per_row);
            
            // 3. Copia a linha processada de volta para o host
            check_cuda_error(cudaMemcpy(&matrix->h_rows[i * matrix->width], matrix->d_rows, row_bytes, cudaMemcpyDeviceToHost), "scalar D->H (Partial)");
        }
    } else {
        return 0; // Modo de alocação inválido
    }

    return check_cuda_error(cudaDeviceSynchronize(), "Execução do kernel scalar_mult");
}

int matrix_matrix_mult(matrix *matrixA, matrix *matrixB, matrix *matrixC) {
    if (!matrixA || !matrixB || !matrixC || !matrixA->d_rows || !matrixB->d_rows || !matrixC->d_rows) return 0;
    if (matrixA->width != matrixB->height || matrixC->height != matrixA->height || matrixC->width != matrixB->width) return 0;

    dim3 block(g_threads_per_block);

    if (matrixA->alloc_mode == FULL_ALLOC && matrixC->alloc_mode == FULL_ALLOC) {
        unsigned long int C_total_elements = matrixC->height * matrixC->width;
        unsigned int num_blocks = (C_total_elements + block.x - 1) / block.x;
        dim3 grid(num_blocks > g_max_blocks_per_grid ? g_max_blocks_per_grid : num_blocks);

        matrix_mult_kernel_full<<<grid, block>>>(matrixA->d_rows, matrixB->d_rows, matrixC->d_rows,
                                               matrixA->width, matrixC->width, C_total_elements);
        check_cuda_error(cudaGetLastError(), "Lançamento do kernel matrix_mult (Full)");
    
    } else if (matrixA->alloc_mode == PARTIAL_ALLOC && matrixC->alloc_mode == PARTIAL_ALLOC && matrixB->alloc_mode == FULL_ALLOC) {
        unsigned long int a_row_bytes = matrixA->width * sizeof(float);
        unsigned long int c_row_bytes = matrixC->width * sizeof(float);
        unsigned long int num_elements_per_row = matrixC->width;
        
        unsigned int num_blocks = (num_elements_per_row + block.x - 1) / block.x;
        dim3 grid(num_blocks > g_max_blocks_per_grid ? g_max_blocks_per_grid : num_blocks);

        // Processa uma linha de C de cada vez
        for (unsigned int i = 0; i < matrixA->height; i++) {
            // 1. Copia a linha 'i' de A (host) para o buffer 'd_rows' de A (device)
            check_cuda_error(cudaMemcpy(matrixA->d_rows, &matrixA->h_rows[i * matrixA->width], a_row_bytes, cudaMemcpyHostToDevice), "matrix_mult A H->D (Partial)");
            
            // 2. Zera o buffer de C no device (não precisa copiar, pois h_rows já é zero)
            check_cuda_error(cudaMemset(matrixC->d_rows, 0, c_row_bytes), "matrix_mult C memset (Partial)");
            
            // 3. Lança o kernel parcial
            matrix_mult_kernel_partial<<<grid, block>>>(matrixA->d_rows, matrixB->d_rows, matrixC->d_rows,
                                                       matrixA->width, matrixC->width);
            
            // 4. Copia o resultado (linha 'i' de C) do device para o host
            check_cuda_error(cudaMemcpy(&matrixC->h_rows[i * matrixC->width], matrixC->d_rows, c_row_bytes, cudaMemcpyDeviceToHost), "matrix_mult C D->H (Partial)");
        }
    } else {
        return 0; // Combinação de alocação não suportada
    }

    return check_cuda_error(cudaDeviceSynchronize(), "Execução do kernel matrix_mult");
}