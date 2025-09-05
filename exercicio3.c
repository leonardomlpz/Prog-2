#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
    typedef struct {
        int codigo;
        char nome[50];
        float preco;
    } Produto;

    int opcao;
    do{
        printf("Digite 1 para cadastrar novos produtos, 2 para listar os produtos e 3 para sair\n");
        scanf("%d", &opcao);

        switch (opcao){
        case 1: {
            Produto prod;
            printf("Digite o codigo:\n");
            scanf("%d", &prod.codigo);

            printf("Digite o nome:\n");
            fgetc(stdin); // remove o '\n' sobrando no buffer
            fgets(prod.nome, sizeof(prod.nome), stdin);
            prod.nome[strcspn(prod.nome, "\n")] = 0; // remove o '\n' do final

            printf("Digite o preco:\n");
            scanf("%f", &prod.preco);
            
            FILE *arquivo = fopen("produtos.bin", "ab");
            if (!arquivo) {
                perror("Erro ao abrir arquivo");
                break;
            }

            fwrite(&prod, sizeof(Produto), 1, arquivo);
            fclose(arquivo);

            break;
        }
        case 2: {
            Produto prod;
            FILE *arq = fopen("produtos.bin", "rb");
            if (!arq) {
                perror("Erro ao abrir arquivo");
                break;
            }

            while (fread(&prod, sizeof(Produto), 1, arq) == 1){
                printf("Codigo:%6d, Nome: %s, Preço: %.2f\n", prod.codigo, prod.nome, prod.preco);
            }
            fclose(arq);

            break;
        }
        case 3:
            printf("Saindo do programa...\n");
            break;
        default:
            printf("Opcao invalida!\n");
            break;
        }
    } while(opcao != 3);

    return 0;
}
