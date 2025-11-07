#include <stdio.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>      // Para o menu e HUD
#include <allegro5/allegro_image.h>     // Para os sprites e background

// Nossos arquivos de cabecalho
#include "game.h"
#include "player.h"
#include "menu.h"

// Funcao de verificacao de erro
void must_init(bool test, const char *description)
{
    if(test) return;
    printf("Nao foi possivel inicializar %s\n", description);
    exit(1);
}

int main()
{
    // --- Inicializacao ---
    must_init(al_init(), "allegro");
    must_init(al_install_keyboard(), "teclado");
    
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / FPS);
    must_init(timer, "timer");

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    must_init(queue, "fila de eventos");

    al_set_new_display_flags(ALLEGRO_WINDOWED);
    ALLEGRO_DISPLAY* disp = al_create_display(SCREEN_WIDTH, SCREEN_HEIGHT);
    must_init(disp, "display");

    // --- Addons ---
    // O enunciado pede sprites, background de imagem e menu
    must_init(al_init_primitives_addon(), "primitivas");
    must_init(al_init_image_addon(), "imagens");
    must_init(al_init_font_addon(), "fontes"); // Precisaremos de al_init_ttf_addon() para fontes .ttf
    must_init(al_init_ttf_addon(), "ttf");

    // Registra as fontes de eventos
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    // --- Variaveis de Jogo ---
    bool done = false;
    bool redraw = true;
    GameState state = STATE_MENU; // O jogo comeca no menu
    Player *player = player_create(100, 100);
    Menu *menu = menu_create();

    // Nossos objetos de jogo serao criados aqui (ex: Player* player = player_create();)
    
    ALLEGRO_EVENT event;
    al_start_timer(timer);

    // --- Game Loop Principal ---
    // Este loop agora gerencia os diferentes estados do jogo
    while(!done)
    {
        al_wait_for_event(queue, &event);

        // --- Logica de Eventos ---
        // Eventos que sao tratados em qualquer estado (ex: fechar a janela)
        if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            done = true;
        }

        // Eventos especificos de cada estado
        switch(state)
        {
            case STATE_MENU:
                menu_handle_event(menu, &event, &state, &done);
                if(event.type == ALLEGRO_EVENT_TIMER) {
                    // (Aqui chamaremos menu_update())
                    redraw = true;
                }
                break;
            
            case STATE_GAMEPLAY:
                player_handle_event(player, &event);

                if(event.type == ALLEGRO_EVENT_TIMER) {
                    player_update(player);
                    redraw = true;
                }
                break;

            case STATE_GAME_OVER_WIN:
            case STATE_GAME_OVER_LOSE:
                // (Aqui chamaremos game_over_handle_event(&event, &state))
                if(event.type == ALLEGRO_EVENT_TIMER) {
                    redraw = true;
                }
                break;
        }

        // --- Logica de Desenho ---
        if(redraw && al_is_event_queue_empty(queue))
        {
            al_clear_to_color(al_map_rgb(0, 0, 0));

            // Desenha a tela correta dependendo do estado
            switch(state)
            {
                case STATE_MENU:
                    menu_draw(menu);
                    break;
                
                case STATE_GAMEPLAY:
                    player_draw(player);
                    break;

                case STATE_GAME_OVER_WIN:
                case STATE_GAME_OVER_LOSE:
                    // (Aqui chamaremos game_over_draw())
                    break;
            }

            al_flip_display();
            redraw = false;
        }
    }

    // --- Finalizacao ---
    player_destroy(player);
    menu_destroy(menu);
    al_destroy_display(disp);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    return 0;
}