//INF-1029 - Trabalho 4
//Diego Miranda - 2210996
//Felipe Cancella - 2210487
#define PARTIAL_ALLOC 0
#define FULL_ALLOC 1

typedef struct matrix {
    unsigned long int height; 
    unsigned long int width;  
    float *h_rows;            // Ponteiro para dados no host (CPU) 
    float *d_rows;            // Ponteiro para dados no device (GPU)
    int alloc_mode;           // FULL_ALLOC ou PARTIAL_ALLOC
} matrix;

int set_grid_size(int threads_per_block, int max_blocks_per_grid);
int scalar_matrix_mult(float scalar_value, matrix *matrix);
int matrix_matrix_mult(matrix *matrixA, matrix *matrixB, matrix *matrixC);