#include <stdio.h>
#include <stdlib.h>

int main(){
    int lin = 3;
    int col = lin;
    int *matriz1, *matriz2;

    matriz1 = malloc(lin*col*sizeof(int));
    matriz2 = malloc(lin*col*sizeof(int));

    printf("Digite a primeira matriz:\n");
    for (int i = 0; i < lin; i++)
        for (int j = 0; j < col; j++)
            scanf("%d", matriz1[(i*col) + j]);

    printf("Digite a segunda matriz:\n");
    for (int i = 0; i < lin; i++)
        for (int j = 0; j < col; j++)
            scanf("%d", matriz2[(i*col) + j]);

    //calcula a multiplicacao das duas matrizes
    for (int i = 0; i < lin; i++)
        for (int j = 0; j < col; j++){
            
        }
}