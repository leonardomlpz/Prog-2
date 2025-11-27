#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include "world.h"
#include "player.h"

typedef enum {
    SPIKE_HEAD_VERTICAL,
    SPIKE_HEAD_HORIZONTAL,
    SAW_HORIZONTAL,
    ROCK_HEAD_RECTANGULAR,
    GOAL_TROPHY
} ObstacleType;

typedef enum {
    OBS_STATE_MOVING,
    OBS_STATE_HIT
} ObstacleState;

typedef struct {
    float x, y;
    float vel_x, vel_y; 
    
    float start_x, start_y;
    
    ObstacleState state;
    ObstacleType type;
    int sprite_width, sprite_height;
    int width, height;
    ALLEGRO_BITMAP *spritesheet;
    
    int anim_row;
    int current_frame;
    float frame_timer;
    float frame_delay;
    int num_frames;

} Obstacle;

Obstacle* obstacle_create(ObstacleType type, float x, float y);

void obstacle_destroy(Obstacle *obs);

void obstacle_update(Obstacle *obs, Player *p, World *world);

void obstacle_draw(Obstacle *obs, World *world);

#endif