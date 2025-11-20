#include "obstacle.h"
#include "game.h" // Para FPS
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
        obs->vel_y = 5; // Velocidade de patrulha
        obs->patrol_range = 0; // Sobe e desce 100 pixels
        obs->num_frames = 4; // Animação "Blink"
    }
    else if (type == ROCK_HEAD_RECTANGULAR) {
        obs->spritesheet = al_load_bitmap("assets/Traps/rock_head_sheet.png");
        
        obs->width = 36;  
        obs->height = 36;
        obs->sprite_height = 42;
        obs->sprite_width = 42;
        // Começa descendo (Velocidade Y positiva)
        obs->vel_x = 0;
        obs->vel_y = 10.0; 
        
        obs->state = OBS_STATE_MOVING;
        obs->anim_row = 1;     // Começa piscando (Blink)
        obs->num_frames = 4;   
        obs->frame_delay = 0.2;
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

// --- Atualizar Lógica ---
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

            // Se o estado era "HIT" (bateu), precisamos voltar a mover
            if (obs->state == OBS_STATE_HIT) {
                obs->state = OBS_STATE_MOVING;
                
                // LÓGICA ESPECÍFICA DO ROCK HEAD (Gira em sentido horário)
                if (obs->type == ROCK_HEAD_RECTANGULAR) {
                    if (obs->anim_row == 3) { // Bateu em BAIXO -> Vai para ESQUERDA
                        obs->vel_y = 0; obs->vel_x = -10.0;
                    } 
                    else if (obs->anim_row == 4) { // Bateu na ESQUERDA -> Vai para CIMA
                        obs->vel_x = 0; obs->vel_y = -10.0;
                    }
                    else if (obs->anim_row == 2) { // Bateu em CIMA -> Vai para DIREITA
                        obs->vel_y = 0; obs->vel_x = 10.0;
                    }
                    else if (obs->anim_row == 5) { // Bateu na DIREITA -> Vai para BAIXO
                        obs->vel_x = 0; obs->vel_y = 10.0;
                    }
                }
                // LÓGICA DO SPIKE HEAD (Apenas inverte Y ou X)
                else if (obs->type == SPIKE_HEAD_VERTICAL) {
                    if (obs->anim_row == 2) obs->vel_y = 5.0;
                    else obs->vel_y = -5.0;
                }
                // (Adicione SPIKE_HEAD_HORIZONTAL aqui se precisar)

                // Volta para animação padrão (Blink)
                obs->anim_row = 1;
                obs->num_frames = 4;
            }
        }
    }

    // ===============================================
    // 2. ATUALIZA O MOVIMENTO
    // ===============================================
    
    // Só move se não estiver batendo (HIT)
    if (obs->state == OBS_STATE_MOVING) {
        
        // --- Lógica do SPIKE HEAD VERTICAL ---
        if (obs->type == SPIKE_HEAD_VERTICAL) {
            obs->y += obs->vel_y;
            if (obs->vel_y > 0) { 
                if (world_is_solid(world, obs->x + obs->width/2, obs->y + obs->height)) {
                    obs->y = (int)(obs->y + obs->height) / TILE_SIZE * TILE_SIZE - obs->height;
                    obs->vel_y = 0;
                    obs->state = OBS_STATE_HIT;
                    obs->anim_row = 3; // Bottom Hit
                    obs->current_frame = 0;
                }
            } else if (obs->vel_y < 0) {
                if (world_is_solid(world, obs->x + obs->width/2, obs->y)) {
                    obs->y = ((int)obs->y / TILE_SIZE + 1) * TILE_SIZE;
                    obs->vel_y = 0;
                    obs->state = OBS_STATE_HIT;
                    obs->anim_row = 2; // Top Hit
                    obs->current_frame = 0;
                }
            }
        }
        // --- Lógica da SERRA e SPIKE HORIZONTAL ---
        else if (obs->type == SAW_HORIZONTAL || obs->type == SPIKE_HEAD_HORIZONTAL) {
            obs->x += obs->vel_x;
            if (obs->vel_x > 0) {
                if (world_is_solid(world, obs->x + obs->width, obs->y + obs->height/2)) {
                    obs->x = (int)(obs->x + obs->width) / TILE_SIZE * TILE_SIZE - obs->width;
                    obs->vel_x *= -1;
                }
            } else if (obs->vel_x < 0) {
                if (world_is_solid(world, obs->x, obs->y + obs->height/2)) {
                    obs->x = ((int)obs->x / TILE_SIZE + 1) * TILE_SIZE;
                    obs->vel_x *= -1;
                }
            }
        }
        // --- Lógica do ROCK HEAD (Retangular) ---
        else if (obs->type == ROCK_HEAD_RECTANGULAR) {
            obs->x += obs->vel_x;
            obs->y += obs->vel_y;

            // Caindo
            if (obs->vel_y > 0 && world_is_solid(world, obs->x + obs->width/2, obs->y + obs->height)) {
                obs->y = (int)(obs->y + obs->height) / TILE_SIZE * TILE_SIZE - obs->height;
                obs->vel_y = 0; obs->state = OBS_STATE_HIT; obs->anim_row = 3; obs->current_frame = 0;
            }
            // Subindo
            else if (obs->vel_y < 0 && world_is_solid(world, obs->x + obs->width/2, obs->y)) {
                obs->y = ((int)obs->y / TILE_SIZE + 1) * TILE_SIZE;
                obs->vel_y = 0; obs->state = OBS_STATE_HIT; obs->anim_row = 2; obs->current_frame = 0;
            }
            // Direita
            else if (obs->vel_x > 0 && world_is_solid(world, obs->x + obs->width, obs->y + obs->height/2)) {
                obs->x = (int)(obs->x + obs->width) / TILE_SIZE * TILE_SIZE - obs->width;
                obs->vel_x = 0; obs->state = OBS_STATE_HIT; obs->anim_row = 5; obs->current_frame = 0;
            }
            // Esquerda
            else if (obs->vel_x < 0 && world_is_solid(world, obs->x, obs->y + obs->height/2)) {
                obs->x = ((int)obs->x / TILE_SIZE + 1) * TILE_SIZE;
                obs->vel_x = 0; obs->state = OBS_STATE_HIT; obs->anim_row = 4; obs->current_frame = 0;
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

        // Se houver intersecção (AABB)
        if (!(ox1 > px2 || ox2 < px1 || oy1 > py2 || oy2 < py1)) {
            
            bool crush = false;

            // 1. VERIFICA ESMAGAMENTO (Prioridade Máxima)
            if (obs->vel_y > 0) { // Descendo: Esmaga no chão
                if (world_is_solid(world, p->x + p->width/2, p->y + p->height + 1)) crush = true;
            }
            else if (obs->vel_y < 0) { // Subindo: Esmaga no teto
                if (world_is_solid(world, p->x + p->width/2, p->y - 1)) crush = true;
            }
            else if (obs->vel_x > 0) { // Direita: Esmaga na parede direita
                if (world_is_solid(world, p->x + p->width + 1, p->y + p->height/2)) crush = true;
            }
            else if (obs->vel_x < 0) { // Esquerda: Esmaga na parede esquerda
                if (world_is_solid(world, p->x - 1, p->y + p->height/2)) crush = true;
            }

            if (crush) {
                printf("ESMAGADO!\n");
                if (player_take_damage(p)) player_respawn(p);
            }
            else {
                // 2. LÓGICA DE ELEVADOR (Plataforma Móvel)
                // Se o pé do jogador está próximo ao topo da pedra E ele não está subindo (pulo)
                // (Usamos +10 de tolerância para garantir que pegue o contato)
                if (py2 <= oy1 + 15 && p->vel_y >= 0) {
                    
                    // "Gruda" o jogador no topo
                    p->y = obs->y - p->height;
                    p->vel_y = 0;
                    p->on_ground = true; // Permite pular de cima da pedra!
                    
                    // Move o jogador junto com a pedra (Horizontalmente)
                    // (Verticalmente já é resolvido pelo 'p->y = obs->y...' acima a cada frame)
                    p->x += obs->vel_x;
                    
                    // Se o jogador estava caindo (anim_row == 3), reseta para IDLE
                    if (p->anim_row == 3) {
                        p->state = IDLE; // Garante o estado
                        p->anim_row = 0; // Linha do Idle (Ninja)
                        p->num_frames = 11; // Idle do Ninja tem 11 frames
                        p->current_frame = 0;
                        p->frame_delay = 0.1; // Velocidade normal do Ninja
                    }
                } 
                else {
                    // 3. DANO DE CONTATO (Se tocar nos lados ou em baixo)
                    if (player_take_damage(p)) player_respawn(p);
                }
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