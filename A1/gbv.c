#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gbv.h"
#include "util.h"

static FILE *open_gbv = NULL;

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
    open_gbv = fopen(filename, "rb+");
    if (!open_gbv){
        perror("Erro ao abrir o arquivo");
        return -1;
    }

    SuperBlock sb;
    if (fread(&sb, sizeof(SuperBlock), 1, open_gbv) != 1){
        perror("Erro ao ler superbloco");
        fclose(open_gbv);
        return -1;
    }

    lib->count = sb.count;
    lib->docs = malloc(sb.count * sizeof(Document));
    if (!lib->docs && sb.count > 0){
        perror("Erro ao alocar memoria");
        fclose(open_gbv);
        return -1;
    }

    fseek(open_gbv, sb.dir_offset, SEEK_SET);
    if (fread(lib->docs, sizeof(Document), sb.count, open_gbv) != (size_t)sb.count){
        perror("Erro ao ler diretorio");
        free(lib->docs);
        fclose(open_gbv);
        return -1;
    }

    return 0;
}

int gbv_add(Library *lib, const char *archive, const char *docname){
    FILE *src = fopen(docname,"rb");
    if (!src){
        perror("Erro ao abrir documento");
        return -1;
    }

    SuperBlock sb;
    fread(&sb, sizeof(SuperBlock), 1, open_gbv);

    // Descobre o tam do doc
    fseek(src, 0, SEEK_END);
    long size = ftell(src);
    // Volta para o inicio para copiar
    rewind(src);

    Document doc;
    strncpy(doc.name, docname, MAX_NAME);
    doc.size = size;
    doc.date = time(NULL);
    // Local onde termina o diretorio
    doc.offset = sb.dir_offset;


    static char buffer1[BUFFER_SIZE];
    size_t n;

    // Pula ate onde comeca o diretorio
    fseek(open_gbv, sb.dir_offset, SEEK_SET);
    while ((n = fread(buffer1, 1, BUFFER_SIZE, src)) > 0)
        fwrite(buffer1, 1, n, open_gbv);

    // Atualizar a biblioteca na memoria
    lib->docs = realloc(lib->docs, (lib->count + 1) * sizeof(Document));
    lib->docs[lib->count] = doc;
    lib->count++;

    // Atualizar o SuperBloco
    sb.count = lib->count;
    // Diretorio comeca apos os dados que acabou de escrever
    sb.dir_offset = ftell(open_gbv);

    // Regravar SuperBloco no inicio do arquivo
    fseek(open_gbv, 0, SEEK_SET);
    fwrite(&sb, sizeof(SuperBlock), 1, open_gbv);

    // Gravar o diretorio atualizado no final do arquivo
    fseek(open_gbv, sb.dir_offset, SEEK_SET);
    fwrite(lib->docs, sizeof(Document), lib->count, open_gbv);

    fclose(src);
    return 0;
}

int gbv_list(const Library *lib) {
    if (lib->count == 0) {
        printf("Biblioteca vazia.\n");
        return 0;
    }

    for (int i = 0; i < lib->count; i++) {
        Document d = lib->docs[i];
        char date[50];

        format_date(d.date, date, 50);

        printf("Nome: %s\n", d.name);
        printf("Tamanho: %ld bytes\n", d.size);
        printf("Data: %s\n", date);
        printf("Offset: %ld\n", d.offset);
        printf("---------------------------\n");
    }

    return 0;
}

int gbv_remove(Library *lib, const char *docname) {
    if (!lib || !docname)
        return -1;

    int remove = -1;
    for (int i = 0; i < lib->count; i++){
        if (strcmp(lib->docs[i].name, docname) == 0){
            remove = i;
            break;
        }
    }
    if (remove == -1){
        printf("Documento %s nao encontrado.\n", docname);
        return -1;
    }

    for (int i = remove; i < (lib->count - 1); i++){
        lib->docs[i] = lib->docs[i+1];
    }
    lib->count--;

    size_t new_size_docs = lib->count * sizeof(Document);
    Document *temp = realloc(lib->docs, new_size_docs);

    if (temp != NULL || lib->count == 0)
        lib->docs = temp;
    else
        perror("Erro ao tentar diminuir diretorio");

    // Reescreve o superbloco
    SuperBlock sb;
    fseek(open_gbv, 0, SEEK_SET);
    fread(&sb, sizeof(SuperBlock), 1, open_gbv);
    sb.count = lib->count;

    fseek(open_gbv, 0, SEEK_SET);
    fwrite(&sb, sizeof(SuperBlock), 1, open_gbv);

    fseek(open_gbv, sb.dir_offset, SEEK_SET);
    fwrite(lib->docs, sizeof(Document), lib->count, open_gbv);

    return 0;
}

int gbv_view(const Library *lib, const char *docname) {
    Document *doc = NULL;
    for (int i = 0; i < lib->count; i++){
        if (strcmp(lib->docs[i].name, docname) == 0){
            doc = &lib->docs[i];
            break;
        }
    }
    if (!doc){
        printf("Documento %s nao encontrado.\n",docname);
        return -1;
    }

    static char buffer[BUFFER_SIZE];
    long pos = 0; // posicao atual no doc

    char opcao;
    do{
        // calcular quanto ainda falta do documento
        long remaining = doc->size - pos;
        size_t to_read = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;

        // andar para o bloco correto dentro do container
        fseek(open_gbv, doc->offset + pos, SEEK_SET);
        size_t n = fread(buffer, 1, to_read, open_gbv);

        fwrite(buffer, 1, n, stdout);
        printf("\n");

        printf("Opcoes: n -> prox bloco;\n        p -> bloco anterior;\n        q -> sair\n");
        scanf("%c", &opcao);
        getchar();

        switch (opcao){
            case 'n':
            if (pos + BUFFER_SIZE < doc->size)
                pos += BUFFER_SIZE;
            else
                printf("Fim do documento.\n");
            break;

            case 'p':
                if (pos - BUFFER_SIZE >= 0)
                    pos -= BUFFER_SIZE;
                else
                    printf("Esta no inicio.\n");
            break;
    
            case 'q':
            break;

            default:
            break;
        }

    } while (opcao != 'q');

    return 0;
}

//  IGNORAR ESSA FUNCAO
int gbv_order(Library *lib, const char *archive, const char *criteria) {
    printf("Função gbv_order ainda não implementada.\n");
    return 0;
}

void gbv_close(){
    if (open_gbv){
        fclose(open_gbv);
    }
}