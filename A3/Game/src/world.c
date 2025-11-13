#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "world.h"
#include "game.h"
#include "player.h"

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

    m->camera_x = 0;
    m->camera_y = 0;

    // Carrega o background
    m->bg_image = al_load_bitmap("assets/background.png");
    if (!m->bg_image) {
        printf("Aviso: Nao foi possivel carregar assets/background.png\n");
    }
    // Carrega a textura do chão
    m->tile_image = al_load_bitmap("assets/tile_floor2.png");
    if (!m->tile_image) {
        printf("Aviso: Nao foi possivel carregar assets/tile_floor.png\n");
    }

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
        if (m->tiles)
            free(m->tiles);
        if (m->bg_image)
            al_destroy_bitmap(m->bg_image);
        if (m->tile_image)
            al_destroy_bitmap(m->tile_image);

        free(m);
        printf("World destruído.\n");
    }
}

// --- Desenhar Mapa ---
void world_draw(World *m) {
    // 1. Desenha o Background com Parallax
    if (m->bg_image) {
        // Largura da imagem
        int bg_w = al_get_bitmap_width(m->bg_image);
        int bg_h = al_get_bitmap_height(m->bg_image);
        
        // Largura da tela virtual (ajustada pelo zoom)
        // Precisamos saber até onde desenhar
        float screen_w = SCREEN_WIDTH / GAME_SCALE;
        
        // Cálculo da Posição X com Parallax
        // Multiplicamos por 0.5 para mover na metade da velocidade da câmera
        float parallax_speed = 0.5; 
        
        // O operador % garante que o valor sempre fique entre 0 e bg_w
        // O sinal negativo faz ele mover para a esquerda quando andamos para a direita
        float bg_x = -((int)(m->camera_x * parallax_speed) % bg_w);
        
        // Desenha a primeira cópia
        al_draw_bitmap(m->bg_image, bg_x, 0, 0);
        
        // Desenha a segunda cópia logo depois, para cobrir o buraco quando a primeira sai da tela
        if (bg_x + bg_w < screen_w) {
            al_draw_bitmap(m->bg_image, bg_x + bg_w, 0, 0);
        }
        
        // (Opcional: Se sua imagem for pequena verticalmente, você pode precisar repetir no Y também
        // ou apenas garantir que sua imagem de fundo tenha altura suficiente para o jogo).
    }

    ALLEGRO_COLOR floor_color = al_map_rgb(100, 100, 100); // Cinza
    
    for (int y = 0; y < m->height; y++) {
        for (int x = 0; x < m->width; x++) {
            
            if (m->tiles[y * m->width + x] == 1) {
                
                // Posição de desenho
                float dx = x * m->tile_size - m->camera_x;
                float dy = y * m->tile_size - m->camera_y;
                
                if (m->tile_image) {
                    // Desenha a IMAGEM do bloco (grama)
                    al_draw_bitmap(m->tile_image, dx, dy, 0);
                } else {
                    // Backup: Retângulo cinza (apenas se a imagem falhar)
                    al_draw_filled_rectangle(dx, dy, 
                                             dx + m->tile_size, dy + m->tile_size, 
                                             floor_color);
                }
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