#ifndef WORLD_H
#define WORLD_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <stdbool.h>

struct Player; 

// Constantes do Mapa
#define TILE_SIZE 16
#define MAP_WIDTH 80
#define MAP_HEIGHT 30

typedef struct {
    int* tiles;    
    int width;
    int height;
    int tile_size;
    float camera_x;
    float camera_y;

    ALLEGRO_BITMAP *bg_image;
    ALLEGRO_BITMAP *tileset_sheet;

    int tileset_cols; // largura da folha
} World;

World* world_create();

void world_destroy(World* m);

void world_update(World *m, struct Player *p);

void world_draw(World* m);

bool world_is_solid(World* m, int x, int y);

int world_get_tile(World *m, int x, int y);

#endif