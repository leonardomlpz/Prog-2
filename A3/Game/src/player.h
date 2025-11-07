#ifndef PLAYER_H
#define PLAYER_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>

typedef enum {
    IDLE,
    WALKING,
    JUMPING,
    CROUCHING,
    INTERACTING
} PlayerState;

typedef struct {
    float x, y;
    int hp;
    float width, height;
    float vel_x, vel_y;

    PlayerState state;
    bool on_ground;

    bool move_left;
    bool move_right;
} Player;

Player *player_create(float start_x, float start_y);

void player_destroy(Player *p);

void player_handle_event(Player *p, ALLEGRO_EVENT *event);

void player_update(Player*p);

void player_draw(Player *p);

#endif