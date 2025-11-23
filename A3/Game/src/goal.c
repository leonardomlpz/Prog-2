#include "goal.h"
#include <stdio.h>
#include <stdlib.h>
#include "game.h" // Para acessar o estado do jogador

// --- Criar Goal ---
Goal* goal_create(float x, float y) {
    Goal *g = (Goal*) malloc(sizeof(Goal));
    if (!g) return NULL;

    g->x = x;
    g->y = y;

    g->sprite = al_load_bitmap("assets/End.png");
    
    if (g->sprite) {
        // Assume que o sprite é 64x64
        g->sprite_width = 64;
        g->sprite_height = 64;
        
        // Define uma hitbox menor (ex: 40x60)
        g->width = 40;
        g->height = 60;
    } else {
        printf("ERRO: Nao foi possivel carregar assets/End.png\n");
        g->width = 32; g->height = 32;
    }

    return g;
}

// --- Destruir Goal ---
void goal_destroy(Goal *g) {
    if (g) {
        if (g->sprite) al_destroy_bitmap(g->sprite);
        free(g);
    }
}

// --- Atualizar Goal (Checagem de Colisão) ---
void goal_update(Goal *g, Player *p) {
    // Se o jogador já estiver no estado WINNING, não checa mais
    if (p->state == WIN) return; 

    // Hitbox da taça (Goal)
    float gx1 = g->x; float gy1 = g->y;
    float gx2 = g->x + g->width; float gy2 = g->y + g->height;
    
    // Hitbox do jogador (Player)
    float px1 = p->x; float py1 = p->y;
    float px2 = p->x + p->width; float py2 = p->y + p->height;

    // Checa intersecção (Algoritmo AABB)
    if (!(gx1 > px2 || gx2 < px1 || gy1 > py2 || gy2 < py1)) {
        // COLISÃO! O jogador ganhou.
        p->state = WIN; 
    }
}

// --- Desenhar Goal ---
void goal_draw(Goal *g, World *w) {
    if (!g->sprite) return;

    float offset_x = (g->sprite_width - g->width) / 2.0;
    float offset_y = (g->sprite_height - g->height);

    // Posição de desenho na tela, ajustada pela câmera e offset
    float dx = (g->x - w->camera_x) - offset_x;
    float dy = (g->y - w->camera_y) - offset_y;

    al_draw_bitmap(g->sprite, dx, dy, 0);

    // Debug Hitbox
    al_draw_rectangle(g->x - w->camera_x, g->y - w->camera_y, 
                      g->x + g->width - w->camera_x, g->y + g->height - w->camera_y, 
                      al_map_rgb(0, 0, 255), 1);
}