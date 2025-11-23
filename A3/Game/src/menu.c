#include "menu.h"
#include "game.h" // Para SCREEN_WIDTH e SCREEN_HEIGHT
#include <stdio.h>
#include <stdlib.h>

// Definições do menu
#define MENU_FONT_SIZE 32
#define FONT_PATH "assets/Fonte.ttf"

// --- Criar Menu ---
Menu* menu_create() {
    Menu* m = (Menu*) malloc(sizeof(Menu));
    if (!m) {
        printf("Erro ao alocar menu\n");
        return NULL;
    }

    // Carrega a fonte TTF
    m->font = al_load_ttf_font(FONT_PATH, MENU_FONT_SIZE, 0);
    if (!m->font) {
        printf("Erro ao carregar fonte em %s\n", FONT_PATH);
        free(m);
        return NULL;
    }

    m->btn_play = al_load_bitmap("assets/Buttons/Play.png");
    m->btn_close = al_load_bitmap("assets/Buttons/Close.png");

    m->selected_option = 0; // Começa em "Iniciar"
    printf("Menu criado!\n");
    return m;
}

// --- Destruir Menu ---
void menu_destroy(Menu* m) {
    if (m) {
        al_destroy_font(m->font);
        free(m);
        printf("Menu destruído.\n");
    }
}

// --- Tratar Eventos (Input) ---
void menu_handle_event(Menu* m, ALLEGRO_EVENT* event, GameState* state, bool* done) {
    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        switch (event->keyboard.keycode) {
            case ALLEGRO_KEY_W: // Mover para cima
            case ALLEGRO_KEY_UP:
                m->selected_option--;
                if (m->selected_option < 0) {
                    m->selected_option = 1; // Volta para "Sair"
                }
                break;
            
            case ALLEGRO_KEY_S: // Mover para baixo
            case ALLEGRO_KEY_DOWN:
                m->selected_option++;
                if (m->selected_option > 1) {
                    m->selected_option = 0; // Volta para "Iniciar"
                }
                break;

            case ALLEGRO_KEY_ENTER:
            case ALLEGRO_KEY_SPACE:
                if (m->selected_option == 0) { // "Iniciar"
                    *state = STATE_GAMEPLAY;
                } else if (m->selected_option == 1) { // "Sair"
                    *done = true;
                }
                break;
        }
    }
}

void menu_draw(Menu* m) {
    // Limpa a tela (opcional, pois main já limpa)
    al_clear_to_color(al_map_rgb(20, 20, 30));

    // Título com Fonte
    al_draw_text(m->font, al_map_rgb(255, 255, 255), 
                 SCREEN_WIDTH/2, 100, ALLEGRO_ALIGN_CENTER, "NINJA FROG ADVENTURE");

    // Configuração dos Botões
    float scale = 5.0; // Botões grandes (pixel art)
    int w = al_get_bitmap_width(m->btn_play);
    int h = al_get_bitmap_height(m->btn_play);
    
    float cx = SCREEN_WIDTH / 2 - (w * scale) / 2;
    float cy = 300;
    
    // Cores de Seleção
    ALLEGRO_COLOR c_sel = al_map_rgb(255, 255, 255); // Normal (Brilhante)
    ALLEGRO_COLOR c_unsel = al_map_rgb(100, 100, 100); // Escuro

    // Botão PLAY (Opção 0)
    al_draw_tinted_scaled_bitmap(
        m->btn_play,
        (m->selected_option == 0) ? c_sel : c_unsel,
        0, 0, w, h,
        cx, cy,
        w * scale, h * scale,
        0
    );

    // Botão CLOSE (Opção 1)
    al_draw_tinted_scaled_bitmap(
        m->btn_close,
        (m->selected_option == 1) ? c_sel : c_unsel,
        0, 0, w, h,
        cx, cy + (h * scale) + 20, // 20px abaixo
        w * scale, h * scale,
        0
    );
}