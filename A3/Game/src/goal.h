#ifndef GOAL_H
#define GOAL_H

#include <allegro5/allegro5.h>
#include "world.h"
#include "player.h"

typedef struct {
    float x, y;
    int width, height; // Hitbox
    int sprite_width, sprite_height;
    ALLEGRO_BITMAP *sprite;
} Goal;

Goal* goal_create(float x, float y);

void goal_destroy(Goal *g);

// Checa colisão com o jogador
void goal_update(Goal *g, Player *p);

void goal_draw(Goal *g, World *w);

#endif