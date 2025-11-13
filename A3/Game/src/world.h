#ifndef WORLD_H
#define WORLD_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <stdbool.h> // Para usar 'bool'

struct Player; 

// --- Constantes do Mapa ---
#define TILE_SIZE 16
#define MAP_WIDTH 50
#define MAP_HEIGHT 19

// --- Estrutura do Mapa ---
typedef struct {
    int* tiles;    
    int width;
    int height;
    int tile_size;
    float camera_x;
    float camera_y;

    ALLEGRO_BITMAP *bg_image;
    ALLEGRO_BITMAP *tile_image;
} World;


// --- Protótipos das Funções ---

World* world_create();

void world_destroy(World* m);

// Porque o "apelido" typedef "Player" não está visível aqui.
void world_update(World *m, struct Player *p);

void world_draw(World* m);

bool world_is_solid(World* m, int x, int y);

#endif // WORLD_H