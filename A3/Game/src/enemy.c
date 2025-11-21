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
    // 1. Atualiza Animação
    e->frame_timer += 1.0 / FPS;
    if (e->frame_timer >= e->frame_delay) {
        e->frame_timer = 0;
        e->current_frame++;
        if (e->current_frame >= e->num_frames) {
            e->current_frame = 0;
            
            // Se acabou a animação de "Bateu na Parede", volta a ficar calmo
            if (e->state == ENEMY_STATE_HIT_WALL) {
                e->state = ENEMY_STATE_IDLE;
                e->anim_row = 0;
                e->num_frames = 11;
                e->frame_delay = 0.1;
                e->state_timer = 1.5;
            }
        }
    }

    // 2. Gravidade (Inimigos também caem!)
    e->vel_y += 0.5; // Gravidade simples
    e->y += e->vel_y;
    
    // Colisão com chão
    if (e->vel_y > 0) {
        if (world_is_solid(w, e->x + e->width/2, e->y + e->height)) {
            e->y = (int)(e->y + e->height) / TILE_SIZE * TILE_SIZE - e->height;
            e->vel_y = 0;
        }
    }

    // 3. Inteligência Artificial (Rhino)
    if (e->type == ENEMY_RHINO) {
        
        // --- ESTADO: ESPERANDO (IDLE) ---
        if (e->state == ENEMY_STATE_IDLE) {
            e->vel_x = 0;
            
            // Se estiver atordoado (timer > 0), diminui o tempo e não faz nada
            if (e->state_timer > 0) {
                e->state_timer -= 1.0 / FPS;
                return; // Sai da função (não procura o jogador)
            }

            // Verifica se o jogador está na mesma altura (plataforma)
            // abs() pega o valor absoluto (distância positiva)
            if (fabs(p->y - e->y) < 32) {
                // Verifica a distância horizontal
                float dist_x = p->x - e->x;
                
                // Se estiver perto o suficiente
                if (fabs(dist_x) < e->sight_range) {
                    // ATACAR!
                    e->state = ENEMY_STATE_RUN;
                    e->anim_row = 1; // Run
                    e->num_frames = 6;
                    e->frame_delay = 0.05; // Corre rápido
                    
                    // Define direção (corre atrás do player)
                    if (dist_x > 0) {
                        e->vel_x = 4.0;
                        e->facing_direction = -1; // O sprite do Rhino original olha pra esquerda?
                        // Nota: Pixel Adventure geralmente tem sprites olhando para a ESQUERDA por padrão.
                        // Se o seu olha pra esquerda, então:
                        // Direita = Flip Horizontal
                        // Esquerda = Normal
                    } else {
                        e->vel_x = -4.0;
                        e->facing_direction = 1; 
                    }
                }
            }
        }
        
        // --- ESTADO: CORRENDO (RUN) ---
        else if (e->state == ENEMY_STATE_RUN) {
            e->x += e->vel_x;
            
            // Verifica colisão com parede lateral
            bool hit_wall = false;
            if (e->vel_x > 0) { // Direita
                if (world_is_solid(w, e->x + e->width, e->y + e->height/2)) hit_wall = true;
            } else { // Esquerda
                if (world_is_solid(w, e->x, e->y + e->height/2)) hit_wall = true;
            }
            
            if (hit_wall) {
                // Bateu! Fica atordoado.
                e->x -= e->vel_x; // Desfaz o passo
                e->vel_x = 0;
                
                e->state = ENEMY_STATE_HIT_WALL;
                e->anim_row = 2; // Hit Wall
                e->num_frames = 5;
                e->current_frame = 0;
                e->frame_delay = 0.1;
                
                // Efeito de tremer a câmera poderia ser adicionado aqui!
            }
        }
    }
    
    // 4. Dano no Jogador (Colisão Simples)
    // Se tocar no jogador enquanto corre, machuca
    float ex1 = e->x; float ey1 = e->y;
    float ex2 = e->x + e->width; float ey2 = e->y + e->height;
    float px1 = p->x; float py1 = p->y;
    float px2 = p->x + p->width; float py2 = p->y + p->height;

    if (!(ex1 > px2 || ex2 < px1 || ey1 > py2 || ey2 < py1)) {
        // Se estiver correndo, dá dano
        if (e->state == ENEMY_STATE_RUN) {
            if (player_take_damage(p)) player_respawn(p);
        }
        // Se estiver parado ou atordoado, talvez não dê dano (opcional)
    }
}

void enemy_draw(Enemy *e, World *w) {
    float sw = e->sprite_width;
    float sh = e->sprite_height; 
    
    float offset_x = (sw - e->width) / 2.0;
    float offset_y = (sh - e->height) / 2.0;

    float sx = e->current_frame * sw; 
    float sy = e->anim_row * sh; 

    float dx = (e->x - w->camera_x) - offset_x;
    float dy = (e->y - w->camera_y) - offset_y;

    int flags = 0;
    // Ajuste de espelhamento (depende de como o sprite foi desenhado)
    // Se o sprite original olha para ESQUERDA:
    if (e->vel_x > 0) flags = ALLEGRO_FLIP_HORIZONTAL; // Inverte pra olhar pra direita
    
    // Se estiver parado, mantém a última direção (pode precisar de ajuste fino)
    if (e->vel_x == 0 && e->facing_direction == -1) flags = ALLEGRO_FLIP_HORIZONTAL;

    if (e->spritesheet) {
        al_draw_bitmap_region(e->spritesheet, sx, sy, sw, sh, dx, dy, flags);
    }
    
    // Debug Hitbox
    al_draw_rectangle(e->x - w->camera_x, e->y - w->camera_y, 
                      e->x + e->width - w->camera_x, e->y + e->height - w->camera_y, 
                      al_map_rgb(255, 0, 0), 1);
}