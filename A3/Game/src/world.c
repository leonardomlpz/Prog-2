#include "world.h"
#include <stdio.h>
#include <stdlib.h>

// Função auxiliar para pegar um tile em (x, y) no array 1D
// (Isso torna o código em `world_create` mais legível)
static void set_tile(World *m, int x, int y, int value) {
    // Verifica se está dentro dos limites para evitar erros
    if (x >= 0 && x < m->width && y >= 0 && y < m->height) {
        m->tiles[y * m->width + x] = value;
    }
}

// --- Criar Mapa ---
World *world_create() {
    World *m = (World*) malloc(sizeof(World));
    if (!m) {
        printf("Erro ao alocar world\n");
        return NULL;
    }

    m->width = MAP_WIDTH;
    m->height = MAP_HEIGHT;
    m->tile_size = TILE_SIZE;

    // Aloca memória para todos os blocos do mapa
    m->tiles = (int*) calloc(m->width * m->height, sizeof(int));
    if (!m->tiles) {
        printf("Erro ao alocar tiles do world\n");
        free(m);
        return NULL;
    }

    // --- Design da Fase (0 = ar, 1 = chão) ---
    // Vamos criar um chão simples com alguns buracos
    for (int x = 0; x < m->width; x++) {
        // Chão principal (duas linhas de altura)
        set_tile(m, x, m->height - 1, 1);
        set_tile(m, x, m->height - 2, 1);

        // Um buraco
        if (x > 10 && x < 15) {
            set_tile(m, x, m->height - 1, 0);
            set_tile(m, x, m->height - 2, 0);
        }
        
        // Outro buraco
        if (x > 30 && x < 35) {
            set_tile(m, x, m->height - 1, 0);
            set_tile(m, x, m->height - 2, 0);
        }
    }
    
    // Uma plataforma flutuante
    for (int x = 20; x < 28; x++) {
        set_tile(m, x, m->height - 6, 1);
    }
    
    printf("World criado!\n");
    return m;
}

// --- Destruir Mapa ---
void world_destroy(World *m) {
    if (m) {
        if (m->tiles) {
            free(m->tiles);
        }
        free(m);
        printf("World destruído.\n");
    }
}

// --- Desenhar Mapa ---
void world_draw(World *m) {
    ALLEGRO_COLOR floor_color = al_map_rgb(100, 100, 100); // Cinza
    
    for (int y = 0; y < m->height; y++) {
        for (int x = 0; x < m->width; x++) {
            
            // Se o bloco no mapa for 1 (sólido)
            if (m->tiles[y * m->width + x] == 1) {
                
                // Desenha um retângulo na posição correta
                al_draw_filled_rectangle(
                    x * m->tile_size,         // x1
                    y * m->tile_size,         // y1
                    (x + 1) * m->tile_size,   // x2
                    (y + 1) * m->tile_size,   // y2
                    floor_color
                );
            }
        }
    }
}

// --- Verificar Colisão ---
bool world_is_solid(World *m, int x, int y) {
    // 1. Verifica se (x, y) está fora dos limites do mundo
    if (x < 0 || x >= (m->width * m->tile_size) ||
        y < 0 || y >= (m->height * m->tile_size)) {
        return true; // Trata "fora do mapa" como uma parede sólida
    }

    // 2. Converte as coordenadas de PIXELS para blocos (TILES)
    int tile_x = x / m->tile_size;
    int tile_y = y / m->tile_size;

    // 3. Verifica o valor do bloco nesse índice
    if (m->tiles[tile_y * m->width + tile_x] == 1) {
        return true; // É sólido
    }

    return false; // Não é sólido (é ar)
}