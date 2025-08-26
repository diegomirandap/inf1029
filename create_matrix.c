#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Uso: %s <linhas> <colunas> <arquivo_saida>\n", argv[0]);
        return 1;
    }

    int h = atoi(argv[1]);
    int w = atoi(argv[2]);
    const char *filename = argv[3];

    FILE *f = fopen(filename, "wb");
    if (!f) { perror("Erro"); return 1; }

    for (int i = 0; i < h*w; i++) {
        float val = (float)(i+1); // números 1,2,3,...
        fwrite(&val, sizeof(float), 1, f);
    }

    fclose(f);
    return 0;
}