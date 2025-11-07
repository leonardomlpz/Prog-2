#include "menu.h"
#include "game.h" // Para SCREEN_WIDTH e SCREEN_HEIGHT
#include <stdio.h>
#include <stdlib.h>

// Definições do menu
#define MENU_FONT_SIZE 32
#define FONT_PATH "assets/Outwrite.ttf"

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

// --- Desenhar Menu ---
void menu_draw(Menu* m) {
    // Cores
    ALLEGRO_COLOR color_selected = al_map_rgb(255, 0, 0); // Vermelho
    ALLEGRO_COLOR color_default = al_map_rgb(255, 255, 255); // Branco

    ALLEGRO_COLOR color_iniciar;
    ALLEGRO_COLOR color_sair;

    // Define a cor baseada na seleção
    if (m->selected_option == 0) {
        color_iniciar = color_selected;
        color_sair = color_default;
    } else {
        color_iniciar = color_default;
        color_sair = color_selected;
    }

    // Desenha o título (opcional)
    al_draw_text(
        m->font, 
        color_default, 
        SCREEN_WIDTH / 2, // Posição X (centro)
        SCREEN_HEIGHT / 3, // Posição Y (um terço para baixo)
        ALLEGRO_ALIGN_CENTER, // Alinhamento
        "PLATAFORMA DE SOBREVIVENCIA"
    );

    // Desenha a opção "Iniciar"
    al_draw_text(
        m->font, 
        color_iniciar, 
        SCREEN_WIDTH / 2, 
        SCREEN_HEIGHT / 2, // Posição Y (meio)
        ALLEGRO_ALIGN_CENTER, 
        "Iniciar Jogo"
    );

    // Desenha a opção "Sair"
    al_draw_text(
        m->font, 
        color_sair, 
        SCREEN_WIDTH / 2, 
        (SCREEN_HEIGHT / 2) + MENU_FONT_SIZE + 10, // Posição Y (logo abaixo de "Iniciar")
        ALLEGRO_ALIGN_CENTER, 
        "Sair"
    );
}