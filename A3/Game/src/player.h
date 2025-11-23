#ifndef PLAYER_H
#define PLAYER_H

#include <stdbool.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include "world.h"

typedef enum {
    IDLE,
    WALKING,
    JUMPING,
    CROUCHING,
    INTERACTING,
    WIN
} PlayerState;

typedef struct Player{
    float x, y;
    int hp;
    float width, height;
    float vel_x, vel_y;

    PlayerState state;
    bool on_ground;

    bool move_left;
    bool move_right;
    bool jump_pressed;

    ALLEGRO_BITMAP *spritesheet; // folha de sprites
    ALLEGRO_BITMAP *heart_icon;

    float invulnerable_timer;
    int facing_direction;    // 1 == Direita, -1 = Esquerda
    int anim_row;           // Qual linha da spritesheet (0=parado, 1=andando)
    int current_frame;      // Qual coluna (frame) dessa animação
    float frame_timer;      // Um contador para saber quando mudar de frame
    float frame_delay;      // O tempo (em segundos) entre cada frame
    int num_frames;         // Quantos frames tem a animação atual
} Player;

Player *player_create(float start_x, float start_y);

void player_destroy(Player *p);

void player_handle_event(Player *p, ALLEGRO_EVENT *event);

void player_update(Player *p, World *world);

void player_draw(Player *p, World *world);

// Tenta aplicar dano. Retorna true se o dano foi aplicado, false se estava invulnerável.
bool player_take_damage(Player *p);

// Teleporta o jogador de volta ao início.
void player_respawn(Player *p);

#endif