//INF-1029 - Trabalho 4
//Diego Miranda - 2210996
//Felipe Cancella - 2210487
// nvcc -o matrix_lib_test matrix_lib_test.cu matrix_lib.cu timer.c
// ./matrix_lib_test 5.0 1024 1024 1024 1024 256 4096 1024 floats_256_2.0f.dat floats_256_5.0f.dat result1.dat result2.dat
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>     
#include <cuda_runtime.h>
#include "matrix_lib.h"   
extern "C" {
#include "timer.h"        
}

static int check_cuda_error(cudaError_t err, const char *msg) {
    if (err != cudaSuccess) {
        fprintf(stderr, "Erro CUDA (%s): %s\n", msg, cudaGetErrorString(err));
        return 0; // Indica erro
    }
    return 1; // Indica sucesso
}

matrix* create_matrix(unsigned int height, unsigned int width) {
    // Aloca estrutura da matriz
    matrix* mat = (matrix*) malloc(sizeof(matrix));
    if (!mat) return NULL;
    mat->height = height;
    mat->width = width;
    mat->alloc_mode = -1;
    mat->d_rows = NULL;   // Device não alocado ainda
    // Aloca memória no host
    mat->h_rows = (float*) malloc(height * width * sizeof(float));
    if (!mat->h_rows) {
        free(mat);
        return NULL;
    }
    return mat;
}

void free_matrix(matrix* mat) {
    if (mat) {
        if (mat->h_rows) free(mat->h_rows);
        if (mat->d_rows) cudaFree(mat->d_rows); // Libera memória da GPU
        free(mat);
    }
}

void init_matrix_zeros(matrix* mat) {
    if (!mat || !mat->h_rows) return;
    unsigned long int total_size = mat->height * mat->width;
    for (unsigned long int i = 0; i < total_size; i++) {
        mat->h_rows[i] = 0.0f;
    }
}

void init_matrix_from_file(matrix* mat, const char* filename) {
    // Carrega dados de um arquivo
    FILE* file = fopen(filename, "rb");
    if (!file) { perror("Erro ao ler arquivo"); exit(1); }
    fread(mat->h_rows, sizeof(float), mat->height * mat->width, file);
    fclose(file);
}

void save_matrix_to_file(matrix* mat, const char* filename) {
    // Salva dados em um arquivo
    FILE* file = fopen(filename, "wb");
    if (!file) { perror("Erro ao salvar arquivo"); exit(1); }
    fwrite(mat->h_rows, sizeof(float), mat->height * mat->width, file);
    fclose(file);
}

void print_first_256(const char* name, matrix* mat) {
    printf("---------- %s - Primeiros 256 elementos ----------\n", name);
    if (!mat || !mat->h_rows) { printf("Matriz Nula.\n\n"); return; }
    
    unsigned long int limit = mat->height * mat->width;
    if (limit > 256) limit = 256;

    for (unsigned int i = 0; i < limit; i++) {
        printf("%.2f ", mat->h_rows[i]);
        if ((i + 1) % mat->width == 0) printf("\n");
    }
    printf("\n------------------------------------------------------------\n");
}



int main(int argc, char* argv[]) {
    if (argc != 13) {
        printf("Erro na execução do arquivo: matrix_lib_test <scalar> <heightA> <widthA> <heightB> <widthB> <threads/blk> <max_blks> <max_mem_MiB> <floatsFile1> <floatsFile2> <resultFile1> <resultFile2>\n");
        return 1;
    }

    struct timeval start_scalar, stop_scalar, start_mult, stop_mult, overall_t1, overall_t2;
    gettimeofday(&overall_t1, NULL); 

    float scalar = atof(argv[1]);
    unsigned int heightA = atoi(argv[2]);
    unsigned int widthA = atoi(argv[3]);
    unsigned int heightB = atoi(argv[4]);
    unsigned int widthB = atoi(argv[5]);
    int threads_per_block = atoi(argv[6]);
    int max_blocks = atoi(argv[7]);
    unsigned long max_gpu_mem_mib = atol(argv[8]);
    const char* floats1 = argv[9];
    const char* floats2 = argv[10];
    const char* results1 = argv[11];
    const char* results2 = argv[12]; 

    if (widthA != heightB) {
        fprintf(stderr, "Erro de dimensão: Largura de A (%u) != Altura de B (%u).\n", widthA, heightB);
        return 1;
    }

    matrix* matA = create_matrix(heightA, widthA);
    matrix* matB = create_matrix(heightB, widthB);
    matrix* matC = create_matrix(heightA, widthB);
    if (!matA || !matB || !matC) {
        fprintf(stderr, "Erro: Falha ao alocar memória no host.\n"); return 1;
    }

    init_matrix_from_file(matA, floats1);
    init_matrix_from_file(matB, floats2);
    init_matrix_zeros(matC); // Inicializa C com zeros 

    // Lógica de Alocação na GPGPU
    unsigned long a_bytes = heightA * widthA * sizeof(float);
    unsigned long b_bytes = heightB * widthB * sizeof(float);
    unsigned long c_bytes = heightA * widthB * sizeof(float);
    unsigned long max_gpu_bytes = max_gpu_mem_mib * 1024 * 1024;
    unsigned long total_full_bytes = a_bytes + b_bytes + c_bytes;
    int cuda_alloc_ok = 1;

    // FULL_ALLOC
    if (total_full_bytes <= max_gpu_bytes) {
        printf("Tentando alocação FULL_ALLOC (%lu MiB)...\n", total_full_bytes / 1024 / 1024);
        cuda_alloc_ok &= check_cuda_error(cudaMalloc(&matA->d_rows, a_bytes), "cudaMalloc A");
        cuda_alloc_ok &= check_cuda_error(cudaMalloc(&matB->d_rows, b_bytes), "cudaMalloc B");
        cuda_alloc_ok &= check_cuda_error(cudaMalloc(&matC->d_rows, c_bytes), "cudaMalloc C");

        if (cuda_alloc_ok) {
            matA->alloc_mode = FULL_ALLOC;
            matB->alloc_mode = FULL_ALLOC;
            matC->alloc_mode = FULL_ALLOC;
            printf("Alocação GPU: FULL_ALLOC bem-sucedida.\n");
        } else {
            // Limpa em caso de falha parcial
            free_matrix(matA); free_matrix(matB); free_matrix(matC);
            // Recria matrizes do host para tentar PARTIAL_ALLOC
            matA = create_matrix(heightA, widthA); matB = create_matrix(heightB, widthB); matC = create_matrix(heightA, widthB);
            init_matrix_from_file(matA, floats1); init_matrix_from_file(matB, floats2); init_matrix_zeros(matC);
            printf("Alocação FULL_ALLOC falhou, tentando PARTIAL_ALLOC...\n");
        }
    }

    // PARTIAL_ALLOC
    if (matA->alloc_mode == -1) { // Se a alocação total não foi tentada ou falhou
        unsigned long a_row_bytes = widthA * sizeof(float);
        unsigned long c_row_bytes = widthB * sizeof(float);
        unsigned long total_partial_bytes = b_bytes + a_row_bytes + c_row_bytes;

        if (total_partial_bytes <= max_gpu_bytes) {
            printf("Tentando alocação PARTIAL_ALLOC (%lu MiB)...\n", total_partial_bytes / 1024 / 1024);
            cuda_alloc_ok = 1;
            cuda_alloc_ok &= check_cuda_error(cudaMalloc(&matB->d_rows, b_bytes), "cudaMalloc B (Partial)");
            cuda_alloc_ok &= check_cuda_error(cudaMalloc(&matA->d_rows, a_row_bytes), "cudaMalloc A-row (Partial)");
            cuda_alloc_ok &= check_cuda_error(cudaMalloc(&matC->d_rows, c_row_bytes), "cudaMalloc C-row (Partial)");
            
            if (cuda_alloc_ok) {
                matA->alloc_mode = PARTIAL_ALLOC;
                matB->alloc_mode = FULL_ALLOC;
                matC->alloc_mode = PARTIAL_ALLOC;
                printf("Alocação GPU: PARTIAL_ALLOC bem-sucedida.\n");
            } else {
                goto fail_alloc; // Falha na alocação parcial
            }
        } else {
            goto fail_alloc; // Memória insuficiente
        }
    }
 
    set_grid_size(threads_per_block, max_blocks);

    printf("\n--- Estado Inicial ---\n");
    print_first_256("Matriz A (Original)", matA); 
    print_first_256("Matriz B (Original)", matB);
    print_first_256("Matriz C (Original)", matC);

    // Copia dados de A para a GPU (somente se FULL_ALLOC)
    if (matA->alloc_mode == FULL_ALLOC) {
        check_cuda_error(cudaMemcpy(matA->d_rows, matA->h_rows, a_bytes, cudaMemcpyHostToDevice), "cudaMemcpy A H->D");
    }
    
    gettimeofday(&start_scalar, NULL);
    scalar_matrix_mult(scalar, matA);
    gettimeofday(&stop_scalar, NULL);

    // Copia dados de A da GPU para o Host (somente se FULL_ALLOC)
    if (matA->alloc_mode == FULL_ALLOC) {
        check_cuda_error(cudaMemcpy(matA->h_rows, matA->d_rows, a_bytes, cudaMemcpyDeviceToHost), "cudaMemcpy A D->H");
    }
    save_matrix_to_file(matA, results1);
 
    if (matA->alloc_mode == FULL_ALLOC) {
        check_cuda_error(cudaMemcpy(matA->d_rows, matA->h_rows, a_bytes, cudaMemcpyHostToDevice), "cudaMemcpy A (mod) H->D");
    }

    if (matB->alloc_mode == FULL_ALLOC) { // B sempre será FULL_ALLOC
        check_cuda_error(cudaMemcpy(matB->d_rows, matB->h_rows, b_bytes, cudaMemcpyHostToDevice), "cudaMemcpy B H->D");
    }

    if (matC->alloc_mode == FULL_ALLOC) {
        check_cuda_error(cudaMemcpy(matC->d_rows, matC->h_rows, c_bytes, cudaMemcpyHostToDevice), "cudaMemcpy C H->D");
    }

    gettimeofday(&start_mult, NULL);
    matrix_matrix_mult(matA, matB, matC); 
    gettimeofday(&stop_mult, NULL);

    if (matC->alloc_mode == FULL_ALLOC) {
        check_cuda_error(cudaMemcpy(matC->h_rows, matC->d_rows, c_bytes, cudaMemcpyDeviceToHost), "cudaMemcpy C D->H");
    }
    save_matrix_to_file(matC, results2);

    printf("\n--- Resultados Finais ---\n");
    print_first_256("Matriz A (Resultado Escalar)", matA);
    print_first_256("Matriz C (Resultado Multiplicação)", matC); 

    free_matrix(matA);
    free_matrix(matB);
    free_matrix(matC);

    printf("\n--- Tempos de Execução ---\n");
    printf("Tempo scalar_matrix_mult: %f ms\n", timedifference_msec(start_scalar, stop_scalar)); 
    printf("Tempo matrix_matrix_mult: %f ms\n", timedifference_msec(start_mult, stop_mult)); 
    gettimeofday(&overall_t2, NULL);
    printf("Tempo total de execução (Overall time): %f ms\n", timedifference_msec(overall_t1, overall_t2));

    return 0;

fail_alloc:
    fprintf(stderr, "Erro de alocação de GPGPU: Memória insuficiente para PARTIAL_ALLOC.\n");
    free_matrix(matA);
    free_matrix(matB);
    free_matrix(matC);
    return 1;
}