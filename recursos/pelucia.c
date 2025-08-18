#include <stdio.h>
#include <stdlib.h>
#include "pelucia.h"

//
//struct maquina_pelucia {
//    unsigned int id;
//    unsigned int probabilidade;
//
//    struct maquina_pelucia *proximo;
//    struct maquina_pelucia *anterior;
//};
//
//struct loja {
//	struct maquina_pelucia *inicio;
//	unsigned int numero_maquinas;
//};
//

int aleat (int min, int max)
{
    return rand() % (max - min + 1) + min;
}

void insere_maquina(struct loja *loja, struct maquina_pelucia *maquina){
    if (!loja->inicio){
        loja->inicio = maquina;
        return;
    }
    
    //somente uma maquina adicionada
    if (!loja->inicio->proximo){
        loja->inicio->proximo = maquina;
        loja->inicio->anterior = maquina;
        maquina->proximo = loja->inicio;
        maquina->anterior = loja->inicio;
        //corrige se o inserido foi posto fora de ordem
        if (loja->inicio->probabilidade < maquina->probabilidade)
            loja->inicio = loja->inicio->proximo;

        return;
    }

    struct maquina_pelucia *i;

    //conferir se nao entra em laco com duas maquinas na mesma probabilidade

    //acha a posicao correta para por a maquina
    for (i = loja->inicio; ((i->proximo != loja->inicio) && (maquina->probabilidade <= i->probabilidade)); i = i->proximo);

    maquina->proximo = i;
    maquina->anterior = i->anterior;
    i->anterior->proximo = maquina;
    i->anterior = maquina;

    return;
}

void criar_maquinas(unsigned int numero_maquinas, struct loja *loja){
    struct maquina_pelucia *maquina;
    
    
    for (int i = 1; i <= numero_maquinas; i++){
        if (! (maquina = malloc(sizeof(struct maquina_pelucia))) )
            return;
        
        maquina->id = i;
        maquina->probabilidade = aleat(0,100);
        maquina->proximo = NULL;
        maquina->anterior = NULL;

        insere_maquina(loja, maquina);
        maquina = NULL;
    }

    return;
}

struct loja* criar_loja (unsigned int numero_maquinas){
    struct loja* loja;
    if (! (loja = malloc(sizeof(struct loja))) )
        return NULL;
    
    loja->numero_maquinas = numero_maquinas;
    criar_maquinas(numero_maquinas,loja);

    
    struct maquina_pelucia *atual;
    atual = loja->inicio;
    printf("maquinas: %u", atual->id);

    if (loja->numero_maquinas > 1){
        atual = atual->proximo;
        while(atual->id != loja->inicio->id){
            printf(" %d", atual->id);
            atual = atual->proximo;
        }
    }

    return loja;
}

void remove_maquina(struct maquina_pelucia *maquina, struct loja *loja){
    struct maquina_pelucia *iterador;
    iterador = loja->inicio;
    //somente uma maquina
    if (loja->numero_maquinas == 1){
        loja->inicio = NULL;
        loja->numero_maquinas--;
        free(maquina);
        return;
    }

    //ajusta o ponteiro auxiliar para a remocao
    while (iterador->id != maquina->id)
        iterador = iterador->proximo;

    iterador->anterior->proximo = iterador->proximo;
    iterador->proximo->anterior = iterador->anterior;
    free(iterador);

    return;
}

int jogar (struct loja *loja){
    if (!loja || !loja->inicio)
        return -1;

    int jogada = aleat(0,100);
    int maquina = aleat(1,loja->numero_maquinas);
    
    int iterador = 1;
    struct maquina_pelucia *it;
    it = loja->inicio;
    while (iterador < maquina)
        it = it->proximo;

    if (jogada < it->probabilidade){
        remove_maquina(it,loja);
        return 1;
    } else return 0;
}
//imprimir maquinas que restam na lista
void encerrar_dia (struct loja *loja){
    if (!loja || !loja->numero_maquinas)
        return;
    
    struct maquina_pelucia *atual;
    atual = loja->inicio;
    printf("maquinas: %d", atual->id);

    if (loja->numero_maquinas > 1){
        atual = atual->proximo;
        while(atual->id != loja->inicio->id){
            printf(" %d", atual->id);
            atual = atual->proximo;
        }
    }
    return;
}

void destruir_loja (struct loja *loja){

}