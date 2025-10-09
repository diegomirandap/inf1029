//INF-1029 - Trabalho 3
//Diego Miranda - 2210996
//Felipe Cancella - 2210487
typedef struct matrix {
    unsigned long int height;
    unsigned long int width;
    float *rows;
}matrix;

int scalar_matrix_mult(float scalar_value, struct matrix* matrix);
int matrix_matrix_mult(struct matrix* matrixA, struct matrix* matrixB, struct matrix* matrixC);
void set_number_threads(int num_threads);
