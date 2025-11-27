#ifndef MENU_H
#define MENU_H

#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include "game.h"

typedef struct {
    ALLEGRO_FONT* font;
    int selected_option; // 0 = Iniciar, 1 = Sair

    ALLEGRO_BITMAP *btn_play;
    ALLEGRO_BITMAP *btn_close;
    ALLEGRO_BITMAP *btn_restart;
    ALLEGRO_BITMAP *btn_back;
    ALLEGRO_BITMAP *btn_settings;
} Menu;

Menu* menu_create();

void menu_destroy(Menu* m);

// Trata eventos (Cima, Baixo, Enter)
void menu_handle_event(Menu* m, ALLEGRO_EVENT* event, GameState* state, bool* done);

void menu_draw(Menu* m);


#endif