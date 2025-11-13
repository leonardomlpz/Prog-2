#ifndef GAME_H
#define GAME_H

// Dimensoes da tela
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define FPS 60.0

#define GAME_SCALE 3.0

// Nossa máquina de estados do jogo
typedef enum {
    STATE_MENU,
    STATE_GAMEPLAY,
    STATE_PAUSE,
    STATE_GAME_OVER_WIN,
    STATE_GAME_OVER_LOSE
} GameState;

#endif