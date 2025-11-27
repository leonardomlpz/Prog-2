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
    // --- NOVO: Salva a posição original ---
    e->start_x = x; 
    e->start_y = y;
    // --------------------------------------

    e->type = type;
    e->vel_x = 0;
    e->vel_y = 0;
    e->facing_direction = -1; // Começa olhando para a esquerda
    e->state_timer = 0;

    // Animação Padrão
    e->current_frame = 0;
    e->frame_timer = 0;
    
    // --- CONFIGURAÇÃO DO RHINO ---
    if (type == ENEMY_RHINO) {
        e->spritesheet = al_load_bitmap("assets/Enemies/rhino_sheet.png");
        e->width = 42;
        e->height = 28;
        e->sprite_width = 52;
        e->sprite_height = 34;
        e->sight_range = 150;
        e->state = ENEMY_STATE_IDLE;
        e->anim_row = 0;
        e->num_frames = 11;
        e->frame_delay = 0.1;
    }
    // --- CONFIGURAÇÃO DO BLUE BIRD ---
    else if (type == ENEMY_BLUEBIRD) {
        e->spritesheet = al_load_bitmap("assets/Enemies/bluebird_sheet.png");
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
        e->frame_delay = 0.08; 
    }
    // --- CONFIGURAÇÃO DO COGUMELO ---
    else if (type == ENEMY_MUSHROOM) {
        e->spritesheet = al_load_bitmap("assets/Enemies/mushroom.png");
        
        e->width = 26;
        e->height = 20; // Baixinho
        e->sprite_width = 32;
        e->sprite_height = 32;

        e->state = ENEMY_STATE_RUN;
        e->vel_x = -1.5; // Começa andando para a esquerda devagar
        e->vel_y = 0;

        e->anim_row = 0;     
        e->num_frames = 14;  // Sua imagem tem 14 frames na linha
        e->frame_delay = 0.1; 
    }

    if (!e->spritesheet) {
        printf("ERRO: Nao foi possivel carregar sprite do inimigo tipo %d.\n", type);
    }

    return e;
}

void enemy_destroy(Enemy *e) {
    if (e) {
        if (e->spritesheet) al_destroy_bitmap(e->spritesheet);
        free(e);
    }
}

// --- NOVO: Função de Reset ---
void enemy_reset(Enemy *e) {
    // Restaura posição
    e->x = e->start_x;
    e->y = e->start_y;
    e->vel_y = 0;
    e->state_timer = 0;

    // Reseta estados específicos
    if (e->type == ENEMY_RHINO) {
        e->state = ENEMY_STATE_IDLE;
        e->vel_x = 0;
        e->anim_row = 0;
        e->num_frames = 11;
    }
    else if (e->type == ENEMY_BLUEBIRD) {
        e->state = ENEMY_STATE_RUN;
        e->vel_x = 0;
        e->anim_row = 0; // Volta a bater asas
        e->num_frames = 9;
    }
    else if (e->type == ENEMY_MUSHROOM) {
        e->state = ENEMY_STATE_RUN;
        e->vel_x = -1.5; // Volta a andar
    }
}

void enemy_update(Enemy *e, Player *p, World *w) {
    if (e->type == ENEMY_BLUEBIRD && e->x <= -1000) {
        // Se está morto, o state_timer funciona como contagem regressiva
        if (e->state_timer > 0) {
            e->state_timer -= 1.0 / FPS;
        } 
        else {
            // Tempo acabou, renasce!
            enemy_reset(e); 
            // O reset zera o state_timer, então o seno vai começar do 0 (suave)
            printf("Passaro respawnou!\n");
        }
        return; // Sai da função, não processa o resto
    }
    // 1. ATUALIZA ANIMAÇÃO
    e->frame_timer += 1.0 / FPS;
    if (e->frame_timer >= e->frame_delay) {
        e->frame_timer = 0;
        e->current_frame++;
        if (e->current_frame >= e->num_frames) {
            e->current_frame = 0;
            
            // Lógica Pós-Hit
            if (e->type == ENEMY_RHINO && e->state == ENEMY_STATE_HIT_WALL) {
                e->state = ENEMY_STATE_IDLE;
                e->anim_row = 0; e->num_frames = 11; e->frame_delay = 0.1; e->state_timer = 1.5;
            }
            else if (e->type == ENEMY_BLUEBIRD && e->state == ENEMY_STATE_HIT_WALL) {
                e->x = -5000; // Some com o pássaro morto
            }
        }
    }

    // 2. FÍSICA E MOVIMENTO
    
    // --- GRAVIDADE ---
    // Aplica gravidade para Rhino e Cogumelo (Pássaro não)
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

    // --- RHINO AI ---
    if (e->type == ENEMY_RHINO) {
        // (Sua lógica do Rhino mantida aqui...)
        if (e->state == ENEMY_STATE_IDLE) {
            e->vel_x = 0;
            if (e->state_timer > 0) { e->state_timer -= 1.0 / FPS; return; }
            if (fabs(p->y - e->y) < 32) {
                float dist_x = p->x - e->x;
                if (fabs(dist_x) < e->sight_range) {
                    e->state = ENEMY_STATE_RUN;
                    e->anim_row = 1; e->num_frames = 6; e->frame_delay = 0.05; 
                    e->vel_x = (dist_x > 0) ? 4.0 : -4.0;
                    e->facing_direction = (e->vel_x > 0) ? -1 : 1;
                }
            }
        }
        else if (e->state == ENEMY_STATE_RUN) {
            e->x += e->vel_x;
            bool hit_wall = false;
            if (e->vel_x > 0 && world_is_solid(w, e->x + e->width, e->y + e->height/2)) hit_wall = true;
            else if (e->vel_x < 0 && world_is_solid(w, e->x, e->y + e->height/2)) hit_wall = true;
            if (hit_wall) {
                e->x -= e->vel_x; e->vel_x = 0;
                e->state = ENEMY_STATE_HIT_WALL;
                e->anim_row = 2; e->num_frames = 5; e->current_frame = 0; e->frame_delay = 0.1;
            }
        }
    }

    // --- BLUE BIRD AI (Trampolim) ---
    else if (e->type == ENEMY_BLUEBIRD && e->state != ENEMY_STATE_HIT_WALL) {
        // Voo senoidal (Sobe e Desce suavemente usando Start Y)
        e->state_timer += 1.0 / FPS;
        e->y = e->start_y + cos(e->state_timer * 5.0) * 10.0;
    }

    // --- MUSHROOM AI (Patrulha Simples) ---
    else if (e->type == ENEMY_MUSHROOM) {
        e->x += e->vel_x;

        // Verifica colisão lateral para virar
        bool hit_wall = false;
        if (e->vel_x > 0) { // Direita
            if (world_is_solid(w, e->x + e->width + 1, e->y + e->height/2)) hit_wall = true;
        } else { // Esquerda
            if (world_is_solid(w, e->x - 1, e->y + e->height/2)) hit_wall = true;
        }

        if (hit_wall) {
            e->vel_x *= -1; // Vira para o outro lado
        }

        // Define a direção do sprite
        // No Pixel Adventure, geralmente Direita = Flip (-1) e Esquerda = Normal (1)
        if (e->vel_x > 0) e->facing_direction = -1;
        else e->facing_direction = 1;
    }
    
    // 3. COLISÃO COM JOGADOR
    if (e->x <= -1000) return;

    float ex1 = e->x; float ey1 = e->y;
    float ex2 = e->x + e->width; float ey2 = e->y + e->height;
    float px1 = p->x; float py1 = p->y;
    float px2 = p->x + p->width; float py2 = p->y + p->height;

    if (!(ex1 > px2 || ex2 < px1 || ey1 > py2 || ey2 < py1)) {
        
        // Trampolim (Blue Bird)
        bool is_above = (p->y + p->height) < (e->y + e->height/2 + 10);
        if (e->type == ENEMY_BLUEBIRD && p->vel_y > 0 && is_above && e->state != ENEMY_STATE_HIT_WALL) {
            p->vel_y = -9.0;
            p->on_ground = false;
            e->state = ENEMY_STATE_HIT_WALL;
            e->anim_row = 1; e->num_frames = 3; e->current_frame = 0; e->frame_delay = 0.1;
        } 
        // Dano
        else {
            if (e->state != ENEMY_STATE_HIT_WALL) {
                if (player_take_damage(p)) player_respawn(p);
            }
        }
    }
}

void enemy_draw(Enemy *e, World *w) {
    if (e->x <= -1000) return; 

    float sw = e->sprite_width;
    float sh = e->sprite_height; 
    
    // Centraliza horizontalmente (Eixo X) - Igual para todos
    float offset_x = (sw - e->width) / 2.0;

    // Eixo Y: Decide com base no tipo
    float offset_y = 0;

    if (e->type == ENEMY_BLUEBIRD) {
        // Voadores: Centraliza (divide a sobra por 2)
        offset_y = (sh - e->height) / 2.0;
    } 
    else {
        // Terrestres (Rhino/Cogumelo): Alinha pela base (joga a sobra toda pra cima)
        offset_y = sh - e->height;
    }

    float sx = e->current_frame * sw; 
    float sy = e->anim_row * sh; 
    
    // Calcula posição final
    float dx = (e->x - w->camera_x) - offset_x;
    float dy = (e->y - w->camera_y) - offset_y;

    int flags = 0;
    // Lógica de virar o sprite
    if (e->type == ENEMY_RHINO || e->type == ENEMY_MUSHROOM) {
         if (e->facing_direction == -1) flags = ALLEGRO_FLIP_HORIZONTAL;
    }

    if (e->spritesheet) {
        al_draw_bitmap_region(e->spritesheet, sx, sy, sw, sh, dx, dy, flags);
    }

    // Debug Hitbox (Vermelho) - Para você conferir se resolveu
    /*al_draw_rectangle(
        e->x - w->camera_x, 
        e->y - w->camera_y, 
        e->x + e->width - w->camera_x, 
        e->y + e->height - w->camera_y, 
        al_map_rgb(255, 0, 0), 1
    );*/
}