#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "gbv.h"
#include "util.h"

static FILE *open_gbv = NULL;
static char *nome_orignial;

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
    nome_orignial = strdup(filename);
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

void gbv_close(){
    if (open_gbv)
        fclose(open_gbv);

    if (nome_orignial)
        free(nome_orignial);
}

int gbv_add(Library *lib, const char *archive, const char *docname){
    FILE *src = fopen(docname,"rb");
    if (!src){
        perror("Erro ao abrir documento");
        return -1;
    }

    // Se existir um doc de mesmo nome remove para adicionar o novo
    for (int i = 0; i < lib->count; i++){
        if (strcmp(docname, lib->docs[i].name) == 0){
            gbv_remove(lib, docname);
            break;
        }
    }

    SuperBlock sb;
    fseek(open_gbv, 0, SEEK_SET);
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
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    }

    return 0;
}

int gbv_remove(Library *lib, const char *docname) {
    if (!lib || !docname)
        return -1;

    int remover = -1;
    // Procura se existe o doc
    for (int i = 0; i < lib->count; i++){
        if (strcmp(lib->docs[i].name, docname) == 0){
            remover = i;
            break;
        }
    }
    if (remover == -1){
        printf("Documento %s nao encontrado.\n", docname);
        return -1;
    }
    
    // Remove do vetor de doc
    for (int i = remover; i < lib->count - 1; i++)
        lib->docs[i] = lib->docs[i+1];

    lib->count--;
    // Realoca o espaco do vetor
    size_t new_size_docs = lib->count * sizeof(Document);
    Document *temp = realloc(lib->docs, new_size_docs);
    if (temp != NULL || lib->count == 0)
        lib->docs = temp;
    else
        perror("Erro ao diminuir diretorio");


    // Cria uma biblioteca temporaria 
    FILE *temp_gbv = fopen("biblioteca.tmp", "wb");
    if (!temp_gbv){
        perror("Erro ao criar arquivo temporario");
        return -1;
    }

    SuperBlock sb_novo;
    sb_novo.count = lib->count;
    long pos_escrita_atual = sizeof(SuperBlock);
    static char buffer[BUFFER_SIZE];

    fseek(temp_gbv, pos_escrita_atual, SEEK_SET);

    // Loop para copiar os dados validos
    for (int i = 0; i < lib->count; i++) {
        long offset_antigo = lib->docs[i].offset;
        long bytes_para_copiar = lib->docs[i].size;
        
        // Atualiza o offset na memoria com a posição correta no novo arquivo
        lib->docs[i].offset = pos_escrita_atual;

        // Posiciona a leitura no arquivo antigo (global)
        fseek(open_gbv, offset_antigo, SEEK_SET);

        // Copia os dados do arquivo antigo para o novo
        while (bytes_para_copiar > 0) {
            size_t to_read = (bytes_para_copiar < BUFFER_SIZE) ? bytes_para_copiar : BUFFER_SIZE;
            size_t n = fread(buffer, 1, to_read, open_gbv);
            fwrite(buffer, 1, n, temp_gbv);
            pos_escrita_atual += n;
            bytes_para_copiar -= n;
        }
    }

    // Agora que sabemos o tamanho final, atualizamos e escrevemos o superbloco
    sb_novo.dir_offset = pos_escrita_atual;
    fseek(temp_gbv, 0, SEEK_SET);
    fwrite(&sb_novo, sizeof(SuperBlock), 1, temp_gbv);

    // Escrevendo o diretorio atualizado no final
    fseek(temp_gbv, sb_novo.dir_offset, SEEK_SET);
    fwrite(lib->docs, sizeof(Document), lib->count, temp_gbv);

    fclose(temp_gbv);
    fclose(open_gbv);
    // Apaga o gbv original
    remove(nome_orignial);
    // Renomeia o novo
    rename("biblioteca.tmp", nome_orignial);

    open_gbv = fopen(nome_orignial, "rb+");

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
    // Posicao atual no doc
    long pos = 0; 

    char opcao;
    do{
        // Calcular quanto ainda falta do documento
        long remaining = doc->size - pos;
        size_t to_read = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;

        // Andar para o bloco correto dentro do container
        fseek(open_gbv, doc->offset + pos, SEEK_SET);
        size_t n = fread(buffer, 1, to_read, open_gbv);

        fwrite(buffer, 1, n, stdout);
        printf("\n");

        fflush(stdout);

        printf("\n\nOpcoes:\n n -> prox bloco;\n p -> bloco anterior;\n q -> sair\n");
        scanf(" %c", &opcao);

        switch (opcao){
            case 'n':
            if (pos + BUFFER_SIZE < doc->size)
                pos += BUFFER_SIZE;
            else
                printf("\n~~~~Fim do documento.~~~~\n\n");
            break;

            case 'p':
                if (pos - BUFFER_SIZE >= 0)
                    pos -= BUFFER_SIZE;
                else
                    printf("\n~~~~Esta no inicio.~~~~\n\n");
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
