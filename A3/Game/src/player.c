#include "player.h"
#include "game.h"   // Para SCREEN_HEIGHT (para o "chão")
#include "world.h"
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
    p->hp = 5;
    p->state = IDLE;
    p->on_ground = false;

    p->move_left = false;
    p->move_right = false;
    p->jump_pressed = false;

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
                p->jump_pressed = true;
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
            case ALLEGRO_KEY_W:
                p->jump_pressed = false;
                break;
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
void player_update(Player *p, World *world) {

    // 1. Define a velocidade horizontal baseado no estado do input
    p->vel_x = 0;
    if (p->move_left)  p->vel_x -= PLAYER_SPEED;
    if (p->move_right) p->vel_x += PLAYER_SPEED;

    // 2. Lógica de Pulo
    if (p->jump_pressed && p->on_ground) {
        p->vel_y = -JUMP_POWER;
        p->state = JUMPING;
        p->on_ground = false;
    }

    // 3. Aplicar gravidade (sempre)
    p->vel_y += GRAVITY;
    // Limita a velocidade máxima de queda
    if (p->vel_y > 15) p->vel_y = 15;


    // --- NOVA LÓGICA DE COLISÃO ---

    // Assume que não estamos no chão até que a colisão prove o contrário
    p->on_ground = false;

    // --- Colisão Eixo X (Horizontal) ---
    // Primeiro, move no eixo X
    p->x += p->vel_x;

    // Verifica colisão horizontal
    if (p->vel_x > 0) { // Movendo para a direita
        // Verifica o canto superior direito e inferior direito
        if (world_is_solid(world, p->x + p->width, p->y) ||
            world_is_solid(world, p->x + p->width, p->y + p->height - 1)) {
            // Bateu! Coloca o jogador "grudado" no bloco
            p->x = (int)(p->x + p->width) / TILE_SIZE * TILE_SIZE - p->width;
            p->vel_x = 0;
        }
    }
    else if (p->vel_x < 0) { // Movendo para a esquerda
        // Verifica o canto superior esquerdo e inferior esquerdo
        if (world_is_solid(world, p->x, p->y) ||
            world_is_solid(world, p->x, p->y + p->height - 1)) {
            // Bateu!
            p->x = ((int)p->x / TILE_SIZE + 1) * TILE_SIZE;
            p->vel_x = 0;
        }
    }

    // --- Colisão Eixo Y (Vertical) ---
    // Agora, move no eixo Y
    p->y += p->vel_y;

    if (p->vel_y > 0) { // Caindo
        // Verifica o canto inferior esquerdo e inferior direito
        if (world_is_solid(world, p->x + 1, p->y + p->height) ||
            world_is_solid(world, p->x + p->width - 1, p->y + p->height)) {
            // Bateu no chão!
            p->y = (int)(p->y + p->height) / TILE_SIZE * TILE_SIZE - p->height;
            p->vel_y = 0;
            p->on_ground = true;
            if (p->state == JUMPING) p->state = IDLE;
        }
    }
    else if (p->vel_y < 0) { // Subindo (pulando)
        // Verifica o canto superior esquerdo e superior direito
        if (world_is_solid(world, p->x + 1, p->y) ||
            world_is_solid(world, p->x + p->width - 1, p->y)) {
            // Bateu a cabeça!
            p->y = ((int)p->y / TILE_SIZE + 1) * TILE_SIZE;
            p->vel_y = 0; // Para de subir
        }
    }

    // 4. (REMOVIDO) Colisão com as bordas da tela
    // A colisão com os limites do mundo agora é tratada por world_is_solid()
    if (p->x < 0) p->x = 0;
    if (p->x + p->width > world->width * world->tile_size) {
        p->x = world->width * world->tile_size - p->width;
    }

    // 5. Define o estado (parado/andando)
    if (p->vel_x != 0 && p->state == IDLE && p->on_ground) {
        p->state = WALKING;
    } 
    else if (p->vel_x == 0 && p->state == WALKING) {
        p->state = IDLE;
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