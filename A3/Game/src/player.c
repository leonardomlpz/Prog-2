#include "player.h"
#include "game.h"   
#include "world.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <allegro5/allegro_primitives.h>

#define PLAYER_SPEED 3.75
#define JUMP_POWER 10.0
#define GRAVITY 0.5

// Alturas da hitbox
#define HEIGHT_STANDING 24
#define HEIGHT_CROUCHING 12

Player* player_create(float start_x, float start_y) {
    Player* p = (Player*) malloc(sizeof(Player));
    if (!p) return NULL;

    p->x = start_x;
    p->y = start_y;
    p->width = 18;
    p->height = HEIGHT_STANDING; // Começa em pé
    p->vel_x = 0;
    p->vel_y = 0;
    p->hp = 3;
    p->state = IDLE;
    p->on_ground = false;

    p->move_left = false;
    p->move_right = false;
    p->jump_pressed = false;
    p->crouch_pressed = false;
    p->level_complete = false;

    p->spritesheet = al_load_bitmap("assets/ninja_sheet.png");
    p->heart_icon = al_load_bitmap("assets/heart.png");

    p->invulnerable_timer = 0.0;
    p->anim_row = 0;        
    p->current_frame = 0;
    p->frame_timer = 0;
    p->frame_delay = 0.05;
    p->num_frames = 11;
    p->facing_direction = 1;

    return p;
}

void player_destroy(Player* p) {
    if (p) {
        if (p->spritesheet) al_destroy_bitmap(p->spritesheet);
        if (p->heart_icon) al_destroy_bitmap(p->heart_icon);
        free(p);
    }
}

void player_handle_event(Player* p, ALLEGRO_EVENT* event) {
    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        switch(event->keyboard.keycode) {
            case ALLEGRO_KEY_W: p->jump_pressed = true; break;
            case ALLEGRO_KEY_A: p->move_left = true; break;
            case ALLEGRO_KEY_D: p->move_right = true; break;
            case ALLEGRO_KEY_S: 
            case ALLEGRO_KEY_DOWN: 
                p->crouch_pressed = true; 
                break;
        }
    } 
    else if (event->type == ALLEGRO_EVENT_KEY_UP) {
        switch(event->keyboard.keycode) {
            case ALLEGRO_KEY_W: p->jump_pressed = false; break;
            case ALLEGRO_KEY_A: p->move_left = false; break;
            case ALLEGRO_KEY_D: p->move_right = false; break;
            case ALLEGRO_KEY_S: 
            case ALLEGRO_KEY_DOWN: 
                p->crouch_pressed = false; 
                break;
        }
    }
}

void player_update(Player *p, World *world) {
    if (p->invulnerable_timer > 0) p->invulnerable_timer -= 1.0 / FPS;

    // MÁQUINA DE ESTADOS E INPUTS
    // Se estiver em HIT (Dano), bloqueia inputs de movimento
    if (p->state == HIT) {
        // Não faz nada, apenas espera. O código de física/colisão abaixo vai empurrá-lo.
    }
    else {
        // MUDANÇA DE ESTADO (AGACHAR)
        if (p->on_ground && p->crouch_pressed) {
            if (p->state != CROUCHING) {
                p->state = CROUCHING;
                p->y = p->y + (HEIGHT_STANDING - HEIGHT_CROUCHING);
                p->height = HEIGHT_CROUCHING;
            }
        } 
        else {
            if (p->state == CROUCHING) {
                bool hit_head = world_is_solid(world, p->x, p->y - (HEIGHT_STANDING - HEIGHT_CROUCHING)) ||
                                world_is_solid(world, p->x + p->width, p->y - (HEIGHT_STANDING - HEIGHT_CROUCHING));
                if (!hit_head) {
                    p->state = IDLE;
                    p->y = p->y - (HEIGHT_STANDING - HEIGHT_CROUCHING);
                    p->height = HEIGHT_STANDING;
                }
            }
        }

        // MOVIMENTO HORIZONTAL
        p->vel_x = 0;
        if (p->state != CROUCHING) {
            if (p->move_left)  p->vel_x -= PLAYER_SPEED;
            if (p->move_right) p->vel_x += PLAYER_SPEED;
        }

        if (p->vel_x > 0) p->facing_direction = 1;
        else if (p->vel_x < 0) p->facing_direction = -1;

        // PULO
        if (p->jump_pressed && p->on_ground && p->state != CROUCHING) {
            p->vel_y = -JUMP_POWER;
            p->state = JUMPING;
            p->on_ground = false;
            p->anim_row = 2; p->num_frames = 1; p->current_frame = 0;
        }
    } // Fim do else (Controles Normais)


    // FÍSICA E COLISÃO
    p->vel_y += GRAVITY;
    if (p->vel_y > 15) p->vel_y = 15;

    // Colisão X
    p->x += p->vel_x;
    if (p->vel_x > 0) { 
        if (world_is_solid(world, p->x + p->width, p->y) ||
            world_is_solid(world, p->x + p->width, p->y + p->height - 1)) {
            p->x = (int)(p->x + p->width) / TILE_SIZE * TILE_SIZE - p->width;
            p->vel_x = 0;
        }
    }
    else if (p->vel_x < 0) { 
        if (world_is_solid(world, p->x, p->y) ||
            world_is_solid(world, p->x, p->y + p->height - 1)) {
            p->x = ((int)p->x / TILE_SIZE + 1) * TILE_SIZE;
            p->vel_x = 0;
        }
    }

    // Colisão Y
    p->y += p->vel_y;
    p->on_ground = false; 

    if (p->vel_y > 0) { 
        if (world_is_solid(world, p->x + 1, p->y + p->height) ||
            world_is_solid(world, p->x + p->width - 1, p->y + p->height)) {
            p->y = (int)(p->y + p->height) / TILE_SIZE * TILE_SIZE - p->height;
            p->vel_y = 0;
            p->on_ground = true;
            
            // Reset de pulo (mas não reseta se for HIT)
            if (p->state == JUMPING) p->state = IDLE;
        }
    }
    else if (p->vel_y < 0) { 
        if (world_is_solid(world, p->x + 1, p->y) ||
            world_is_solid(world, p->x + p->width - 1, p->y)) {
            p->y = ((int)p->y / TILE_SIZE + 1) * TILE_SIZE;
            p->vel_y = 0; 
        }
    }

    // ANIMAÇÃO
    if (p->state == HIT) {
        // Mantém a linha 4. Não faz nada aqui, a animação roda sozinha.
    }
    else if (p->state == CROUCHING) {
        p->anim_row = 0; p->num_frames = 11; 
    }
    else if (p->state == JUMPING || !p->on_ground) {
        if (p->vel_y > 0) p->anim_row = 3; 
        else p->anim_row = 2; 
        p->num_frames = 1;
    }
    else if (p->vel_x != 0) {
        p->state = WALKING;
        p->anim_row = 1; p->num_frames = 12; p->frame_delay = 0.05;
    } 
    else {
        p->state = IDLE;
        p->anim_row = 0; p->num_frames = 11; p->frame_delay = 0.05;
    }

    // Avanço de frames (para resetar o HIT)
    p->frame_timer += 1.0 / FPS; 
    if (p->frame_timer >= p->frame_delay) {
        p->current_frame++;
        
        // Verifica se a animação acabou
        if (p->current_frame >= p->num_frames) {
            p->current_frame = 0;
            
            // SE A ANIMAÇÃO DE HIT ACABOU, VOLTA AO NORMAL
            if (p->state == HIT) {
                p->state = IDLE;
            }
        }
        p->frame_timer = 0;
    }

    // Dano do espinho
    int tile_at_player = world_get_tile(world, p->x + p->width/2, p->y + p->height/2);
    if (tile_at_player == 243) { 
        if (player_take_damage(p)) player_respawn(p);
    }
}

bool player_take_damage(Player *p) {
    // Se já estiver invulnerável, ignora o dano
    if (p->invulnerable_timer > 0) return false;

    p->hp--; 
    
    // Retorna true para que o main.c saiba que deve chamar o player_respawn
    if (p->hp <= 0) return true;

    // SE SOBREVIVEU: configura o estado HIT e a animação
    p->state = HIT;
    p->invulnerable_timer = 2.0; 
    
    // Configura para a linha 4 (Animação de Hit)
    p->anim_row = 4;
    p->num_frames = 5; 
    p->current_frame = 0;
    p->frame_delay = 0.1; 

    // Empurrão (Knockback)
    p->vel_y = -4; 
    p->vel_x = -4 * p->facing_direction; 
    
    // IMPORTANTE: Retorna false!
    // Isso diz ao jogo: "Não reinicie a fase, apenas continue com o jogador machucado".
    return false; 
}

void player_respawn(Player *p) {
    p->x = 32;
    p->y = 400;
    p->vel_x = 0;
    p->vel_y = 0;
    p->state = IDLE;
    p->height = HEIGHT_STANDING; // Garante que respawna em pé
}

void player_reset(Player *p) {
    player_respawn(p);
    p->hp = 3;  
    p->level_complete = false; 
    p->invulnerable_timer = 0;
}

void player_draw(Player *p, World *world) {
    float sw = 32; 
    float sh = 32; 

    float sx = p->current_frame * sw; 
    float sy = p->anim_row * sh; 

    float draw_scale_y = 1.0;
    if (p->state == CROUCHING) {
        draw_scale_y = 0.6; 
    }

    float dest_w = sw;
    float dest_h = sh * draw_scale_y; 

    float offset_x = (sw - p->width) / 2.0;
    float dy = (p->y + p->height) - dest_h - world->camera_y;
    float dx = (p->x - world->camera_x) - offset_x;

    int flags = 0;
    if (p->facing_direction == -1) flags = ALLEGRO_FLIP_HORIZONTAL;

    // ALTERAÇÃO: Efeito de Piscar
    bool draw = true;
    if (p->invulnerable_timer > 0) {
        // Pisca rapidamente
        int blink = (int)(p->invulnerable_timer * 10);
        if (blink % 2 == 0) draw = false;
    }

    if (draw) {
        al_draw_scaled_bitmap(
            p->spritesheet, 
            sx, sy,         
            sw, sh,         
            dx, dy,         
            dest_w, dest_h, 
            flags           
        );
    }
    // ------------------------------------

    // Debug Hitbox (Pode descomentar se quiser ver)
    /*al_draw_rectangle(
        p->x - world->camera_x, p->y - world->camera_y, 
        p->x + p->width - world->camera_x, p->y + p->height - world->camera_y, 
        al_map_rgb(0, 255, 0), 1
    );*/
}