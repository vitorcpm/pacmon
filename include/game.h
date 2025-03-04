#ifndef GAME_H
#define GAME_H

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

const float FPS = 30;
const float LOGIC_FPS = 7.5;
const int SCREEN_W = 500;
const int SCREEN_H = 550;
const int HUNTER_SKIP_FRAMES = 10;
const int MODE_SCATTER = 0;
const int MODE_CHASE = 1;
const int MODE_RANDOM = 2;
const int END_GAME_DELAY = 180;

enum MYKEYS { KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT };
enum DIRECTION { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT, DIR_NONE };

extern char MAP[26][26];

extern ALLEGRO_DISPLAY *display;
extern ALLEGRO_EVENT_QUEUE *event_queue;
extern ALLEGRO_TIMER *timer;
extern ALLEGRO_TIMER *logic_timer;
extern ALLEGRO_BITMAP *map_bitmap;
extern ALLEGRO_BITMAP *pacmon_bitmap[4];
extern ALLEGRO_BITMAP *hunter_red;
extern ALLEGRO_BITMAP *hunter_blue;
extern ALLEGRO_BITMAP *hunter_pink;
extern ALLEGRO_BITMAP *hunter_orange;
extern ALLEGRO_BITMAP *banana_bitmap;
extern ALLEGRO_BITMAP *victory_bitmap;
extern ALLEGRO_BITMAP *defeat_bitmap;
extern ALLEGRO_FONT *font;
extern ALLEGRO_SAMPLE *banana_sound;
extern ALLEGRO_SAMPLE *victory_sound;
extern ALLEGRO_SAMPLE *defeat_sound;
extern ALLEGRO_AUDIO_STREAM *music;


extern int cell_size;
extern int bananas;
extern int score;
extern int pacmon_i, pacmon_j;
extern int pacmon_x, pacmon_y;
extern int pacmon_direction;
extern int hunter_red_i, hunter_red_j;
extern int hunter_red_x, hunter_red_y;
extern int hunter_blue_i, hunter_blue_j;
extern int hunter_blue_x, hunter_blue_y;
extern int hunter_pink_i, hunter_pink_j;
extern int hunter_pink_x, hunter_pink_y;
extern int hunter_orange_i, hunter_orange_j;
extern int hunter_orange_x, hunter_orange_y;
extern int hunter_modes[4];
extern int mode_timer;
extern int scatter_targets_x[4];
extern int scatter_targets_y[4];
extern bool key[4];
extern bool redraw;
extern bool exit_game;
extern bool victory;
extern bool defeat;
extern bool update_logic;
extern bool game_over;
extern int end_game_timer;

void runGameLoop();

#endif