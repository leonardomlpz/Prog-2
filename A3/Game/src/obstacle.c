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
        obs->vel_x = 0;
        obs->vel_y = 1.5; // Velocidade de patrulha
        obs->patrol_range = 0; // Sobe e desce 100 pixels
        obs->num_frames = 4; // Animação "Blink"
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

            // Se o estado era "HIT" (bateu)...
            if (obs->state == OBS_STATE_HIT) {
                
                // --- A ANIMAÇÃO DE 'HIT' TERMINOU, VOLTE A MOVER ---
                obs->state = OBS_STATE_MOVING; // ...volte para "MOVING"
                
                // Decide a PRÓXIMA direção baseado em QUAL "hit" acabou
                if (obs->anim_row == 2) { // Se foi "Top Hit" (Linha 2)
                    obs->vel_y = 2.0; // ...agora desce
                } else { // Se foi "Bottom Hit" (Linha 3)
                    obs->vel_y = -2.0; // ...agora sobe
                }
                
                // Configura a animação de "Blink"
                obs->anim_row = 1;
                obs->num_frames = 4;
            }
        }
    }

    // ===============================================
    // 2. ATUALIZA O MOVIMENTO (Patrulha por Colisão)
    // ===============================================
    
    // O obstáculo SÓ se move se estiver no estado "MOVING"
    // Se estiver em "HIT", este bloco inteiro é pulado.
    if (obs->state == OBS_STATE_MOVING) {
        obs->y += obs->vel_y;
        
        if (obs->vel_y > 0) { // Caindo
            if (world_is_solid(world, obs->x + obs->width/2, obs->y + obs->height)) {
                // Bateu no chão!
                obs->y = (int)(obs->y + obs->height) / TILE_SIZE * TILE_SIZE - obs->height; // Snap
                
                // --- ATIVA O ESTADO DE HIT ---
                obs->vel_y = 0; // PARA de mover
                obs->state = OBS_STATE_HIT;
                obs->anim_row = 3;           // Animação "Bottom Hit" (Linha 3)
                obs->num_frames = 4;
                obs->current_frame = 0;
            }
        } 
        else if (obs->vel_y < 0) { // Subindo
            if (world_is_solid(world, obs->x + obs->width/2, obs->y)) {
                // Bateu no teto!
                obs->y = ((int)obs->y / TILE_SIZE + 1) * TILE_SIZE; // Snap
                
                // --- ATIVA O ESTADO DE HIT ---
                obs->vel_y = 0; // PARA de mover
                obs->state = OBS_STATE_HIT;
                obs->anim_row = 2;           // Animação "Top Hit" (Linha 2)
                obs->num_frames = 4;
                obs->current_frame = 0;
            }
        }
    } // Fim do if (obs->state == OBS_STATE_MOVING)

    // ===============================================
    // 3. CHECA COLISÃO COM O JOGADOR
    // ===============================================
    float obs_x1 = obs->x;
    float obs_y1 = obs->y;
    float obs_x2 = obs->x + obs->width;
    float obs_y2 = obs->y + obs->height;
    
    float player_x1 = p->x;
    float player_y1 = p->y;
    float player_x2 = p->x + p->width;
    float player_y2 = p->y + p->height;
    
    if (obs_x1 > player_x2 || obs_x2 < player_x1 || obs_y1 > player_y2 || obs_y2 < player_y1) {
        // Sem colisão
    } else {
        // COLISÃO!
        player_take_damage(p); 
    }
}

// --- Desenhar Obstáculo ---
void obstacle_draw(Obstacle *obs, World *world) {
    
    // 1. Tamanho do RECORTE (o frame na folha, sempre 54x52)
    float sw = 54; // Largura do frame original
    float sh = 52; // Altura do frame original

    // 2. Cálculo de Offset (Centraliza a hitbox 36x36 no sprite 54x52)
    // (54 - 36) / 2 = 9 pixels de offset em X
    // (52 - 36) / 2 = 8 pixels de offset em Y
    float offset_x = (sw - obs->width) / 2.0;
    float offset_y = (sh - obs->height) / 2.0;
    
    // 3. Posição do RECORTE (sx, sy) na folha de sprites
    float sx = obs->current_frame * sw; 
    float sy = obs->anim_row * sh; 

    // 4. Posição da FÍSICA (hitbox) na tela
    float phys_dx = obs->x - world->camera_x;
    float phys_dy = obs->y - world->camera_y;

    // 5. Posição do DESENHO (sprite) na tela
    // (A posição da física menos o offset)
    float draw_dx = phys_dx - offset_x;
    float draw_dy = phys_dy - offset_y;

    // Desenha o SPRITE (imagem)
    al_draw_bitmap_region(
        obs->spritesheet, 
        sx, sy,
        sw, sh,
        draw_dx, draw_dy,
        0
    );
    
    // Desenha a HITBOX (debug)
    al_draw_rectangle(
        phys_dx,                 // Posição X da física
        phys_dy,                 // Posição Y da física
        phys_dx + obs->width,    // X + Largura da física
        phys_dy + obs->height,   // Y + Altura da física
        al_map_rgb(0, 255, 0), // Cor Verde
        1                     
    );
}