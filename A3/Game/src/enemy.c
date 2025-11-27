#include "enemy.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h> // Para abs() e fabs()

Enemy* enemy_create(EnemyType type, float x, float y) {
    Enemy *e = (Enemy*) malloc(sizeof(Enemy));
    if (!e) return NULL;

    e->x = x;
    e->y = y;
    e->type = type;
    e->vel_x = 0;
    e->vel_y = 0;
    e->facing_direction = -1; // Começa olhando para a esquerda (padrão do sprite)
    e->state_timer = 0;

    // Animação Padrão
    e->current_frame = 0;
    e->frame_timer = 0;
    
    if (type == ENEMY_RHINO) {
        e->spritesheet = al_load_bitmap("assets/Enemies/rhino_sheet.png");
        e->width = 42;   // Hitbox menor que o sprite
        e->height = 28;
        e->sprite_width = 52;
        e->sprite_height = 34;
        
        e->sight_range = 150; // Enxerga x pixels
        
        // Começa em IDLE
        e->state = ENEMY_STATE_IDLE;
        e->anim_row = 0;
        e->num_frames = 11;
        e->frame_delay = 0.1;
    }

    else if (type == ENEMY_BLUEBIRD) {
        e->spritesheet = al_load_bitmap("assets/Enemies/bluebird_sheet.png");
        
        // Hitbox (menor que o sprite 32x32 para ficar justo)
        e->width = 24;
        e->height = 24;
        
        e->sprite_width = 32;
        e->sprite_height = 32;

        e->sight_range = 0; 
        
        e->state = ENEMY_STATE_RUN; 
        e->vel_x = 0;
        e->vel_y = 0;

        e->anim_row = 0;     
        e->num_frames = 9;   
        e->frame_delay = 0.05; 
    }

    if (!e->spritesheet) {
        printf("ERRO: Nao foi possivel carregar sprite do inimigo.\n");
    }

    return e;
}

void enemy_destroy(Enemy *e) {
    if (e) {
        if (e->spritesheet) al_destroy_bitmap(e->spritesheet);
        free(e);
    }
}

void enemy_update(Enemy *e, Player *p, World *w) {
    
    // ============================================================
    // 1. ATUALIZA ANIMAÇÃO
    // ============================================================
    e->frame_timer += 1.0 / FPS;
    if (e->frame_timer >= e->frame_delay) {
        e->frame_timer = 0;
        e->current_frame++;
        if (e->current_frame >= e->num_frames) {
            e->current_frame = 0;
            
            // Lógica Pós-Hit (Rhino)
            if (e->type == ENEMY_RHINO && e->state == ENEMY_STATE_HIT_WALL) {
                e->state = ENEMY_STATE_IDLE;
                e->anim_row = 0;
                e->num_frames = 11;
                e->frame_delay = 0.1;
                e->state_timer = 1.5;
            }
            // Lógica Pós-Hit (Blue Bird - Morte)
            else if (e->type == ENEMY_BLUEBIRD && e->state == ENEMY_STATE_HIT_WALL) {
                // Remove o pássaro da tela (move para longe)
                e->x = -5000; 
            }
        }
    }

    // ============================================================
    // 2. FÍSICA E MOVIMENTO
    // ============================================================
    
    // --- GRAVIDADE ---
    // Apenas aplica gravidade se NÃO for o pássaro
    if (e->type != ENEMY_BLUEBIRD) {
        e->vel_y += 0.5; 
        e->y += e->vel_y;
        
        // Colisão com chão
        if (e->vel_y > 0) {
            if (world_is_solid(w, e->x + e->width/2, e->y + e->height)) {
                e->y = (int)(e->y + e->height) / TILE_SIZE * TILE_SIZE - e->height;
                e->vel_y = 0;
            }
        }
    }

    // --- RHINO AI (Inteligência) ---
    if (e->type == ENEMY_RHINO) {
        if (e->state == ENEMY_STATE_IDLE) {
            e->vel_x = 0;
            if (e->state_timer > 0) {
                e->state_timer -= 1.0 / FPS;
                return; 
            }
            if (fabs(p->y - e->y) < 32) {
                float dist_x = p->x - e->x;
                if (fabs(dist_x) < e->sight_range) {
                    e->state = ENEMY_STATE_RUN;
                    e->anim_row = 1; 
                    e->num_frames = 6;
                    e->frame_delay = 0.05; 
                    if (dist_x > 0) {
                        e->vel_x = 4.0;
                        e->facing_direction = -1; 
                    } else {
                        e->vel_x = -4.0;
                        e->facing_direction = 1; 
                    }
                }
            }
        }
        else if (e->state == ENEMY_STATE_RUN) {
            e->x += e->vel_x;
            bool hit_wall = false;
            if (e->vel_x > 0 && world_is_solid(w, e->x + e->width, e->y + e->height/2)) hit_wall = true;
            else if (e->vel_x < 0 && world_is_solid(w, e->x, e->y + e->height/2)) hit_wall = true;
            
            if (hit_wall) {
                e->x -= e->vel_x; 
                e->vel_x = 0;
                e->state = ENEMY_STATE_HIT_WALL;
                e->anim_row = 2; 
                e->num_frames = 5;
                e->current_frame = 0;
                e->frame_delay = 0.1;
            }
        }
    }
    
    // ============================================================
    // 3. COLISÃO COM JOGADOR (DANO OU TRAMPOLIM)
    // ============================================================
    
    // Se o inimigo já estiver "morto" (longe), ignora
    if (e->x <= -1000) return;

    float ex1 = e->x; float ey1 = e->y;
    float ex2 = e->x + e->width; float ey2 = e->y + e->height;
    float px1 = p->x; float py1 = p->y;
    float px2 = p->x + p->width; float py2 = p->y + p->height;

    // Se houver intersecção entre os retângulos
    if (!(ex1 > px2 || ex2 < px1 || ey1 > py2 || ey2 < py1)) {
        
        // --- LÓGICA DO TRAMPOLIM (BLUE BIRD) ---
        // Condição: Player caindo (vel_y > 0) E Player acima do pássaro
        bool is_above = (p->y + p->height) < (e->y + e->height/2 + 10);

        if (e->type == ENEMY_BLUEBIRD && p->vel_y > 0 && is_above && e->state != ENEMY_STATE_HIT_WALL) {
            printf("Trampolim ativado!\n");
            
            // 1. Player pula alto
            p->vel_y = -9.0; // Força do pulo
            p->on_ground = false;

            // 2. Pássaro morre
            e->state = ENEMY_STATE_HIT_WALL; // Usamos esse estado para "Hit"
            e->anim_row = 1; // Linha de Hit na spritesheet
            e->num_frames = 3;
            e->current_frame = 0;
            e->frame_delay = 0.1;
        } 
        
        // --- LÓGICA DE DANO ---
        else {
            // Se não foi trampolim, então machuca
            // (Verifica se o inimigo não está atordoado/morrendo antes de dar dano)
            if (e->state != ENEMY_STATE_HIT_WALL) {
                if (player_take_damage(p)) player_respawn(p);
            }
        }
    }
}

void enemy_draw(Enemy *e, World *w) {
    if (e->x <= -1000) return; // Não desenha se já morreu

    float sw = e->sprite_width;
    float sh = e->sprite_height; 
    
    float offset_x = (sw - e->width) / 2.0;
    float offset_y = (sh - e->height) / 2.0;

    float sx = e->current_frame * sw; 
    float sy = e->anim_row * sh; 

    float dx = (e->x - w->camera_x) - offset_x;
    float dy = (e->y - w->camera_y) - offset_y;

    int flags = 0;
    
    // RHINO: Lógica de direção baseada na velocidade
    if (e->type == ENEMY_RHINO) {
        if (e->vel_x > 0) flags = ALLEGRO_FLIP_HORIZONTAL;
        else if (e->vel_x == 0 && e->facing_direction == -1) flags = ALLEGRO_FLIP_HORIZONTAL;
    }
    // BLUE BIRD: Como ele está parado, vamos deixar virado para a esquerda (padrão)
    // Se quiser virar para a direita, use: if (e->facing_direction == 1) flags = ALLEGRO_FLIP_HORIZONTAL;

    if (e->spritesheet) {
        al_draw_bitmap_region(e->spritesheet, sx, sy, sw, sh, dx, dy, flags);
    }
    
    // Debug Hitbox (Descomente para ver o quadrado vermelho)
    al_draw_rectangle(e->x - w->camera_x, e->y - w->camera_y, 
                      e->x + e->width - w->camera_x, e->y + e->height - w->camera_y, 
                      al_map_rgb(255, 0, 0), 1);
    
}