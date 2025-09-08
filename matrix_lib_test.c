//matrix_lib_test 5.0 8 16 16 8 floats_256_2.0f.dat floats_256_5.0f.dat result1.dat result2.dat
#include "matrix_lib.h"
#include "timer.h"
#include <stdio.h>
#include <stdlib.h>

void load_matrix(char* filename, struct matrix* m){
    FILE* f = fopen(filename, "rb");
    if (!f)
        exit(0);
    fread(m->rows, sizeof(float), m->height * m->width, f);
    fclose(f);
}

void save_matrix(char* filename, struct matrix* m){
    FILE* f = fopen(filename, "wb");
    if (!f)
        exit(0);
    fwrite(m->rows, sizeof(float), m->height * m->width, f);
    fclose(f);
}

int main (int argc, char* argv[]){
    if (argc != 10){
        printf("Erro na execução do arquivo: matrix_lib_test <scalar> <heightA> <widthB> <heightB> <widthB> <floatsFile1> <floatsFile2> <resultFile1> <resultFile2>");
    }

    float scalar = atof(argv[1]);
    int heightA = atoi(argv[2]);
    int widthA = atoi(argv[3]);
    int heightB = atoi(argv[4]);
    int widthB = atoi(argv[5]);
    char* floats1 = argv[6]; 
    char* floats2 = argv[7];
    char* result1 = argv[8];
    char* result2 = argv[9];

    if (widthA != heightB)
        return 1;
    
    struct matrix A = {heightA, widthA, malloc(heightA * widthA * sizeof(float))};
    struct matrix B = {heightB, widthB, malloc(heightB * widthB * sizeof(float))};
    struct matrix C = {heightA, widthB, malloc(heightA * widthB * sizeof(float))};
    struct timeval start, stop, overall_t1, overall_t2;
    
    load_matrix(floats1, &A);
    load_matrix(floats2, &B);

    /*//////////////////////////////////////////////////////////////////////////
    printf("----------Matrix A - Primeiros 256 termos----------\n");
    for(int i = 0; i < 256; i++){
        printf("%f ", A.rows[i]);
        if (i%A.width==A.width-1)
            printf("\n");
    }
    printf("---------------------------------------------------\n");

    printf("----------Matrix B - Primeiros 256 termos----------\n");
    for(int i = 0; i < B.height * B.width; i++){
        printf("%f ", B.rows[i]);
        if (i%B.width==B.width-1)
            printf("\n");
    }
    printf("---------------------------------------------------\n");

    printf("----------Matrix C - Primeiros 256 termos----------\n");
    for(int i = 0; i < C.height * C.width; i++){
        printf("%f ", C.rows[i]);
        if (i%C.width==C.width-1)
            printf("\n");
    }
    printf("---------------------------------------------------\n\n\n\n");
    //////////////////////////////////////////////////////////////////////////*/

    gettimeofday(&overall_t1, NULL);

    gettimeofday(&start, NULL);
    scalar_matrix_mult(scalar, &A);
    gettimeofday(&stop, NULL);
    printf("scalar_matrix_mult: %f ms\n", timedifference_msec(start, stop));
    save_matrix(result1, &A);

    gettimeofday(&start, NULL);
    matrix_matrix_mult(&A, &B, &C);
    gettimeofday(&stop, NULL);
    printf("matrix_matrix_mult: %f ms\n", timedifference_msec(start, stop));
    save_matrix(result2, &C);

    gettimeofday(&overall_t2, NULL);
    printf("Overall time: %f ms\n", timedifference_msec(overall_t1, overall_t2));
    
    
    /*//////////////////////////////////////////////////////////////////////////
    printf("\n\n\n----------Matrix A - Primeiros 256 termos----------\n");
    for(int i = 0; i < A.height * A.width; i++){
        printf("%f ", A.rows[i]);
        if (i%A.width==A.width-1)
        printf("\n");
    }
    printf("---------------------------------------------------\n");
    
    printf("----------Matrix B - Primeiros 256 termos----------\n");
    for(int i = 0; i < B.height * B.width; i++){
        printf("%f ", B.rows[i]);
        if (i%B.width==B.width-1)
        printf("\n");
    }
    printf("---------------------------------------------------\n");
    
    printf("----------Matrix C - Primeiros 256 termos----------\n");
    for(int i = 0; i < C.height * C.width; i++){
        printf("%f ", C.rows[i]);
        if (i%C.width==C.width-1)
        printf("\n");
    }
    printf("---------------------------------------------------\n");
    //////////////////////////////////////////////////////////////////////////*/
    
    free(A.rows);
    free(B.rows);
    free(C.rows);
    
}
