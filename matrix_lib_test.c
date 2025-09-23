//INF-1029 - Trabalho 2
//Diego Miranda - 2210996
//Felipe Cancella - 2210487
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
    
    struct timeval start1, start2, stop1, stop2, overall_t1, overall_t2;
    gettimeofday(&overall_t1, NULL);

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
    
    load_matrix(floats1, &A);
    load_matrix(floats2, &B);

    //////////////////////////////////////////////////////////////////////////
    printf("Antes:");
    printf("----------Matrix A - Primeiros 256 termos----------\n");
    for(int i = 0; i < A.height * A.width; i++){
        printf("%f ", A.rows[i]);
        if (i%A.width==A.width-1)
            printf("\n");
        if (i==255)
            break;
    }
    printf("\n---------------------------------------------------\n");

    printf("----------Matrix B - Primeiros 256 termos----------\n");
    for(int i = 0; i < B.height * B.width; i++){
        printf("%f ", B.rows[i]);
        if (i%B.width==B.width-1)
            printf("\n");
        if (i==255)
            break;
    }
    printf("\n---------------------------------------------------\n");

    printf("----------Matrix C - Primeiros 256 termos----------\n");
    for(int i = 0; i < C.height * C.width; i++){
        printf("%f ", C.rows[i]);
        if (i%C.width==C.width-1)
            printf("\n");
        if (i==255)
            break;
    }
    printf("\n---------------------------------------------------\n\n\n\n");
    //////////////////////////////////////////////////////////////////////////

    gettimeofday(&start1, NULL);
    scalar_matrix_mult(scalar, &A);
    save_matrix(result1, &A);
    gettimeofday(&stop1, NULL);

    gettimeofday(&start2, NULL);
    matrix_matrix_mult(&A, &B, &C);
    save_matrix(result2, &C);  
    gettimeofday(&stop2, NULL);
    
    //////////////////////////////////////////////////////////////////////////
    printf("Depois:");
    printf("\n\n\n----------Matrix A - Primeiros 256 termos----------\n");
    for(int i = 0; i < A.height * A.width; i++){
        printf("%f ", A.rows[i]);
        if (i%A.width==A.width-1)
            printf("\n");
        if (i==255)
            break;
    }
    printf("\n---------------------------------------------------\n");
    
    printf("----------Matrix B - Primeiros 256 termos----------\n");
    for(int i = 0; i < B.height * B.width; i++){
        printf("%f ", B.rows[i]);
        if (i%B.width==B.width-1)
            printf("\n");
        if (i==255)
            break;
    }
    printf("\n---------------------------------------------------\n");
    
    printf("----------Matrix C - Primeiros 256 termos----------\n");
    for(int i = 0; i < C.height * C.width; i++){
        printf("%f ", C.rows[i]);
        if (i%C.width==C.width-1)
            printf("\n");
        if (i==255)
            break;
    }
    printf("\n---------------------------------------------------\n");
    //////////////////////////////////////////////////////////////////////////
    
    free(A.rows);
    free(B.rows);
    free(C.rows);
    
    printf("scalar_matrix_mult: %f ms\n", timedifference_msec(start1, stop1));
    printf("matrix_matrix_mult: %f ms\n", timedifference_msec(start2, stop2));
    gettimeofday(&overall_t2, NULL);
    printf("Overall time: %f ms\n", timedifference_msec(overall_t1, overall_t2));
    
}