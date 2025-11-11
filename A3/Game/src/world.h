#ifndef WORLD_H
#define WORLD_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <stdbool.h> // Para usar 'bool'

// --- Constantes do Mapa ---

// O tamanho de cada bloco (tile) em pixels
#define TILE_SIZE 32

// As dimensões do nosso mapa, em blocos
// (Vamos começar com um mapa de 50 blocos de largura e 19 de altura)
#define MAP_WIDTH 50
#define MAP_HEIGHT 19 // (Para 600 pixels de tela / 32 = 18.75, então 19)


// --- Estrutura do Mapa ---

typedef struct {
    // Um array 1D para guardar o mapa (0 = ar, 1 = chão)
    // Usamos 1D (MAP_WIDTH * MAP_HEIGHT) em vez de 2D (mapa[y][x])
    // porque é mais fácil de alocar dinamicamente.
    int* tiles;
    
    int width;    // Largura em blocos (MAP_WIDTH)
    int height;   // Altura em blocos (MAP_HEIGHT)
    int tile_size; // Tamanho do bloco em pixels (TILE_SIZE)

} World;


// --- Protótipos das Funções ---

// Aloca memória e "desenha" o mapa inicial no array 'tiles'
World* world_create();

// Libera a memória do mapa
void world_destroy(World* m);

// Desenha os blocos do mapa na tela
void world_draw(World* m);

// A função mais importante:
// Verifica se uma coordenada EM PIXELS (x, y) está dentro de um bloco sólido
bool world_is_solid(World* m, int x, int y);

#endif // WORLD_H