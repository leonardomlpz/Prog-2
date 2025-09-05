#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gbv.h"

typedef struct {
    int count;
    long dir_offset;
} SuperBlock;

int gbv_create(const char *filename){
    FILE *fp = fopen(filename,"wb");
    if (!fp){
        perror("Erro ao criar arquivo");
        return -1;
    }

    SuperBlock sb;
    sb.count = 0;
    sb.dir_offset = sizeof(SuperBlock);

    if (fwrite(&sb, sizeof(SuperBlock), 1, fp) != 1){
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

int gbv_open(Library *lib, const char *filename){
    FILE *fp = fopen(filename, "rb");
    if (!fp){
        perror("Erro ao abrir o arquivo");
        return -1;
    }

    SuperBlock sb;
    if (fread(&sb, sizeof(SuperBlock), 1, fp) != 1){
        perror("Erro ao ler superbloco");
        fclose(fp);
        return -1;
    }

    lib->count = sb.count;
    lib->docs = malloc(sb.count * sizeof(Document));
    if (!lib->docs && sb.count > 0){
        perror("Erro ao alocar memoria");
        fclose(fp);
        return -1;
    }

    fseek(fp, sb.dir_offset, SEEK_SET);
    if (fread(lib->docs, sizeof(Document), sb.count, fp) != (size_t)sb.count){
        perror("Erro ao ler diretorio");
        free(lib->docs);
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}