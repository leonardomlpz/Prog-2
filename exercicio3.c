#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
    char texto[1000];
    char *token;
    unsigned int num_colunas = 0;

    printf("Digite o texto:\n");
    fgets(texto, sizeof(texto), stdin);

    texto[strcspn(texto, "\n")] = '\0';

    token = strtok(texto, ",");
    num_colunas++;
    while (token != NULL){
        printf("%2u: Frase = %s\n", num_colunas, token);
        num_colunas++;
        token = strtok(NULL, ",");
    }

    return 0;
}