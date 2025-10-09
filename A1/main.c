#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "gbv.h"

void gbv_close();

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

    if (opcao[0] == '-' && strlen(opcao) == 2) {
        switch (opcao[1]) {
            case 'a':
                for (int i = 3; i < argc; i++) {
                    gbv_add(&lib, biblioteca, argv[i]);
                }
                break;

            case 'r':
                for (int i = 3; i < argc; i++) {
                    gbv_remove(&lib, argv[i]);
                }
                break;

            case 'l':
                gbv_list(&lib);
                break;

            case 'v':
                if (argc < 4) {
                    fprintf(stderr, "Erro: A opção '-v' requer o nome de um documento.\n");
                } else {
                    gbv_view(&lib, argv[3]);
                }
                break;

            case 'o':
                if (argc < 4) {
                    fprintf(stderr, "Erro: A opção '-o' requer um critério.\n");
                } else {
                    gbv_order(&lib, biblioteca, argv[3]);
                }
                break;

            default:
                printf("Opção '-%c' é inválida.\n", opcao[1]);
                break;
        }
    } else {
        printf("Formato de opção inválido: '%s'. Use opções como '-a', '-l', etc.\n", opcao);
    }

    free(lib.docs);
    gbv_close();
    return 0;
}