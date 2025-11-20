#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "world.h"
#include "game.h"
#include "player.h"

// --- Carregar Mapa do CSV ---
// (Função "privada", só o world.c precisa dela)
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
            
            // Salva o GID real do Tiled
            // (0 para ar, IDs > 0 para blocos)
            m->tiles[y * m->width + x] = tile_gid;

            token = strtok(NULL, ",");
            x++;
        }
        y++;
    }
    fclose(file);
    printf("Mapa '%s' carregado com sucesso!\n", filename);
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
    m->camera_x = 0;
    m->camera_y = 0;

    // 1. Aloca memória para os tiles (zerado)
    m->tiles = (int*) calloc(m->width * m->height, sizeof(int)); // Use calloc
    if (!m->tiles) {
        printf("Erro ao alocar tiles do world\n");
        free(m);
        return NULL;
    }

    // 2. Carrega as imagens
    m->bg_image = al_load_bitmap("assets/background.png");
    if (!m->bg_image) printf("Aviso: Nao foi possivel carregar assets/background.png\n");

    // Carrega a FOLHA MESTRA
    m->tileset_sheet = al_load_bitmap("assets/tileset_mestre.png");
    if (!m->tileset_sheet) {
        printf("ERRO: Nao foi possivel carregar assets/tileset_mestre.png\n");
    } else {
        // Calcula quantas colunas de tiles tem a folha
        m->tileset_cols = al_get_bitmap_width(m->tileset_sheet) / m->tile_size;
    }

    // 3. Carrega o mapa do CSV (que vai preencher m->tiles)
    world_load_csv(m, "assets/level1.csv");

    printf("World criado!\n");
    return m;
}

// --- Destruir Mapa ---
void world_destroy(World *m) {
    if (m) {
        if (m->tiles) free(m->tiles);
        if (m->bg_image) al_destroy_bitmap(m->bg_image);
        if (m->tileset_sheet) al_destroy_bitmap(m->tileset_sheet); // Limpa a folha mestra

        free(m);
        printf("World destruído.\n");
    }
}

// --- Desenhar Mapa ---
void world_draw(World *m) {
    // 1. Desenha o Background com Parallax (Seu código está correto)
    if (m->bg_image) {
        int bg_w = al_get_bitmap_width(m->bg_image);
        float screen_w = SCREEN_WIDTH / GAME_SCALE;
        float parallax_speed = 0.5; 
        float bg_x = -((int)(m->camera_x * parallax_speed) % bg_w);
        al_draw_bitmap(m->bg_image, bg_x, 0, 0);
        if (bg_x + bg_w < screen_w) {
            al_draw_bitmap(m->bg_image, bg_x + bg_w, 0, 0);
        }
    }

    // 2. Desenha os Tiles do Mapa
    if (!m->tileset_sheet) return; // Não desenha se a folha mestra falhou

    for (int y = 0; y < m->height; y++) {
        for (int x = 0; x < m->width; x++) {
            
            int gid = m->tiles[y * m->width + x];
            
            // Se gid for 0 (Ar), não desenha nada
            if (gid > 0) {
                // Tiled GIDs começam em 1, nosso grid de imagem começa em 0
                int tile_id = gid - 1; 
                
                // Calcula a posição (sx, sy) do tile na Folha Mestra
                int sx = (tile_id % m->tileset_cols) * m->tile_size;
                int sy = (tile_id / m->tileset_cols) * m->tile_size;
                
                // Posição de desenho na tela
                float dx = x * m->tile_size - m->camera_x;
                float dy = y * m->tile_size - m->camera_y;
                
                // Recorta e desenha o tile correto
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

// --- Verificar Colisão ---
bool world_is_solid(World *m, int x, int y) {
    int tile_gid = world_get_tile(m, x, y);

    // Ar (0) não é sólido
    if (tile_gid == 0) return false;

    // Espinho não é sólido, você atravessa
    if (tile_gid == 243) return false;

    // Correntes não são solidas
    if (tile_gid > 60 && tile_gid < 64) return false;

    // Se não for ar e não for perigo, deve ser sólido
    return true;
}

// --- Atualizar Câmera ---
void world_update(World *m, struct Player *p) {
    
    // Tela virtual (800 / 3 = aprox 266 pixels de visão)
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