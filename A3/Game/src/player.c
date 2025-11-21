#include "player.h"
#include "game.h"   // Para SCREEN_HEIGHT (para o "chão")
#include "world.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <allegro5/allegro_primitives.h>

// Constantes da física do jogador (você pode ajustar)
#define PLAYER_SPEED 3.75
#define JUMP_POWER 10.0
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
    p->width = 18;
    p->height = 24;
    p->vel_x = 0;
    p->vel_y = 0;
    p->hp = 3;
    p->state = IDLE;
    p->on_ground = false;

    p->move_left = false;
    p->move_right = false;
    p->jump_pressed = false;

    p->spritesheet = al_load_bitmap("assets/ninja_sheet.png");
    if (!p->spritesheet) {
        printf("Erro ao carregar spritesheet 'assets/ninja_sheet.png'\n");
        free(p);
        return NULL;
    }

    p->heart_icon = al_load_bitmap("assets/heart.png");
    if (!p->heart_icon) {
        printf("Aviso: Nao foi possivel carregar assets/heart.png\n");
    }

    p->invulnerable_timer = 0.0;
    p->anim_row = 0;        // Começa na linha 0 (parado)
    p->current_frame = 0;
    p->frame_timer = 0;
    p->frame_delay = 0.05;
    p->num_frames = 11;
    p->facing_direction = 1;

    printf("Jogador (quadrado) criado!\n");
    return p;
}

// --- Destruir Jogador ---
void player_destroy(Player* p) {
    if (p) {
        if (p->spritesheet)
            al_destroy_bitmap(p->spritesheet);
            
        if (p->heart_icon)
            al_destroy_bitmap(p->heart_icon);
            
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
    if (p->invulnerable_timer > 0)
        p->invulnerable_timer -= 1.0 / FPS;

    // 1. Define a velocidade horizontal baseado no estado do input
    p->vel_x = 0;
    if (p->move_left)  p->vel_x -= PLAYER_SPEED;
    if (p->move_right) p->vel_x += PLAYER_SPEED;

    if (p->vel_x > 0)
        p->facing_direction = 1; // Direita
    else if (p->vel_x < 0)
        p->facing_direction = -1; // Esquerda

    // 2. Lógica de Pulo
    if (p->jump_pressed && p->on_ground) {
        p->vel_y = -JUMP_POWER;
        p->state = JUMPING;
        p->on_ground = false;

        p->anim_row = 2;
        p->num_frames = 1;
        p->current_frame = 0;
    }

    // 3. Aplicar gravidade (sempre)
    p->vel_y += GRAVITY;
    // Limita a velocidade máxima de queda
    if (p->vel_y > 15) p->vel_y = 15;

    // Se o jogador estiver no ar E caindo (vel_y > 0)
    if (!p->on_ground && p->vel_y > 0.1) {
        // (Assumindo que sua animação de "Fall" está na Linha 3)
        if (p->anim_row != 3) {
            p->anim_row = 3;
            p->num_frames = 1; // (A animação de queda do Ninja tem 1 frame)
            p->current_frame = 0;
        }
    }

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
            if (p->state == JUMPING || p->anim_row == 3){
                p->state = IDLE;
                p->anim_row = 0;
                p->num_frames = 11;
                p->current_frame = 0;
                p->frame_delay = 0.1;
            }
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
        p->anim_row = 1;
        p->num_frames = 12;
        p->frame_timer = 0; 
        p->frame_delay = 0.05;
    } 
    else if (p->vel_x == 0 && p->state == WALKING) {
        p->state = IDLE;
        p->anim_row = 0;
        p->num_frames = 11;
        p->current_frame = 0;
        p->frame_delay = 0.1;
    }

    // 6. Lógica de Animação
    if (p->state == WALKING || p->state == IDLE || p->state == JUMPING) {
        // Adiciona o tempo do último frame (1.0 / FPS) ao nosso timer
        p->frame_timer += 1.0 / FPS; 

        // Se o timer passou do nosso "delay"
        if (p->frame_timer >= p->frame_delay) {
            p->current_frame++; // Avança para o próximo frame
            if (p->current_frame >= p->num_frames) {
                p->current_frame = 0; // Volta ao início
            }
            p->frame_timer = 0; // Reinicia o timer
        }
    }

    int tile_at_player = world_get_tile(world, p->x + p->width/2, p->y + p->height/2);
    
    if (tile_at_player == 243) { 
        printf("DANO! Pisou no espinho.\n");
        if (player_take_damage(p))
            player_respawn(p);
    }

    int map_bottom_y = world->height * world->tile_size;

    // 2. Checa se o jogador (pela cabeça, p->y) passou desse limite
    if (p->y > map_bottom_y) {
        printf("DANO! Caiu do mapa.\n");
        if (player_take_damage(p)) // Se o dano foi aplicado...
            player_respawn(p); // ...faz o respawn.
    }
}

// (Implementação da função de dano)
bool player_take_damage(Player *p) {
    // Se já estiver invulnerável, não faz nada e retorna 'false'
    if (p->invulnerable_timer > 0) {
        return false;
    }

    // Se não estava invulnerável:
    printf("DANO! HP: %d\n", p->hp - 1);
    p->hp--; 
    
    // Ativa a invulnerabilidade por 2 segundos
    p->invulnerable_timer = 2.0; 
    
    // Aplica um pequeno "knockback" (pulo/empurrão)
    p->vel_y = -5; // Pulo para cima
    p->vel_x = -5 * p->facing_direction; // Empurrão para trás
    
    return true; // Dano foi aplicado
}

// (Implementação da função de respawn)
void player_respawn(Player *p) {
    printf("Respawn!\n");
    p->x = 32;
    p->y = 400;
    p->vel_x = 0;
    p->vel_y = 0;
}

// --- Desenhar Jogador ---
void player_draw(Player *p, World *world) {
    float sw = 32; 
    float sh = 32; 

    // --- CÁLCULO DE OFFSET (Centralizar o desenho na hitbox) ---
    // Diferença X: (64 - 28) / 2 = 18 pixels para a esquerda
    // Diferença Y: (64 - 50) = 14 pixels para cima
    float offset_x = (sw - p->width) / 2;
    float offset_y = (sh - p->height);

    // sx, sy (Recorte da imagem)
    float sx = p->current_frame * sw; 
    float sy = p->anim_row * sh; 

    // dx, dy (Posição na tela com compensação)
    float dx = (p->x - world->camera_x) - offset_x;
    float dy = (p->y - world->camera_y) - offset_y;

    int flags = 0;
    if (p->facing_direction == -1) {
        flags = ALLEGRO_FLIP_HORIZONTAL;
    }

    al_draw_bitmap_region(
        p->spritesheet, 
        sx, sy,         
        sw, sh,         
        dx, dy,         
        flags           
    );
    
    // DICA: Se quiser ver a caixa verde real da colisão para testar, descomente abaixo:
    al_draw_rectangle(
        p->x - world->camera_x, p->y - world->camera_y, 
        p->x + p->width - world->camera_x, p->y + p->height - world->camera_y, 
        al_map_rgb(0, 255, 0), 1
    );
    
}