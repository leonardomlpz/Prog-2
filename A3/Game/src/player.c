#include "player.h"
#include "game.h"   // Para SCREEN_HEIGHT (para o "chão")
#include <stdio.h>
#include <stdlib.h>
#include <allegro5/allegro_primitives.h>

// Constantes da física do jogador (você pode ajustar)
#define PLAYER_SPEED 5.0
#define JUMP_POWER 12.0
#define GRAVITY 0.5

// --- Criar Jogador ---
Player* player_create(float start_x, float start_y) {
    Player* p = (Player*) malloc(sizeof(Player));
    if (!p) {
        printf("Erro ao alocar jogador\n");
        return NULL;
    }

    p->x = start_x;
    p->y = start_y;
    p->width = 32;  // Um quadrado de 32x32 pixels
    p->height = 32;
    p->vel_x = 0;
    p->vel_y = 0;
    p->hp = 5;      // [cite: 34] (requisito: sistema de pontos de vida)
    p->state = IDLE;
    p->on_ground = false;

    p->move_left = false;
    p->move_right = false;

    printf("Jogador (quadrado) criado!\n");
    return p;
}

// --- Destruir Jogador ---
void player_destroy(Player* p) {
    if (p) {
        free(p);
        printf("Jogador destruído.\n");
    }
}

// --- Tratar Eventos (Input) ---
void player_handle_event(Player* p, ALLEGRO_EVENT* event) {
    
    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        switch(event->keyboard.keycode) {
            case ALLEGRO_KEY_W: // Pular
                if (p->on_ground) {
                    p->vel_y = -JUMP_POWER;
                    p->state = JUMPING;
                    p->on_ground = false;
                }
                break;
            case ALLEGRO_KEY_A: // Esquerda
                p->move_left = true;
                break;
            case ALLEGRO_KEY_D: // Direita
                p->move_right = true;
                break;
        }
    } 
    else if (event->type == ALLEGRO_EVENT_KEY_UP) {
        switch(event->keyboard.keycode) {
            case ALLEGRO_KEY_A:
                p->move_left = false;
                break;
            case ALLEGRO_KEY_D:
                p->move_right = false;
                break;
        }
    }
}

// --- Atualizar Lógica (Física) ---
void player_update(Player* p) {
    
    // 1. Define a velocidade horizontal baseado no estado do input
    //    (Esta é a nova lógica que corrige o A+D)
    p->vel_x = 0;
    if (p->move_left)  p->vel_x -= PLAYER_SPEED;
    if (p->move_right) p->vel_x += PLAYER_SPEED;

    // 2. Define o estado (parado/andando)
    if (p->vel_x != 0 && p->state == IDLE && p->on_ground) {
        p->state = WALKING;
    } 
    else if (p->vel_x == 0 && p->state == WALKING) {
        p->state = IDLE;
    }
    
    // 3. Aplicar gravidade
    p->vel_y += GRAVITY;

    // 4. Atualizar posição
    p->x += p->vel_x;
    p->y += p->vel_y;

    // 5. Colisão com o "chão"
    float ground_level = 500;
    
    if (p->y + p->height > ground_level) {
        p->y = ground_level - p->height; 
        p->vel_y = 0;
        
        if (p->state == JUMPING) { 
            p->state = IDLE;     
        }
        p->on_ground = true;
    }

    // 6. (NOVO) Colisão com as bordas da tela
    if (p->x < 0) { // Borda esquerda
        p->x = 0;
    }
    else if (p->x + p->width > SCREEN_WIDTH) { // Borda direita
        p->x = SCREEN_WIDTH - p->width;
    }
}

// --- Desenhar Jogador ---
void player_draw(Player* p) {
    // Desenha um retângulo preenchido
    al_draw_filled_rectangle(
        p->x, 
        p->y, 
        p->x + p->width, 
        p->y + p->height, 
        al_map_rgb(255, 0, 0) // Cor vermelha
    );
}