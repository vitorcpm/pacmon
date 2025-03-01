#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <iostream>
#include <time.h>
#include <cmath>
#include <stdio.h>
#include <stdlib.h>
#include <cctype>

using namespace std;

const float FPS = 7.5;
const int SCREEN_W = 500;
const int SCREEN_H = 550;

enum MYKEYS {
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT
};

// Game map: 1 represents walls, 0 represents corridors, 2 represents bananas
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

ALLEGRO_DISPLAY *display = nullptr;
ALLEGRO_EVENT_QUEUE *event_queue = nullptr;
ALLEGRO_TIMER *timer = nullptr;
ALLEGRO_BITMAP *map_bitmap = nullptr;
ALLEGRO_BITMAP *pacmon_bitmap = nullptr;
ALLEGRO_BITMAP *hunter_red = nullptr;
ALLEGRO_BITMAP *hunter_blue = nullptr;
ALLEGRO_BITMAP *hunter_pink = nullptr;
ALLEGRO_BITMAP *hunter_orange = nullptr;
ALLEGRO_BITMAP *banana_bitmap = nullptr;
ALLEGRO_BITMAP *victory_bitmap = nullptr;
ALLEGRO_BITMAP *defeat_bitmap = nullptr;
ALLEGRO_FONT *font = nullptr;
ALLEGRO_SAMPLE *banana_sound = nullptr;
ALLEGRO_SAMPLE *victory_sound = nullptr;
ALLEGRO_SAMPLE *defeat_sound = nullptr;
ALLEGRO_AUDIO_STREAM *music = nullptr;

int cell_size = 20; // Size of each cell in the map
int bananas = 226;  // Total bananas
int score = 0;      // Player score

// pacmon position
int pacmon_i = 15, pacmon_j = 12;
int pacmon_y = pacmon_i * cell_size;
int pacmon_x = pacmon_j * cell_size;

// Hunters positions
int hunter_red_i = 12, hunter_red_j = 10;
int hunter_red_y = hunter_red_i * cell_size;
int hunter_red_x = hunter_red_j * cell_size;

int hunter_blue_i = 13, hunter_blue_j = 10;
int hunter_blue_y = hunter_blue_i * cell_size;
int hunter_blue_x = hunter_blue_j * cell_size;

int hunter_pink_i = 12, hunter_pink_j = 14;
int hunter_pink_y = hunter_pink_i * cell_size;
int hunter_pink_x = hunter_pink_j * cell_size;

int hunter_orange_i = 13, hunter_orange_j = 14;
int hunter_orange_y = hunter_orange_i * cell_size;
int hunter_orange_x = hunter_orange_j * cell_size;

bool key[4] = { false, false, false, false };
bool redraw = true;
bool exit_game = false;
bool victory = false;
bool defeat = false;

int last_random_pos = -1;
int random_chase = -1;

// Function to calculate distance between two points
double distance(int x1, int y1, int x2, int y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

// Structure for hunter movement
struct Chase {
    int x, y;
    char path;
    double dist;
};

// Function for red hunter movement
void hunterRedMovement(char map[][26], int &x, int &y, int &pos_x, int &pos_y) {
    // Red hunter does not enter the spawn zone after leaving it
    if ((x == 8 && y == 11 && pacmon_j <= y)) {
        y--;
        pos_y = x * cell_size;
        return;
    } else if ((x == 8 && y == 11 && pacmon_j >= y)) {
        y++;
        pos_y = x * cell_size;
        return;
    }

    // Calculate distances around the hunter
    Chase calc[4];
    calc[0].dist = distance(x + 1, y, pacmon_i, pacmon_j); calc[0].x = x + 1; calc[0].y = y; calc[0].path = 'S';
    calc[1].dist = distance(x, y - 1, pacmon_i, pacmon_j); calc[1].x = x; calc[1].y = y - 1; calc[1].path = 'A';
    calc[2].dist = distance(x - 1, y, pacmon_i, pacmon_j); calc[2].x = x - 1; calc[2].y = y; calc[2].path = 'W';
    calc[3].dist = distance(x, y + 1, pacmon_i, pacmon_j); calc[3].x = x; calc[3].y = y + 1; calc[3].path = 'D';

    int way = 1000;
    double min_dist = 1000;

    // Find the best path to chase pacmon
    for (int i = 0; i < 4; i++) {
        if (calc[i].dist < min_dist && map[calc[i].x][calc[i].y] != '1') {
            min_dist = calc[i].dist;
            way = i;
        }
    }

    if (way == 0) {
        x++;
        pos_y = x * cell_size;
    } else if (way == 2) {
        x--;
        pos_y = x * cell_size;
    } else if (way == 3) {
        y++;
        pos_x = y * cell_size;
    } else if (way == 1) {
        y--;
        pos_x = y * cell_size;
    }

    // Teleport hunter if it reaches the edge
    if (x == 10 && y == -1) {
        x = 10;
        y = 23;
        pos_y = x * cell_size;
        pos_x = y * cell_size;
    } else if (x == 10 && y == 22) {
        x = 10;
        y = -1;
        pos_y = x * cell_size;
        pos_x = y * cell_size;
    }
}

// Function for random hunter movement
void randomMovement(char map[][26], int &x, int &y, int &pos_x, int &pos_y, int hunter_type) {
    random_chase = rand() % 4;

    // Teleport hunter if it reaches the edge
    if (x == 12 && y == -1) {
        x = 12;
        y = 24;
        pos_x = y * cell_size;
        pos_y = x * cell_size;
        return;
    } else if (x == 12 && y == 25) {
        x = 12;
        y = 0;
        pos_x = y * cell_size;
        pos_y = x * cell_size;
        return;
    }

    if (random_chase == 0 && map[x - 1][y] != '1') {
        x--;
        pos_y = x * cell_size;
    } else if (random_chase == 1 && map[x + 1][y] != '1') {
        x++;
        pos_y = x * cell_size;
    } else if (random_chase == 2 && map[x][y + 1] != '1') {
        y++;
        pos_x = y * cell_size;
    } else if (random_chase == 3 && map[x][y - 1] != '1') {
        y--;
        pos_x = y * cell_size;
    }
}

int initialize() {
    srand(time(nullptr));

    if (!al_init()) {
        cout << "Failed to initialize Allegro!" << endl;
        return 0;
    }

    if (!al_install_keyboard()) {
        cout << "Failed to initialize the keyboard!" << endl;
        return 0;
    }

    timer = al_create_timer(1.0 / FPS);
    if (!timer) {
        cout << "Failed to create timer!" << endl;
        return 0;
    }

    if (!al_init_image_addon()) {
        cout << "Failed to initialize image addon!" << endl;
        return 0;
    }

    if (!al_init_font_addon()) {
        cout << "Failed to initialize font addon!" << endl;
        return 0;
    }

    if (!al_init_ttf_addon()) {
        cout << "Failed to initialize TTF addon!" << endl;
        return 0;
    }

    if (!al_install_audio()) {
        cout << "Failed to install audio!" << endl;
        return 0;
    }

    if (!al_init_acodec_addon()) {
        cout << "Failed to initialize audio codecs!" << endl;
        return 0;
    }

    if (!al_reserve_samples(2)) {
        cout << "Failed to reserve audio samples!" << endl;
        return 0;
    }

    display = al_create_display(SCREEN_W, SCREEN_H);
    if (!display) {
        cout << "Failed to create display!" << endl;
        al_destroy_timer(timer);
        return 0;
    }

    // Load map
    map_bitmap = al_load_bitmap("assets/map/map.bmp");
    if (!map_bitmap) {
        cout << "Failed to load map!" << endl;
        al_destroy_display(display);
        return 0;
    }

    // Load bananas
    banana_bitmap = al_load_bitmap("assets/map/banana.png");
    if (!banana_bitmap) {
        cout << "Failed to load bananas!" << endl;
        al_destroy_display(display);
        return 0;
    }

    // Load victory and defeat screens
    victory_bitmap = al_load_bitmap("assets/map/victorymap.bmp");
    if (!victory_bitmap) {
        cout << "Failed to load victory screen!" << endl;
        al_destroy_display(display);
        return 0;
    }

    defeat_bitmap = al_load_bitmap("assets/map/defeatmap.bmp");
    if (!defeat_bitmap) {
        cout << "Failed to load defeat screen!" << endl;
        al_destroy_display(display);
        return 0;
    }

    // Load pacmon
    pacmon_bitmap = al_load_bitmap("assets/characters/pacmon.png");
    if (!pacmon_bitmap) {
        cout << "Failed to load pacmon!" << endl;
        al_destroy_display(display);
        return 0;
    }

    // Load hunters
    hunter_red = al_load_bitmap("assets/characters/hunterr.png");
    if (!hunter_red) {
        cout << "Failed to load red hunter!" << endl;
        al_destroy_display(display);
        return 0;
    }

    hunter_blue = al_load_bitmap("assets/characters/hunterb.png");
    if (!hunter_blue) {
        cout << "Failed to load blue hunter!" << endl;
        al_destroy_display(display);
        return 0;
    }

    hunter_pink = al_load_bitmap("assets/characters/hunterp.png");
    if (!hunter_pink) {
        cout << "Failed to load pink hunter!" << endl;
        al_destroy_display(display);
        return 0;
    }

    hunter_orange = al_load_bitmap("assets/characters/huntero.png");
    if (!hunter_orange) {
        cout << "Failed to load orange hunter!" << endl;
        al_destroy_display(display);
        return 0;
    }

    // Load font
    font = al_load_font("assets/fonts/high_pixel-7.ttf", 25, 0);
    if (!font) {
        cout << "Failed to load font!" << endl;
        al_destroy_display(display);
        return 0;
    }

    // Load sounds
    banana_sound = al_load_sample("assets/sounds/bananacoincut.wav");
    if (!banana_sound) {
        cout << "Failed to load banana sound!" << endl;
        return 0;
    }

    victory_sound = al_load_sample("assets/sounds/victory.wav");
    if (!victory_sound) {
        cout << "Failed to load victory sound!" << endl;
        return 0;
    }

    defeat_sound = al_load_sample("assets/sounds/youfailed.wav");
    if (!defeat_sound) {
        cout << "Failed to load defeat sound!" << endl;
        return 0;
    }

    // Load music
    music = al_load_audio_stream("assets/sounds/junglemungle.wav", 4, 1024);
    if (!music) {
        cout << "Failed to load music!" << endl;
        return 0;
    }

    event_queue = al_create_event_queue();
    if (!event_queue) {
        cout << "Failed to create event queue!" << endl;
        al_destroy_display(display);
        al_destroy_timer(timer);
        return 0;
    }

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    al_clear_to_color(al_map_rgb(91, 141, 91));
    al_flip_display();
    al_start_timer(timer);

    return 1;
}

int main(int argc, char **argv) {
    int time = 0;

    if (!initialize()) return -1;

    al_attach_audio_stream_to_mixer(music, al_get_default_mixer());
    al_set_audio_stream_playing(music, true);

    while (!exit_game) {
        time++;

        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_TIMER) {
            if (key[KEY_UP] && MAP[pacmon_i - 1][pacmon_j] != '1') {
                pacmon_i--;
                pacmon_y = pacmon_i * cell_size;
            }

            if (key[KEY_DOWN] && MAP[pacmon_i + 1][pacmon_j] != '1') {
                pacmon_i++;
                pacmon_y = pacmon_i * cell_size;
            }

            if (key[KEY_LEFT] && MAP[pacmon_i][pacmon_j - 1] != '1') {
                pacmon_j--;
                pacmon_x = pacmon_j * cell_size;
            }

            if (key[KEY_RIGHT] && MAP[pacmon_i][pacmon_j + 1] != '1') {
                pacmon_j++;
                pacmon_x = pacmon_j * cell_size;
            }

            redraw = true;
        } else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            break;
        } else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch (ev.keyboard.keycode) {
                case ALLEGRO_KEY_UP:
                    key[KEY_UP] = true;
                    break;
                case ALLEGRO_KEY_DOWN:
                    key[KEY_DOWN] = true;
                    break;
                case ALLEGRO_KEY_LEFT:
                    key[KEY_LEFT] = true;
                    break;
                case ALLEGRO_KEY_RIGHT:
                    key[KEY_RIGHT] = true;
                    break;
            }
        } else if (ev.type == ALLEGRO_EVENT_KEY_UP) {
            switch (ev.keyboard.keycode) {
                case ALLEGRO_KEY_UP:
                    key[KEY_UP] = false;
                    break;
                case ALLEGRO_KEY_DOWN:
                    key[KEY_DOWN] = false;
                    break;
                case ALLEGRO_KEY_LEFT:
                    key[KEY_LEFT] = false;
                    break;
                case ALLEGRO_KEY_RIGHT:
                    key[KEY_RIGHT] = false;
                    break;
                case ALLEGRO_KEY_ESCAPE:
                    exit_game = true;
                    break;
            }
        }

        // Teleport pacmon
        if (pacmon_i == 12 && pacmon_j >= 25) {
            pacmon_i = 12;
            pacmon_j = 0;
            pacmon_y = pacmon_i * cell_size;
            pacmon_x = pacmon_j * cell_size;
        }

        if (pacmon_i == 12 && pacmon_j <= -1) {
            pacmon_i = 12;
            pacmon_j = 24;
            pacmon_y = pacmon_i * cell_size;
            pacmon_x = pacmon_j * cell_size;
        }

        // pacmon eats a banana
        if (MAP[pacmon_i][pacmon_j] == '2') {
            MAP[pacmon_i][pacmon_j] = '0';
            al_play_sample(banana_sound, 0.5, 0.0, 2.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
            bananas--;
            score++;
        }

        if (bananas == 0) {
            victory = true;
        }

        if (redraw && al_is_event_queue_empty(event_queue)) {
            redraw = false;

            al_clear_to_color(al_map_rgb(91, 141, 91));

            al_draw_bitmap(map_bitmap, 0, 0, 0);
            al_draw_bitmap(pacmon_bitmap, pacmon_x, pacmon_y, 0);
            al_draw_bitmap(hunter_red, hunter_red_x, hunter_red_y, 0);
            al_draw_bitmap(hunter_blue, hunter_blue_x, hunter_blue_y, 0);
            al_draw_bitmap(hunter_pink, hunter_pink_x, hunter_pink_y, 0);
            al_draw_bitmap(hunter_orange, hunter_orange_x, hunter_orange_y, 0);

            // Start hunters
            randomMovement(MAP, hunter_blue_i, hunter_blue_j, hunter_blue_x, hunter_blue_y, 0);

            if (time >= 30)
                randomMovement(MAP, hunter_orange_i, hunter_orange_j, hunter_orange_x, hunter_orange_y, 1);

            if (time >= 50)
                randomMovement(MAP, hunter_pink_i, hunter_pink_j, hunter_pink_x, hunter_pink_y, 2);

            if (time >= 70)
                hunterRedMovement(MAP, hunter_red_i, hunter_red_j, hunter_red_x, hunter_red_y);

            // Draw bananas
            for (int i = 1; i < 25; i++) {
                for (int j = 1; j < 25; j++) {
                    if (MAP[i][j] == '2') {
                        int banana_y = i * cell_size;
                        int banana_x = j * cell_size;
                        al_draw_bitmap(banana_bitmap, banana_x, banana_y, 0);
                    }
                }
            }

            al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 515, 0, "Score: %d", score);

            if (victory) {
                al_destroy_sample(banana_sound);
                al_destroy_audio_stream(music);
                al_play_sample(victory_sound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
                al_destroy_bitmap(map_bitmap);
                al_draw_bitmap(victory_bitmap, 0, 0, 0);
                al_rest(7.5);
                break;
            }

            // Check for defeat conditions
            if ((pacmon_i == hunter_red_i && pacmon_j == hunter_red_j) ||
                (pacmon_i == hunter_orange_i && pacmon_j == hunter_orange_j) ||
                (pacmon_i == hunter_blue_i && pacmon_j == hunter_blue_j) ||
                (pacmon_i == hunter_pink_i && pacmon_j == hunter_pink_j)) {
                defeat = true;
            }

            if (defeat) {
                al_destroy_sample(banana_sound);
                al_destroy_audio_stream(music);
                al_play_sample(defeat_sound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
                al_destroy_bitmap(map_bitmap);
                al_draw_bitmap(defeat_bitmap, 0, 0, 0);
                al_rest(7.5);
                break;
            }

            al_flip_display();
        }
    }

    // Clean up resources
    al_destroy_bitmap(map_bitmap);
    al_destroy_bitmap(pacmon_bitmap);
    al_destroy_bitmap(hunter_red);
    al_destroy_bitmap(hunter_blue);
    al_destroy_bitmap(hunter_pink);
    al_destroy_bitmap(hunter_orange);
    al_destroy_bitmap(banana_bitmap);
    al_destroy_bitmap(victory_bitmap);
    al_destroy_bitmap(defeat_bitmap);
    al_destroy_audio_stream(music);
    al_destroy_sample(victory_sound);
    al_destroy_sample(defeat_sound);
    al_destroy_font(font);
    al_destroy_timer(timer);
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);

    return 0;
}