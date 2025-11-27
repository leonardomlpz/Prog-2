#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "world.h"
#include "game.h"
#include "player.h"

// Carregar Mapa do CSV
static void world_load_csv(World *m, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("ERRO: Nao foi possivel abrir %s\n", filename);
        return;
    }
    char buffer[4096]; 
    int y = 0;
    while (fgets(buffer, sizeof(buffer), file) && y < m->height) {
        char *token;
        int x = 0;
        token = strtok(buffer, ","); 
        while (token != NULL && x < m->width) {
            
            int tile_gid = atoi(token);
            m->tiles[y * m->width + x] = tile_gid;

            token = strtok(NULL, ",");
            x++;
        }
        y++;
    }
    fclose(file);
    printf("Mapa '%s' carregado com sucesso!\n", filename);
}

World *world_create() {
    World *m = (World*) malloc(sizeof(World));
    if (!m) {
        printf("Erro ao alocar world\n");
        return NULL;
    }

    m->width = MAP_WIDTH;
    m->height = MAP_HEIGHT;
    m->tile_size = TILE_SIZE;
    m->camera_x = 0;
    m->camera_y = 0;

    // Aloca memória para os tiles
    m->tiles = (int*) calloc(m->width * m->height, sizeof(int));
    if (!m->tiles) {
        printf("Erro ao alocar tiles do world\n");
        free(m);
        return NULL;
    }

    m->bg_image = al_load_bitmap("assets/background.png");
    if (!m->bg_image) printf("Aviso: Nao foi possivel carregar assets/background\n");

    m->tileset_sheet = al_load_bitmap("assets/tileset_mestre.png");
    if (!m->tileset_sheet) {
        printf("ERRO: Nao foi possivel carregar assets/tileset_mestre.png\n");
    } else {
        m->tileset_cols = al_get_bitmap_width(m->tileset_sheet) / m->tile_size;
    }

    world_load_csv(m, "assets/level1.csv");

    printf("World criado!\n");
    return m;
}

void world_destroy(World *m) {
    if (m) {
        if (m->tiles) free(m->tiles);
        if (m->bg_image) al_destroy_bitmap(m->bg_image);
        if (m->tileset_sheet) al_destroy_bitmap(m->tileset_sheet); 

        free(m);
        printf("World destruído.\n");
    }
}

void world_draw(World *m) {
    if (m->bg_image) {
        int bg_w = al_get_bitmap_width(m->bg_image);
        int bg_h = al_get_bitmap_height(m->bg_image);

        // Dimensões da tela virtual
        float screen_h = SCREEN_HEIGHT / GAME_SCALE;

        // Calcula a escala para a imagem caber exatamente na ALTURA da tela
        float scale = screen_h / (float)bg_h;
        
        // Largura final da imagem na tela após redimensionar
        float scaled_w = bg_w * scale;

        // CORREÇÃO DO LOOP INFINITO
        float parallax_speed = 0.5;
        
        // Calcula quanto já andamos no total
        float total_scroll = m->camera_x * parallax_speed;
        
        // Usa fmod para criar o "loop".
        float bg_x = -fmod(total_scroll, scaled_w);

        // Desenha a PRIMEIRA imagem (que vai sair pela esquerda)
        al_draw_scaled_bitmap(
            m->bg_image,
            0, 0, bg_w, bg_h,
            bg_x, 0, scaled_w, screen_h,
            0
        );
        
        // Desenha a SEGUNDA imagem (que vem entrando pela direita)
        al_draw_scaled_bitmap(
            m->bg_image,
            0, 0, bg_w, bg_h,
            bg_x + scaled_w, 0, scaled_w, screen_h,
            0
        );
    }

    // Desenha os Tiles do Mapa
    if (!m->tileset_sheet) return;

    for (int y = 0; y < m->height; y++) {
        for (int x = 0; x < m->width; x++) {
            
            int gid = m->tiles[y * m->width + x];
            
            if (gid > 0) {
                int tile_id = gid - 1; 
                int sx = (tile_id % m->tileset_cols) * m->tile_size;
                int sy = (tile_id / m->tileset_cols) * m->tile_size;
                float dx = x * m->tile_size - m->camera_x;
                float dy = y * m->tile_size - m->camera_y;
                
                al_draw_bitmap_region(
                    m->tileset_sheet,
                    sx, sy,
                    m->tile_size, m->tile_size,
                    dx, dy,
                    0
                );
            }
        }
    }
}

// Verificar Colisão
bool world_is_solid(World *m, int x, int y) {
    int tile_gid = world_get_tile(m, x, y);

    if (tile_gid == 0) return false; // Ar
    if (tile_gid == 243) return false; // Espinho
    if (tile_gid > 60 && tile_gid < 64) return false; // Correntes/Decoração

    return true;
}

// Atualizar Câmera
void world_update(World *m, struct Player *p) {
    float screen_w = SCREEN_WIDTH / GAME_SCALE;
    float screen_h = SCREEN_HEIGHT / GAME_SCALE;

    // Centraliza X
    m->camera_x = p->x - (screen_w / 2);

    // Travas X
    if (m->camera_x < 0) m->camera_x = 0;
    float max_x = (m->width * m->tile_size) - screen_w;
    if (m->camera_x > max_x) m->camera_x = max_x;

    // Centraliza Y
    m->camera_y = p->y - (screen_h / 2);

    // Travas Y
    if (m->camera_y < 0) m->camera_y = 0;
    float max_y = (m->height * m->tile_size) - screen_h;
    if (m->camera_y > max_y) m->camera_y = max_y;

    // Arredonda
    m->camera_x = floor(m->camera_x);
    m->camera_y = floor(m->camera_y);
}

int world_get_tile(World *m, int x, int y) {
    if (x < 0 || x >= m->width * m->tile_size || y < 0 || y >= m->height * m->tile_size) return 0;
    
    int tx = x / m->tile_size;
    int ty = y / m->tile_size;
    
    return m->tiles[ty * m->width + tx];
}