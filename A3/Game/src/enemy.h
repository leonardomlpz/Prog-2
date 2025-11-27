#ifndef ENEMY_H
#define ENEMY_H

#include <allegro5/allegro5.h>
#include "world.h"
#include "player.h"

typedef enum {
    ENEMY_RHINO,
    ENEMY_MUSHROOM,
    ENEMY_BLUEBIRD
} EnemyType;

typedef enum {
    ENEMY_STATE_IDLE,
    ENEMY_STATE_RUN,
    ENEMY_STATE_HIT_WALL // Específico do Rinoceronte
} EnemyState;

typedef struct {
    float x, y;
    float vel_x, vel_y;
    int width, height;          // Hitbox física
    int sprite_width, sprite_height; // Tamanho do desenho
    int facing_direction;       // 1 = Direita, -1 = Esquerda

    EnemyType type;
    EnemyState state;
    
    // Inteligência Artificial
    float sight_range; // Distância que ele enxerga o jogador
    float state_timer; // Timer genérico (ex: tempo atordoado)

    // Animação
    ALLEGRO_BITMAP *spritesheet;
    int anim_row;
    int current_frame;
    float frame_timer;
    float frame_delay;
    int num_frames;

} Enemy;

Enemy* enemy_create(EnemyType type, float x, float y);

void enemy_destroy(Enemy *e);

void enemy_update(Enemy *e, Player *p, World *w);

void enemy_draw(Enemy *e, World *w);

#endif