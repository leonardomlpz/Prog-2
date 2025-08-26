#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pelucia.h"

int aleat (int min, int max)
{
    return rand() % (max - min + 1) + min;
}

void insere_maquina(struct loja *loja, struct maquina_pelucia *maquina){
    if (!loja || !maquina)
        return;
    //primeira máquina
    if (!loja->inicio){
        loja->inicio = maquina;
        maquina->proximo = maquina;
        maquina->anterior = maquina;
        loja->numero_maquinas++;
        return;
    }

    struct maquina_pelucia *i = loja->inicio;

    if (maquina->probabilidade > loja->inicio->probabilidade){
        maquina->proximo = loja->inicio;
        maquina->anterior = loja->inicio->anterior;
        loja->inicio->anterior->proximo = maquina;
        loja->inicio->anterior = maquina;
        //atualiza inicio
        loja->inicio = maquina;
        loja->numero_maquinas++;
        return;
    }

    //procura posicao para inserir
    while (i->proximo != loja->inicio && maquina->probabilidade <= i->proximo->probabilidade)
        i = i->proximo;

    maquina->proximo = i->proximo;
    maquina->anterior = i;
    i->proximo->anterior = maquina;
    i->proximo = maquina;
    loja->numero_maquinas++;
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

    srand(time(NULL));
    
    loja->inicio = NULL;
    loja->numero_maquinas = 0;
    criar_maquinas(numero_maquinas,loja);

    return loja;
}

void remove_maquina(struct maquina_pelucia *maquina, struct loja *loja){
    if (!maquina || !loja || !loja->inicio) return;

    if (loja->numero_maquinas == 1){
        loja->inicio = NULL;
        loja->numero_maquinas = 0;
        free(maquina);
        return;
    }

    //se remover a primeira maquina
    if (loja->inicio == maquina){
        loja->inicio = maquina->proximo;
    }

    maquina->anterior->proximo = maquina->proximo;
    maquina->proximo->anterior = maquina->anterior;

    loja->numero_maquinas--;
    free(maquina);
}


int jogar (struct loja *loja){
    if (!loja || !loja->inicio || !loja->numero_maquinas)
        return -1;

    int jogada = aleat(0,100);
    int maquina = aleat(1,loja->numero_maquinas);
    
    int iterador = 1;
    struct maquina_pelucia *it;
    it = loja->inicio;
    while (iterador < maquina){
        iterador++;
        it = it->proximo;
    }

    if (jogada < it->probabilidade){
        remove_maquina(it,loja);
        return 1;
    } else return 0;
}

void encerrar_dia (struct loja *loja){
    if (!loja || !loja->numero_maquinas)
        return;
    
    struct maquina_pelucia *atual;
    atual = loja->inicio;
    printf("maquinas|prob: (%u|%3u)", atual->id, atual->probabilidade);

    if (loja->numero_maquinas > 1){
        atual = atual->proximo;
        while(atual != loja->inicio){
            printf(" (%u|%3u)", atual->id, atual->probabilidade);
            atual = atual->proximo;
        }
    }
    printf("\n");
    return;
}

void destruir_loja (struct loja *loja){
    if (!loja)
        return;

    while (loja->numero_maquinas > 0)
        remove_maquina(loja->inicio, loja);

    free(loja);
    return;
}
