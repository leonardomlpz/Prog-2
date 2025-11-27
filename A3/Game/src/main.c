#include <stdio.h>
#include <allegro5/allegro5.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>

#include "game.h"
#include "player.h"
#include "menu.h"
#include "world.h"
#include "obstacle.h"
#include "enemy.h"

// Funcao de verificacao de erro
void must_init(bool test, const char *description)
{
    if(test) return;
    printf("Nao foi possivel inicializar %s\n", description);
    exit(1);
}

// Gerenciamento de Estados
GameState state_stack[5];
int stack_top = -1;

void push_state(GameState state) {
    if (stack_top < 4) {
        stack_top++;
        state_stack[stack_top] = state;
    }
}

void pop_state() {
    if (stack_top > 0) { // Nunca removemos o último estado (Menu)
        stack_top--;
    }
}

void change_state(GameState state) {
    if (stack_top >= 0) {
        state_stack[stack_top] = state;
    } else {
        push_state(state);
    }
}

GameState peek_state() {
    return state_stack[stack_top];
}

int main()
{
    // --- Inicializacao ---
    must_init(al_init(), "allegro");
    must_init(al_install_keyboard(), "teclado");
    must_init(al_install_mouse(), "mouse");
    
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / FPS);
    must_init(timer, "timer");

    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    must_init(queue, "fila de eventos");

    al_set_new_display_flags(ALLEGRO_WINDOWED);
    ALLEGRO_DISPLAY* disp = al_create_display(SCREEN_WIDTH, SCREEN_HEIGHT);
    must_init(disp, "display");

    must_init(al_init_primitives_addon(), "primitivas");
    must_init(al_init_image_addon(), "imagens");
    must_init(al_init_font_addon(), "fontes");
    must_init(al_init_ttf_addon(), "ttf");

    // Registra as fontes de eventos
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_display_event_source(disp));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_mouse_event_source());

    // Variaveis de Jogo
    bool done = false;
    bool redraw = true;

    push_state(STATE_MENU);

    Player *player = player_create(32, 400);
    Menu *menu = menu_create();
    World *world = world_create();
    Obstacle *spike_head = obstacle_create(SPIKE_HEAD_VERTICAL, 483, 128);
    Obstacle *rock_head_1 = obstacle_create(ROCK_HEAD_RECTANGULAR, 1232, 32);
    Obstacle *rock_head_2 = obstacle_create(ROCK_HEAD_RECTANGULAR, 672, 416);
    Obstacle *saw = obstacle_create(SAW_HORIZONTAL, 890, 292);
    Enemy *rhino = enemy_create(ENEMY_RHINO, 400, 400);
    Enemy *bird = enemy_create(ENEMY_BLUEBIRD, 992, 208);
    Enemy *mushroom = enemy_create(ENEMY_MUSHROOM, 208, 240);
    Obstacle *trophy = obstacle_create(GOAL_TROPHY, 758,346);
    
    // Começa descendo
    rock_head_1->vel_y = 10.0;
    rock_head_1->vel_x = 0.0;
    // Começa subindo
    rock_head_2->vel_y = -10.0;
    rock_head_2->vel_x = 0.0;

    ALLEGRO_EVENT event;
    al_start_timer(timer);

    // Game Loop Principal
    while(!done)
    {
        al_wait_for_event(queue, &event);

        // Logica de Eventos
        if(event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            done = true;
        }

        GameState current_state = peek_state();

        // Eventos especificos de cada estado
        switch(current_state)
        {
            case STATE_MENU:
                GameState temp_state = current_state;
                menu_handle_event(menu, &event, &temp_state, &done);
                if (temp_state != current_state) {
                    change_state(temp_state);
                }
                if(event.type == ALLEGRO_EVENT_TIMER) {
                    redraw = true;
                }
                break;
            
            case STATE_GAMEPLAY:
                player_handle_event(player, &event);

                if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
                    if (event.keyboard.keycode == ALLEGRO_KEY_P || 
                        event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                        push_state(STATE_PAUSE);
                        menu->selected_option = 0; // Reseta selecao ao pausar
                    }
                }

                if(event.type == ALLEGRO_EVENT_TIMER) {
                    player_update(player, world);
                    world_update(world, player);
                    obstacle_update(spike_head, player, world);
                    obstacle_update(rock_head_1, player, world);
                    obstacle_update(rock_head_2, player, world);
                    obstacle_update(saw, player, world);
                    obstacle_update(trophy, player, world);
                    enemy_update(rhino, player, world);
                    enemy_update(bird, player, world);
                    enemy_update(mushroom, player, world);

                    if (player->hp <= 0) {
                        change_state(STATE_GAME_OVER_LOSE);
                        menu->selected_option = 0; // Reseta selecao
                    }
                    else if (player->level_complete) {
                        change_state(STATE_GAME_OVER_WIN);
                        menu->selected_option = 0; // Reseta selecao
                    }

                    redraw = true;
                }
                break;

            case STATE_PAUSE:
                {
                    float p_scale = 4.0;
                    int btn_w = al_get_bitmap_width(menu->btn_play);
                    int btn_h = al_get_bitmap_height(menu->btn_play);
                    int btn_x = (SCREEN_WIDTH / 2) - ((btn_w * p_scale) / 2);

                    // --- TECLADO PAUSE ---
                    if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
                        // Navegação
                        if (event.keyboard.keycode == ALLEGRO_KEY_W || event.keyboard.keycode == ALLEGRO_KEY_UP) {
                            menu->selected_option--;
                            if (menu->selected_option < 0) menu->selected_option = 2;
                        }
                        if (event.keyboard.keycode == ALLEGRO_KEY_S || event.keyboard.keycode == ALLEGRO_KEY_DOWN) {
                            menu->selected_option++;
                            if (menu->selected_option > 2) menu->selected_option = 0;
                        }
                        // Ação (Enter)
                        if (event.keyboard.keycode == ALLEGRO_KEY_ENTER || event.keyboard.keycode == ALLEGRO_KEY_SPACE) {
                            if (menu->selected_option == 0) { // Continuar
                                pop_state();
                            } else if (menu->selected_option == 1) { // Reiniciar
                                player_reset(player);
                                enemy_reset(rhino);
                                enemy_reset(bird);
                                enemy_reset(mushroom);
                                pop_state();
                            } else if (menu->selected_option == 2) { // Menu
                                player_reset(player);
                                stack_top = -1;
                                push_state(STATE_MENU);
                            }
                        }
                        // Tecla P ou ESC para despausar (atalho rápido)
                        if (event.keyboard.keycode == ALLEGRO_KEY_P || event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                            pop_state(); 
                        }
                    }

                    // MOUSE PAUSE
                    if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
                        int mx = event.mouse.x;
                        int my = event.mouse.y;

                        // Botão 0: Continuar
                        if (mx >= btn_x && mx <= btn_x + (btn_w * p_scale) && my >= 250 && my <= 250 + (btn_h * p_scale)) {
                            menu->selected_option = 0;
                        }
                        // Botão 1: Reiniciar
                        else if (mx >= btn_x && mx <= btn_x + (btn_w * p_scale) && my >= 350 && my <= 350 + (btn_h * p_scale)) {
                            menu->selected_option = 1;
                        }
                        // Botão 2: Menu
                        else if (mx >= btn_x && mx <= btn_x + (btn_w * p_scale) && my >= 450 && my <= 450 + (btn_h * p_scale)) {
                            menu->selected_option = 2;
                        }
                    }

                    // MOUSE PAUSE (CLIQUE)
                    if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
                        if (event.mouse.button == 1) {
                            // Executa a opção que está selecionada (se o mouse estiver em cima, o hover já selecionou)
                            int mx = event.mouse.x;
                            int my = event.mouse.y;
                            
                            // Verifica se clicou na area valida de algum botao
                            bool clicked = false;
                            if (mx >= btn_x && mx <= btn_x + (btn_w * p_scale)) {
                                if (my >= 250 && my <= 250 + (btn_h * p_scale)) clicked = true;
                                if (my >= 350 && my <= 350 + (btn_h * p_scale)) clicked = true;
                                if (my >= 450 && my <= 450 + (btn_h * p_scale)) clicked = true;
                            }

                            if (clicked) {
                                if (menu->selected_option == 0) { // Continuar
                                    pop_state();
                                } else if (menu->selected_option == 1) { // Reiniciar
                                    player_reset(player);
                                    enemy_reset(rhino);
                                    enemy_reset(bird);
                                    enemy_reset(mushroom);
                                    pop_state();
                                } else if (menu->selected_option == 2) { // Menu
                                    player_reset(player);
                                    stack_top = -1;
                                    push_state(STATE_MENU);
                                }
                            }
                        }
                    }
                    
                    if(event.type == ALLEGRO_EVENT_TIMER) redraw = true;
                }
                break;

            case STATE_GAME_OVER_WIN:
            case STATE_GAME_OVER_LOSE:
                {
                    float scale = 4.0;
                    int btn_w = al_get_bitmap_width(menu->btn_restart); // Restart e Back tem mesmo tamanho geralmente
                    int btn_h = al_get_bitmap_height(menu->btn_restart);
                    int btn_x_center = (SCREEN_WIDTH / 2) - ((btn_w * scale) / 2);

                    // TECLADO GAME OVER
                    if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
                        // Navegação
                        if (event.keyboard.keycode == ALLEGRO_KEY_W || event.keyboard.keycode == ALLEGRO_KEY_UP) {
                            menu->selected_option--;
                            if (menu->selected_option < 0) menu->selected_option = 1;
                        }
                        if (event.keyboard.keycode == ALLEGRO_KEY_S || event.keyboard.keycode == ALLEGRO_KEY_DOWN) {
                            menu->selected_option++;
                            if (menu->selected_option > 1) menu->selected_option = 0;
                        }
                        // Ação
                        if (event.keyboard.keycode == ALLEGRO_KEY_ENTER || event.keyboard.keycode == ALLEGRO_KEY_SPACE) {
                            if (menu->selected_option == 0) { // Restart
                                player_reset(player);
                                enemy_reset(rhino);
                                enemy_reset(bird);
                                enemy_reset(mushroom);
                                stack_top = -1; 
                                push_state(STATE_GAMEPLAY);
                            } else if (menu->selected_option == 1) { // Back/Menu
                                player_reset(player);
                                stack_top = -1;
                                push_state(STATE_MENU);
                            }
                        }
                    }

                    // MOUSE GAME OVER
                    if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
                        int mx = event.mouse.x;
                        int my = event.mouse.y;

                        // Botão 0: Restart
                        if (mx >= btn_x_center && mx <= btn_x_center + (btn_w * scale) && my >= 300 && my <= 300 + (btn_h * scale)) {
                            menu->selected_option = 0;
                        }
                        // Botão 1: Back
                        else if (mx >= btn_x_center && mx <= btn_x_center + (btn_w * scale) && my >= 450 && my <= 450 + (btn_h * scale)) {
                            menu->selected_option = 1;
                        }
                    }

                    // MOUSE GAME OVER (CLIQUE)
                    if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
                         if (event.mouse.button == 1) {
                            int mx = event.mouse.x;
                            int my = event.mouse.y;
                            bool clicked = false;

                            if (mx >= btn_x_center && mx <= btn_x_center + (btn_w * scale)) {
                                if (my >= 300 && my <= 300 + (btn_h * scale)) clicked = true;
                                if (my >= 450 && my <= 450 + (btn_h * scale)) clicked = true;
                            }

                            if (clicked) {
                                if (menu->selected_option == 0) { // Restart
                                    player_reset(player);
                                    enemy_reset(rhino);
                                    enemy_reset(bird);
                                    enemy_reset(mushroom);
                                    stack_top = -1; 
                                    push_state(STATE_GAMEPLAY);
                                } else if (menu->selected_option == 1) { // Back
                                    player_reset(player);
                                    stack_top = -1;
                                    push_state(STATE_MENU);
                                }
                            }
                         }
                    }
                    
                    if(event.type == ALLEGRO_EVENT_TIMER) redraw = true;
                }
                break;
        }

        // Logica de Desenho
        if(redraw && al_is_event_queue_empty(queue))
        {
            al_clear_to_color(al_map_rgb(0, 0, 0));

            // Reseta qualquer zoom anterior para garantir que o clear e o menu fiquem certos
            ALLEGRO_TRANSFORM t;
            al_identity_transform(&t);
            al_use_transform(&t);

            // Desenha a tela correta dependendo do estado
            switch(current_state)
            {
                case STATE_MENU:
                    menu_draw(menu);
                    break;
                
                case STATE_GAMEPLAY:
                    // Inicia o Zoom
                    ALLEGRO_TRANSFORM transform;
                    al_identity_transform(&transform);
                    al_scale_transform(&transform, GAME_SCALE, GAME_SCALE);
                    al_use_transform(&transform);
                
                    world_draw(world);

                    obstacle_draw(spike_head, world);
                    obstacle_draw(rock_head_1, world);
                    obstacle_draw(rock_head_2, world);
                    obstacle_draw(saw, world);
                    obstacle_draw(trophy, world);

                    enemy_draw(rhino, world);
                    enemy_draw(bird, world);
                    enemy_draw(mushroom, world);

                    player_draw(player, world);

                    // ZONA SEM ZOOM
                    al_use_transform(&t);

                    if (player->heart_icon) {
                        int heart_w = al_get_bitmap_width(player->heart_icon);
                        int heart_h = al_get_bitmap_height(player->heart_icon);
                        
                        // CONFIGURAÇÃO DE TAMANHO
                        float hud_scale = 2.0;
                        int padding = 10;      // Espaço entre os corações

                        // Desenha os corações
                        for (int i = 0; i < player->hp; i++) {
                            al_draw_scaled_bitmap(
                                player->heart_icon,
                                0, 0, heart_w, heart_h, // Origem (pega a imagem inteira)
                                10 + (i * (heart_w * hud_scale + padding)), // Posição X na tela
                                10,                                         // Posição Y na tela
                                heart_w * hud_scale, // Largura Final (Aumentada)
                                heart_h * hud_scale, // Altura Final (Aumentada)
                                0
                            );
                        }
                    }

                    break;

                case STATE_PAUSE:
                    // ZONA DO ZOOM (Desenha o jogo congelado no fundo)
                    {
                        ALLEGRO_TRANSFORM transform;
                        al_identity_transform(&transform);
                        al_scale_transform(&transform, GAME_SCALE, GAME_SCALE);
                        al_use_transform(&transform);
                        
                        world_draw(world);
                        player_draw(player, world);
                        obstacle_draw(spike_head, world);
                    }

                    // ZONA SEM ZOOM (Interface e Botões)
                    al_use_transform(&t); // Reseta para 1:1 (Tela cheia 800x600)

                    // Tela Escura
                    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
                    al_draw_filled_rectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, al_map_rgba_f(0, 0, 0, 0.6));
                    al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);

                    // Título "PAUSADO"
                    al_draw_text(menu->font, al_map_rgb(255, 255, 255), 
                                 SCREEN_WIDTH / 2, 100, 
                                 ALLEGRO_ALIGN_CENTER, "PAUSADO");

                    // DESENHO DOS BOTÕES
                    float p_scale = 4.0;
                    
                    // Cores de Seleção
                    ALLEGRO_COLOR c_sel = al_map_rgb(255, 255, 255); // Brilhante
                    ALLEGRO_COLOR c_unsel = al_map_rgb(100, 100, 100); // Escuro

                    // Botão CONTINUAR (Usando o Play)
                    if (menu->btn_play) {
                        int w = al_get_bitmap_width(menu->btn_play);
                        int h = al_get_bitmap_height(menu->btn_play);
                        al_draw_tinted_scaled_bitmap(menu->btn_play, 
                            (menu->selected_option == 0) ? c_sel : c_unsel,
                            0, 0, w, h,
                            (SCREEN_WIDTH/2) - (w*p_scale)/2, 250,
                            w*p_scale, h*p_scale, 0);
                    }

                    // Botão REINICIAR
                    if (menu->btn_restart) {
                        int w = al_get_bitmap_width(menu->btn_restart);
                        int h = al_get_bitmap_height(menu->btn_restart);
                        al_draw_tinted_scaled_bitmap(menu->btn_restart, 
                            (menu->selected_option == 1) ? c_sel : c_unsel,
                            0, 0, w, h,
                            (SCREEN_WIDTH/2) - (w*p_scale)/2, 350,
                            w*p_scale, h*p_scale, 0);
                    }

                    // Botão VOLTAR AO MENU
                    if (menu->btn_back) {
                        int w = al_get_bitmap_width(menu->btn_back);
                        int h = al_get_bitmap_height(menu->btn_back);
                        al_draw_tinted_scaled_bitmap(menu->btn_back, 
                            (menu->selected_option == 2) ? c_sel : c_unsel,
                            0, 0, w, h,
                            (SCREEN_WIDTH/2) - (w*p_scale)/2, 450,
                            w*p_scale, h*p_scale, 0);
                    }
                    break;
                case STATE_GAME_OVER_WIN:
                case STATE_GAME_OVER_LOSE:
                    // Desenha o jogo congelado no fundo (COM ZOOM)
                    {
                        ALLEGRO_TRANSFORM transform;
                        al_identity_transform(&transform);
                        al_scale_transform(&transform, GAME_SCALE, GAME_SCALE);
                        al_use_transform(&transform);

                        world_draw(world);
                        player_draw(player, world);
                    }

                    // Desenha a Interface (SEM ZOOM / 1:1)
                    al_use_transform(&t); // Reseta

                    // Tela Escura
                    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
                    al_draw_filled_rectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, al_map_rgba_f(0, 0, 0, 0.7));
                    al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);

                    // Título
                    if (current_state == STATE_GAME_OVER_WIN) {
                        al_draw_text(menu->font, al_map_rgb(255, 255, 0), 
                            SCREEN_WIDTH/2, 150, ALLEGRO_ALIGN_CENTER, "F A S E   C O M P L E T A !");
                    } else {
                        al_draw_text(menu->font, al_map_rgb(255, 0, 0), 
                            SCREEN_WIDTH/2, 150, ALLEGRO_ALIGN_CENTER, "G A M E  O V E R");
                    }

                    // Botões (Centralizados)
                    float scale = 4.0;
                    
                    // Cores de Seleção
                    ALLEGRO_COLOR c_sel_go = al_map_rgb(255, 255, 255); // Brilhante
                    ALLEGRO_COLOR c_unsel_go = al_map_rgb(100, 100, 100); // Escuro

                    // RESTART (Opção 0)
                    if (menu->btn_restart) {
                        int w = al_get_bitmap_width(menu->btn_restart);
                        int h = al_get_bitmap_height(menu->btn_restart);
                        al_draw_tinted_scaled_bitmap(menu->btn_restart, 
                            (menu->selected_option == 0) ? c_sel_go : c_unsel_go,
                            0, 0, w, h,
                            (SCREEN_WIDTH/2) - (w*scale)/2, 300, // X, Y
                            w*scale, h*scale, 0);
                    }

                    // BACK/MENU (Opção 1)
                    if (menu->btn_back) {
                        int w = al_get_bitmap_width(menu->btn_back);
                        int h = al_get_bitmap_height(menu->btn_back);
                        al_draw_tinted_scaled_bitmap(menu->btn_back, 
                            (menu->selected_option == 1) ? c_sel_go : c_unsel_go,
                            0, 0, w, h,
                            (SCREEN_WIDTH/2) - (w*scale)/2, 450, // X, Y
                            w*scale, h*scale, 0);
                    }
                    break;
            }

            al_flip_display();
            redraw = false;
        }
    }

    // Finalizacao
    player_destroy(player);
    menu_destroy(menu);
    al_destroy_timer(timer);
    world_destroy(world);
    obstacle_destroy(spike_head);
    obstacle_destroy(rock_head_1);
    obstacle_destroy(rock_head_2);
    obstacle_destroy(saw);
    obstacle_destroy(trophy);
    enemy_destroy(rhino);
    enemy_destroy(bird);
    enemy_destroy(mushroom);
    al_destroy_event_queue(queue);
    al_destroy_display(disp);

    return 0;
}