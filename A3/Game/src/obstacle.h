#ifndef OBSTACLE_H
#define OBSTACLE_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_image.h>
#include "world.h" // Para desenhar com a câmera
#include "player.h" // Para checar colisão com o player

typedef enum {
    SPIKE_HEAD_VERTICAL, // O Spike Head que sobe e desce
    SPIKE_HEAD_HORIZONTAL,
    ROCK_HEAD_VERTICAL,
    ROCK_HEAD_HORIZONTAL
} ObstacleType;

typedef enum {
    OBS_STATE_MOVING,    // Estado normal
    OBS_STATE_HIT        // Estado de "bateu"
} ObstacleState;

typedef struct {
    float x, y;         // Posição no mundo
    float vel_x, vel_y; 
    
    // Variáveis de patrulha
    float start_x, start_y; // Posição inicial (âncora)
    float patrol_range;     // Distância que ele anda (ex: 100 pixels)
    
    // Propriedades físicas e visuais
    ObstacleState state;
    ObstacleType type;
    int width, height; // Tamanho da Hitbox
    ALLEGRO_BITMAP *spritesheet;
    
    // Variáveis de animação (copiadas do player.h)
    int anim_row;
    int current_frame;
    float frame_timer;
    float frame_delay;
    int num_frames;

} Obstacle;

// Cria um novo obstáculo em uma posição (x,y)
Obstacle* obstacle_create(ObstacleType type, float x, float y);

// Libera a memória
void obstacle_destroy(Obstacle *obs);

// Atualiza a lógica (movimento, animação, colisão com player)
void obstacle_update(Obstacle *obs, Player *p, World *world);

// Desenha o obstáculo na tela
void obstacle_draw(Obstacle *obs, World *world);

#endif