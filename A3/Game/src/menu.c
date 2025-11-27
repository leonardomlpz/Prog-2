#include "menu.h"
#include "game.h"
#include <stdio.h>
#include <stdlib.h>

// Definições do menu
#define MENU_FONT_SIZE 64
#define FONT_PATH "assets/Fonte.ttf"

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
    m->btn_restart = al_load_bitmap("assets/Buttons/Restart.png");
    m->btn_back = al_load_bitmap("assets/Buttons/Back.png");
    m->btn_settings = al_load_bitmap("assets/Buttons/Settings.png");

    m->selected_option = 0; // Começa em "Iniciar"
    printf("Menu criado!\n");
    return m;
}

void menu_destroy(Menu* m) {
    if (m) {
        al_destroy_font(m->font);

        if (m->btn_restart) al_destroy_bitmap(m->btn_restart);
        if (m->btn_back) al_destroy_bitmap(m->btn_back);
        if (m->btn_play) al_destroy_bitmap(m->btn_play);
        if (m->btn_close) al_destroy_bitmap(m->btn_close);
        if (m->btn_settings) al_destroy_bitmap(m->btn_settings);

        free(m);
        printf("Menu destruído.\n");
    }
}

// Tratar Eventos (Input: Teclado e Mouse)
void menu_handle_event(Menu* m, ALLEGRO_EVENT* event, GameState* state, bool* done) {
    // Configuração igual à do draw para calcular colisão
    float scale = 5.0;
    int w_play = al_get_bitmap_width(m->btn_play);
    int h_play = al_get_bitmap_height(m->btn_play);
    int w_close = al_get_bitmap_width(m->btn_close);
    int h_close = al_get_bitmap_height(m->btn_close);

    // Posições (Devem bater com o menu_draw)
    float x_play = (SCREEN_WIDTH / 2) - ((w_play * scale) / 2);
    float y_play = 300; // Posição Y fixa definida no draw

    float x_close = (SCREEN_WIDTH / 2) - ((w_close * scale) / 2);
    float y_close = y_play + (h_play * scale) + 20;

    // TECLADO
    if (event->type == ALLEGRO_EVENT_KEY_DOWN) {
        switch (event->keyboard.keycode) {
            case ALLEGRO_KEY_W: 
            case ALLEGRO_KEY_UP:
                m->selected_option--;
                if (m->selected_option < 0) m->selected_option = 1;
                break;
            
            case ALLEGRO_KEY_S: 
            case ALLEGRO_KEY_DOWN:
                m->selected_option++;
                if (m->selected_option > 1) m->selected_option = 0;
                break;

            case ALLEGRO_KEY_ENTER:
            case ALLEGRO_KEY_SPACE:
                if (m->selected_option == 0) *state = STATE_GAMEPLAY;
                else if (m->selected_option == 1) *done = true;
                break;
        }
    }
    
    // MOUSE
    else if (event->type == ALLEGRO_EVENT_MOUSE_AXES) {
        int mx = event->mouse.x;
        int my = event->mouse.y;

        // Checa se mouse está sobre o botão Play
        if (mx >= x_play && mx <= x_play + (w_play * scale) &&
            my >= y_play && my <= y_play + (h_play * scale)) {
            m->selected_option = 0;
        }
        // Checa se mouse está sobre o botão Close
        else if (mx >= x_close && mx <= x_close + (w_close * scale) &&
                 my >= y_close && my <= y_close + (h_close * scale)) {
            m->selected_option = 1;
        }
    }

    // MOUSE (Clique)
    else if (event->type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
        if (event->mouse.button == 1) { // Botão esquerdo
            // Apenas confirma a opção que já foi selecionada pelo Hover
            int mx = event->mouse.x;
            int my = event->mouse.y;

            // Verifica clique no Play
            if (mx >= x_play && mx <= x_play + (w_play * scale) &&
                my >= y_play && my <= y_play + (h_play * scale)) {
                *state = STATE_GAMEPLAY;
            }
            // Verifica clique no Close
            else if (mx >= x_close && mx <= x_close + (w_close * scale) &&
                     my >= y_close && my <= y_close + (h_close * scale)) {
                *done = true;
            }
        }
    }
}

void menu_draw(Menu* m) {
    // Limpa a tela
    al_clear_to_color(al_map_rgb(20, 20, 30));

    // Título com Fonte
    al_draw_text(m->font, al_map_rgb(255, 255, 255), 
                 SCREEN_WIDTH/2, 100, ALLEGRO_ALIGN_CENTER, "NINJA FROG ADVENTURE");

    // Configuração Geral
    float scale = 5.0; 
    float start_y = 300;
    
    // Cores de Seleção
    ALLEGRO_COLOR c_sel = al_map_rgb(255, 255, 255); 
    ALLEGRO_COLOR c_unsel = al_map_rgb(100, 100, 100); 

    // Botão PLAY
    int w_play = al_get_bitmap_width(m->btn_play);
    int h_play = al_get_bitmap_height(m->btn_play);
    
    // Calcula o X centralizado para o tamanho do PLAY
    float cx_play = (SCREEN_WIDTH / 2) - ((w_play * scale) / 2);

    al_draw_tinted_scaled_bitmap(
        m->btn_play,
        (m->selected_option == 0) ? c_sel : c_unsel,
        0, 0, w_play, h_play,
        cx_play, start_y,
        w_play * scale, h_play * scale,
        0
    );

    // Botão CLOSE
    int w_close = al_get_bitmap_width(m->btn_close);
    int h_close = al_get_bitmap_height(m->btn_close);

    // Calcula o X centralizado para o tamanho do CLOSE (Essa é a correção!)
    float cx_close = (SCREEN_WIDTH / 2) - ((w_close * scale) / 2);
    
    // Calcula o Y logo abaixo do Play
    float cy_close = start_y + (h_play * scale) + 20;

    al_draw_tinted_scaled_bitmap(
        m->btn_close,
        (m->selected_option == 1) ? c_sel : c_unsel,
        0, 0, w_close, h_close,
        cx_close, cy_close,
        w_close * scale, h_close * scale,
        0
    );
}