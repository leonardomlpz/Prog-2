#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "gbv.h"

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <opção> <biblioteca> [documentos...]\n", argv[0]);
        return 1;
    }

    const char *opcao = argv[1];
    const char *biblioteca = argv[2];

    Library lib;

    if (access(biblioteca, F_OK) != 0) {
        if (gbv_create(biblioteca) != 0) {
            fprintf(stderr, "Erro ao criar biblioteca %s\n", biblioteca);
            return 1;
        }
    }

    if (gbv_open(&lib, biblioteca) != 0) {
        printf("Erro ao abrir biblioteca %s\n", biblioteca);
        return 1;
    }

    if (strcmp(opcao, "-a") == 0) {
        for (int i = 3; i < argc; i++) {
            gbv_add(&lib, biblioteca, argv[i]);
        }


    } else if (strcmp(opcao, "-r") == 0) {
        for (int i = 3; i < argc; i++) {
            gbv_remove(&lib, argv[i]);
        }


    } else if (strcmp(opcao, "-l") == 0) {
        gbv_list(&lib);










    } else if (strcmp(opcao, "-v") == 0 && argc >= 4) {
        //gbv_view(&lib, argv[3]);

    
        Document *doc = NULL;
        for (int i = 0; i < lib.count; i++){
            if (strcmp(lib.docs[i].name, argv[3]) == 0){
                doc = &lib.docs[i];
                break;
            }
        }
        if (!doc){
            printf("Documento %s nao encontrado.\n",argv[3]);
            return -1;
        }


        FILE *fp = fopen(biblioteca,"rb");
        if (!fp){
            perror("Erro ao abirir container");
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
            fseek(fp, doc->offset + pos, SEEK_SET);
            size_t n = fread(buffer, 1, to_read, fp);

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

        fclose(fp);
        return 0;
    







    } else if (strcmp(opcao, "-o") == 0 && argc >= 4) {
        gbv_order(&lib, biblioteca, argv[3]);
    } else {
        printf("Opção inválida.\n");
    }

    return 0;
}

