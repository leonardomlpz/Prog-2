#ifndef MENU_H
#define MENU_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include "game.h" // Precisamos disso para o "GameState"

typedef struct {
    ALLEGRO_FONT* font;      // A fonte que vamos carregar
    int selected_option; // 0 = Iniciar, 1 = Sair

    ALLEGRO_BITMAP *btn_play;
    ALLEGRO_BITMAP *btn_close;
    ALLEGRO_BITMAP *btn_restart;
    ALLEGRO_BITMAP *btn_back;
    ALLEGRO_BITMAP *btn_settings;
} Menu;

// --- Protótipos das Funções ---

// Carrega a fonte
Menu* menu_create();

// Libera a fonte
void menu_destroy(Menu* m);

// Trata eventos (Cima, Baixo, Enter)
void menu_handle_event(Menu* m, ALLEGRO_EVENT* event, GameState* state, bool* done);

// Desenha o texto do menu na tela
void menu_draw(Menu* m);


#endif // MENU_H