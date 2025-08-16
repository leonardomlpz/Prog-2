#include <stdio.h>
#include <stdlib.h>
#include "jukefila.h"
#include "pedido.h"

jukefila* criar_jukefila(){
    jukefila *novo;
    if(! (novo=malloc(sizeof(jukefila))) )
        return NULL;
    
    novo->inicio = NULL;
    novo->final = NULL;

    return novo;
}

unsigned int contar_jukefila(jukefila* fila){
    if (!fila)
        return 0;

    pedido *pedido = fila->inicio;
    //fila vazia
    if (!pedido)
    return 0;
    
    unsigned int i = 0;
    while (pedido){
        pedido = pedido->proximo;
        i++;
    }
    return i;
}

void inserir_jukefila(pedido* elemento, jukefila* fila){
    if (!elemento || !fila)
        return;
    //fila vazia
    if (!contar_jukefila(fila)){
        fila->inicio = elemento;
        fila->final = elemento;
        return;
    }
    
    pedido *iterador;
    for (iterador = fila->inicio; ((iterador) && elemento->valor <=iterador->valor); iterador = iterador->proximo);

    //se for NULL o ultimo, chegou ao fim da fila
    if (!iterador){
        fila->final->proximo = elemento;
        elemento->anterior = fila->final;
        fila->final = elemento;
        return;
    }
    //insere no comeco
    if (iterador == fila->inicio){
        elemento->proximo = fila->inicio;
        elemento->anterior = NULL;
        fila->inicio->anterior = elemento;
        fila->inicio = elemento;
        return;
    }
    
    //adiciona no meio
    elemento->proximo = iterador;
    elemento->anterior = iterador->anterior;
    iterador->anterior->proximo = elemento;
    iterador->anterior = elemento;

    return;
}

pedido* consumir_jukefila(jukefila* fila){
    if (!fila || !fila->inicio)
        return NULL;

    pedido *removido = fila->inicio;
    //testa se tem mais de um item
    if (removido->proximo)
        removido->proximo->anterior = NULL;
    else fila->final = NULL;
    fila->inicio = fila->inicio->proximo;

    return removido;
}

void destruir_jukefila(jukefila *fila){
    if (!fila)
        return;

    pedido *i;
    while(i = consumir_jukefila(fila))
        destruir_pedido(i);
    free(fila);

    return;
}