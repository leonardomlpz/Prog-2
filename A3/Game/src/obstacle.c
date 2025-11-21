#include "obstacle.h"
#include "game.h" // Para FPS
#include "world.h"
#include <stdio.h>
#include <stdlib.h>

Obstacle* obstacle_create(ObstacleType type, float x, float y) {
    Obstacle *obs = (Obstacle*) malloc(sizeof(Obstacle));
    if (!obs) return NULL;

    obs->x = x;
    obs->y = y;
    obs->start_x = x; // Define a "âncora"
    obs->start_y = y;
    obs->type = type;
    
    obs->spritesheet = NULL;
    
    // Lógica de animação
    obs->state = OBS_STATE_MOVING;
    obs->anim_row = 1;
    obs->current_frame = 0;
    obs->frame_timer = 0;
    obs->frame_delay = 0.4; // Velocidade da animação

    // Define as propriedades baseado no tipo
    if (type == SPIKE_HEAD_VERTICAL) {
        obs->spritesheet = al_load_bitmap("assets/Traps/spike_head_sheet.png");
        obs->width = 42;
        obs->height = 42;
        obs->sprite_width = 54;
        obs->sprite_height = 52;
        obs->vel_x = 0;
        obs->vel_y = 6.0; // Velocidade de patrulha
        obs->anim_row = 1;
        obs->num_frames = 4; // Animação "Blink"
        obs->frame_delay = 0.2;
    }
    else if (type == ROCK_HEAD_RECTANGULAR) {
        obs->spritesheet = al_load_bitmap("assets/Traps/rock_head_sheet.png");
        
        obs->width = 36;  
        obs->height = 36;
        obs->sprite_height = 42;
        obs->sprite_width = 42;
        // Começa descendo (Velocidade Y positiva)
        obs->vel_x = 0;
        obs->vel_y = 6.0; 
        
        obs->state = OBS_STATE_MOVING;
        obs->anim_row = 1;     // Começa piscando (Blink)
        obs->num_frames = 4;   
        obs->frame_delay = 0.2;
    }
    
    else if (type == SAW_HORIZONTAL) {
        obs->spritesheet = al_load_bitmap("assets/Traps/saw_sheet.png");
        
        // Hitbox um pouco menor que 38 já que a serra é redonda
        obs->width = 30;  
        obs->height = 30;
        
        // Tamanho da Imagem
        obs->sprite_width = 38;
        obs->sprite_height = 38;

        obs->vel_x = 2.0; // Velocidade
        obs->vel_y = 0;

        obs->state = OBS_STATE_MOVING;
        obs->anim_row = 0;     // Só tem 1 linha
        obs->num_frames = 8;   // A serra tem 8 frames girando
        obs->frame_delay = 0.05; // Gira rápido
    }

    if (!obs->spritesheet) {
        printf("ERRO: Nao foi possivel carregar a folha de sprites do obstaculo\n");
        free(obs);
        return NULL;
    }
    
    return obs;
}

// --- Destruir Obstáculo ---
void obstacle_destroy(Obstacle *obs) {
    if (obs) {
        if (obs->spritesheet) al_destroy_bitmap(obs->spritesheet);
        free(obs);
    }
}

void obstacle_update(Obstacle *obs, Player *p, World *world) {
    
    // ===============================================
    // 1. ATUALIZA A ANIMAÇÃO (Timer)
    // ===============================================
    obs->frame_timer += 1.0 / FPS; 
    if (obs->frame_timer >= obs->frame_delay) {
        obs->frame_timer = 0;
        obs->current_frame++;

        if (obs->current_frame >= obs->num_frames) {
            obs->current_frame = 0; 

            // Se o estado era "HIT" (bateu), volta ao normal
            if (obs->state == OBS_STATE_HIT) {
                obs->state = OBS_STATE_MOVING;
                
                // LÓGICA ESPECÍFICA DO ROCK HEAD (Gira em sentido ANTI-HORÁRIO)
                if (obs->type == ROCK_HEAD_RECTANGULAR) {
                    
                    if (obs->anim_row == 3) { // Bateu em BAIXO -> Vai para DIREITA
                        obs->vel_y = 0; obs->vel_x = 10.0; 
                    } 
                    else if (obs->anim_row == 5) { // Bateu na DIREITA -> Vai para CIMA
                        obs->vel_x = 0; obs->vel_y = -10.0;
                    }
                    else if (obs->anim_row == 2) { // Bateu em CIMA -> Vai para ESQUERDA
                        obs->vel_y = 0; obs->vel_x = -10.0;
                    }
                    else if (obs->anim_row == 4) { // Bateu na ESQUERDA -> Vai para BAIXO
                        obs->vel_x = 0; obs->vel_y = 10.0;
                    }
                }
                // LÓGICA DO SPIKE HEAD (Apenas inverte Y ou X)
                else if (obs->type == SPIKE_HEAD_VERTICAL) {
                    // Se bateu em cima (Top Hit), desce com velocidade positiva
                    if (obs->anim_row == 2) obs->vel_y = 6.0; // <--- Mude de 5.0 para 10.0
                    // Se bateu em baixo (Bottom Hit), sobe com velocidade negativa
                    else obs->vel_y = -6.0;                   // <--- Mude de -5.0 para -10.0
                }

                // --- DEFINE A ANIMAÇÃO PADRÃO DE VOLTA ---
                if (obs->type == SAW_HORIZONTAL) {
                    obs->anim_row = 0; // Serra usa linha 0
                    obs->num_frames = 8;
                } else {
                    obs->anim_row = 1; // Outros usam Blink (linha 1)
                    obs->num_frames = 4;
                }
            }
        }
    }

    // ===============================================
    // 2. ATUALIZA O MOVIMENTO
    // ===============================================
    
    if (obs->state == OBS_STATE_MOVING) {
        
        // --- LÓGICA DA SERRA (Segue o Trilho / Medo de Altura) ---
        if (obs->type == SAW_HORIZONTAL) {
            obs->x += obs->vel_x;

            // Verifica o tile exatamente no CENTRO da serra
            int cx = obs->x + obs->width / 2;
            int cy = obs->y + obs->height / 2;
            int tile_id = world_get_tile(world, cx, cy);

            // Se tocar no AR (0), significa que o trilho acabou
            if (tile_id == 0) {
                obs->x -= obs->vel_x; // Desfaz o passo que saiu
                obs->vel_x *= -1;     // Inverte a direção
            }
        }
        
        // --- LÓGICA DO ROCK HEAD (Movimento Quadrado) ---
        else if (obs->type == ROCK_HEAD_RECTANGULAR) {
            obs->x += obs->vel_x;
            obs->y += obs->vel_y;

            // Colisões com o mapa (mudam estado para HIT)
            if (obs->vel_y > 0 && world_is_solid(world, obs->x + obs->width/2, obs->y + obs->height)) {
                obs->y = (int)(obs->y + obs->height) / TILE_SIZE * TILE_SIZE - obs->height;
                obs->vel_y = 0; obs->state = OBS_STATE_HIT; obs->anim_row = 3; obs->current_frame = 0;
            }
            else if (obs->vel_y < 0 && world_is_solid(world, obs->x + obs->width/2, obs->y)) {
                obs->y = ((int)obs->y / TILE_SIZE + 1) * TILE_SIZE;
                obs->vel_y = 0; obs->state = OBS_STATE_HIT; obs->anim_row = 2; obs->current_frame = 0;
            }
            else if (obs->vel_x > 0 && world_is_solid(world, obs->x + obs->width, obs->y + obs->height/2)) {
                obs->x = (int)(obs->x + obs->width) / TILE_SIZE * TILE_SIZE - obs->width;
                obs->vel_x = 0; obs->state = OBS_STATE_HIT; obs->anim_row = 5; obs->current_frame = 0;
            }
            else if (obs->vel_x < 0 && world_is_solid(world, obs->x, obs->y + obs->height/2)) {
                obs->x = ((int)obs->x / TILE_SIZE + 1) * TILE_SIZE;
                obs->vel_x = 0; obs->state = OBS_STATE_HIT; obs->anim_row = 4; obs->current_frame = 0;
            }
        }
        
        // --- LÓGICA DO SPIKE HEAD VERTICAL (Simples) ---
        else if (obs->type == SPIKE_HEAD_VERTICAL) {
            obs->y += obs->vel_y;
            if (obs->vel_y > 0) { 
                if (world_is_solid(world, obs->x + obs->width/2, obs->y + obs->height)) {
                    obs->y = (int)(obs->y + obs->height) / TILE_SIZE * TILE_SIZE - obs->height;
                    obs->vel_y = 0; obs->state = OBS_STATE_HIT; obs->anim_row = 3; obs->current_frame = 0;
                }
            } else if (obs->vel_y < 0) {
                if (world_is_solid(world, obs->x + obs->width/2, obs->y)) {
                    obs->y = ((int)obs->y / TILE_SIZE + 1) * TILE_SIZE;
                    obs->vel_y = 0; obs->state = OBS_STATE_HIT; obs->anim_row = 2; obs->current_frame = 0;
                }
            }
        }
    }

    // ===============================================
    // 3. CHECA COLISÃO COM O JOGADOR
    // ===============================================
    float ox1 = obs->x; float oy1 = obs->y;
    float ox2 = obs->x + obs->width; float oy2 = obs->y + obs->height;
    float px1 = p->x; float py1 = p->y;
    float px2 = p->x + p->width; float py2 = p->y + p->height;
    
    // Se houver intersecção
    if (!(ox1 > px2 || ox2 < px1 || oy1 > py2 || oy2 < py1)) {
        
        // ROCK HEAD: Verifica Esmagamento + Elevador
        if (obs->type == ROCK_HEAD_RECTANGULAR) {
            bool crush = false;
            if (obs->vel_y > 0 && world_is_solid(world, p->x + p->width/2, p->y + p->height + 1)) crush = true;
            else if (obs->vel_y < 0 && world_is_solid(world, p->x + p->width/2, p->y - 1)) crush = true;
            else if (obs->vel_x > 0 && world_is_solid(world, p->x + p->width + 1, p->y + p->height/2)) crush = true;
            else if (obs->vel_x < 0 && world_is_solid(world, p->x - 1, p->y + p->height/2)) crush = true;

            if (crush) {
                printf("ESMAGADO!\n");
                if (player_take_damage(p)) player_respawn(p);
            } else {
                // Elevador (Se estiver em cima)
                if (py2 <= oy1 + 15 && p->vel_y >= 0) {
                    p->y = obs->y - p->height;
                    p->vel_y = 0;
                    p->on_ground = true; 
                    p->x += obs->vel_x;
                    
                    if (p->anim_row == 3) { // Reseta animação de queda
                        p->state = IDLE; p->anim_row = 0; p->num_frames = 11;
                        p->current_frame = 0; p->frame_delay = 0.1;
                    }
                } else {
                    if (player_take_damage(p)) player_respawn(p);
                }
            }
        }
        // SERRAS E SPIKES: Dano Imediato
        else {
            if (player_take_damage(p)) player_respawn(p);
        }
    }
}

// --- Desenhar Obstáculo ---
void obstacle_draw(Obstacle *obs, World *world) {
    
    // Usa as variáveis que guardamos na struct!
    float sw = obs->sprite_width;
    float sh = obs->sprite_height; 

    // Cálculo de Offset (Centraliza a hitbox no sprite)
    float offset_x = (sw - obs->width) / 2.0;
    float offset_y = (sh - obs->height) / 2.0;
    
    // Posição do RECORTE
    float sx = obs->current_frame * sw; 
    float sy = obs->anim_row * sh; 

    // Posição do DESENHO
    float dx = (obs->x - world->camera_x) - offset_x;
    float dy = (obs->y - world->camera_y) - offset_y;

    if (obs->spritesheet) {
        al_draw_bitmap_region(
            obs->spritesheet, 
            sx, sy,
            sw, sh,
            dx, dy,
            0
        );
    }
    
    // Debug Hitbox (se quiser ver o quadrado verde)
    al_draw_rectangle(obs->x - world->camera_x, obs->y - world->camera_y, 
                      obs->x + obs->width - world->camera_x, obs->y + obs->height - world->camera_y, 
                      al_map_rgb(0, 255, 0), 1);
}