#include "../include/game.h"
#include "../include/entities.h"
#include "../include/resources.h"
#include <iostream>
#include <time.h>

char MAP[26][26] = {
    "1111111111111111111111111",
    "1222222222222222222222221",
    "1211211121111111211121121",
    "1211211122222222211121121",
    "1222222221111111222222221",
    "1121111121111111211111211",
    "1222111122221222211112221",
    "1212111111121211111112121",
    "1212222111221221112222121",
    "1211112111211121112111121",
    "1222222100000000012222221",
    "1111112201110111022111111",
    "0000002101000001012000000",
    "1211112201000001022111121",
    "1211111101111111011111121",
    "1211122200000000022211121",
    "1222221111112111111222221",
    "1111121112222222111211111",
    "1111121112111112111211111",
    "1222222222222222222222221",
    "1211111211111111121111121",
    "1222221211111111121222221",
    "1211121222211122221211121",
    "1222221111222221111222221",
    "1111111111111111111111111",
};

int cell_size = 20; //size of each cell
int bananas = 226;
int score = 0; //player score
int pacmon_i = 15, pacmon_j = 12;
int pacmon_x = pacmon_j * cell_size;
int pacmon_y = pacmon_i * cell_size;
int pacmon_direction = DIR_RIGHT;
int hunter_red_i = 12, hunter_red_j = 10;
int hunter_red_x = hunter_red_j * cell_size;
int hunter_red_y = hunter_red_i * cell_size;
int hunter_blue_i = 13, hunter_blue_j = 10;
int hunter_blue_x = hunter_blue_j * cell_size;
int hunter_blue_y = hunter_blue_i * cell_size;
int hunter_pink_i = 12, hunter_pink_j = 14;
int hunter_pink_x = hunter_pink_j * cell_size;
int hunter_pink_y = hunter_pink_i * cell_size;
int hunter_orange_i = 13, hunter_orange_j = 14;
int hunter_orange_x = hunter_orange_j * cell_size;
int hunter_orange_y = hunter_orange_i * cell_size;
int hunter_modes[4] = {MODE_SCATTER, MODE_SCATTER, MODE_SCATTER, MODE_SCATTER};
int mode_timer = 0;
int scatter_targets_x[4] = {3, 3, 23, 23};
int scatter_targets_y[4] = {3, 23, 3, 23};
bool key[4] = {false, false, false, false};
bool redraw = true;
bool exit_game = false;
bool victory = false;
bool defeat = false;
bool update_logic = false;
bool game_over = false;
int end_game_timer = 0;

void runGameLoop() {
    int game_time = 0;
    srand(time(nullptr));

    while (!exit_game) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_TIMER) {
            if (ev.timer.source == logic_timer && !game_over) {
                game_time++;
                update_logic = true;

                if (key[KEY_UP] && isValidMove(pacmon_i - 1, pacmon_j)) {
                    pacmon_i--;
                    pacmon_y = pacmon_i * cell_size;
                    pacmon_direction = DIR_UP;
                }
                else if (key[KEY_DOWN] && isValidMove(pacmon_i + 1, pacmon_j)) {
                    pacmon_i++;
                    pacmon_y = pacmon_i * cell_size;
                    pacmon_direction = DIR_DOWN;
                }
                else if (key[KEY_LEFT] && isValidMove(pacmon_i, pacmon_j - 1)) {
                    pacmon_j--;
                    pacmon_x = pacmon_j * cell_size;
                    pacmon_direction = DIR_LEFT;
                }
                else if (key[KEY_RIGHT] && isValidMove(pacmon_i, pacmon_j + 1)) {
                    pacmon_j++;
                    pacmon_x = pacmon_j * cell_size;
                    pacmon_direction = DIR_RIGHT;
                }

                // Teleport pacmon
                if (pacmon_i == 12 && pacmon_j >= 25) {
                    pacmon_j = 0;
                    pacmon_x = pacmon_j * cell_size;
                }
                else if (pacmon_i == 12 && pacmon_j <= -1) {
                    pacmon_j = 24;
                    pacmon_x = pacmon_j * cell_size;
                }

                if (update_logic) {
                    mode_timer++;
                    if (mode_timer % 150 == 0) {
                        for (int i = 0; i < 4; i++) {
                            hunter_modes[i] = (hunter_modes[i] + 1) % 3;
                        }
                    }

                    hunterMovement(MAP, hunter_blue_i, hunter_blue_j, hunter_blue_x, hunter_blue_y, 0);
                    if (game_time >= 30)
                        hunterMovement(MAP, hunter_orange_i, hunter_orange_j, hunter_orange_x, hunter_orange_y, 1);
                    if (game_time >= 50)
                        hunterMovement(MAP, hunter_pink_i, hunter_pink_j, hunter_pink_x, hunter_pink_y, 2);
                    if (game_time >= 70)
                        hunterMovement(MAP, hunter_red_i, hunter_red_j, hunter_red_x, hunter_red_y, 3);
                }

                if (MAP[pacmon_i][pacmon_j] == '2') {
                    MAP[pacmon_i][pacmon_j] = '0';
                    al_play_sample(banana_sound, 0.5, 0.0, 2.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
                    bananas--;
                    score++;
                }

                if (bananas == 0) {
                    victory = true;
                    game_over = true;
                    al_stop_samples();
                    al_destroy_audio_stream(music);
                    al_play_sample(victory_sound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
                }

                if (checkCollision()) {
                    defeat = true;
                    game_over = true;
                    al_stop_samples();
                    al_destroy_audio_stream(music);
                    music = nullptr;
                    al_play_sample(defeat_sound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
                }
            }
            else if (ev.timer.source == timer) {
                if (game_over) {
                    end_game_timer++;
                    if (end_game_timer >= END_GAME_DELAY) {
                        exit_game = true;
                    }
                }
                redraw = true;
            }
        }
        else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            exit_game = true;
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch (ev.keyboard.keycode) {
                case ALLEGRO_KEY_UP: key[KEY_UP] = true; break;
                case ALLEGRO_KEY_DOWN: key[KEY_DOWN] = true; break;
                case ALLEGRO_KEY_LEFT: key[KEY_LEFT] = true; break;
                case ALLEGRO_KEY_RIGHT: key[KEY_RIGHT] = true; break;
            }
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_UP) {
            switch (ev.keyboard.keycode) {
                case ALLEGRO_KEY_UP: key[KEY_UP] = false; break;
                case ALLEGRO_KEY_DOWN: key[KEY_DOWN] = false; break;
                case ALLEGRO_KEY_LEFT: key[KEY_LEFT] = false; break;
                case ALLEGRO_KEY_RIGHT: key[KEY_RIGHT] = false; break;
                case ALLEGRO_KEY_ESCAPE: exit_game = true; break;
            }
        }

        if (redraw && al_is_event_queue_empty(event_queue)) {
            redraw = false;
            al_clear_to_color(al_map_rgb(91, 141, 91));

            if (victory) {
                al_draw_bitmap(victory_bitmap, 0, 0, 0);
                al_draw_textf(font, al_map_rgb(255, 255, 255), SCREEN_W / 2 - 50, SCREEN_H - 50, 0, "Score: %d", score);
            }
            else if (defeat) {
                al_draw_bitmap(defeat_bitmap, 0, 0, 0);
                al_draw_textf(font, al_map_rgb(255, 255, 255), SCREEN_W / 2 - 50, SCREEN_H - 50, 0, "Score: %d", score);
            }
            else {
                al_draw_bitmap(map_bitmap, 0, 0, 0);
                al_draw_bitmap(pacmon_bitmap[pacmon_direction], pacmon_x, pacmon_y, 0);
                al_draw_bitmap(hunter_red, hunter_red_x, hunter_red_y, 0);
                al_draw_bitmap(hunter_blue, hunter_blue_x, hunter_blue_y, 0);
                al_draw_bitmap(hunter_pink, hunter_pink_x, hunter_pink_y, 0);
                al_draw_bitmap(hunter_orange, hunter_orange_x, hunter_orange_y, 0);

                for (int i = 1; i < 25; i++) {
                    for (int j = 1; j < 25; j++) {
                        if (MAP[i][j] == '2') {
                            al_draw_bitmap(banana_bitmap, j * cell_size, i * cell_size, 0);
                        }
                    }
                }

                al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 515, 0, "Score: %d", score);
            }

            al_flip_display();
        }
    }
}