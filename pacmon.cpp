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

const float FPS = 30;               // Higher FPS for smoother movement
const float LOGIC_FPS = 7.5;        // Keep game logic at original speed
const int SCREEN_W = 500;
const int SCREEN_H = 550;
const int HUNTER_SKIP_FRAMES = 10;  // Higher values make hunters slower

const int MODE_SCATTER = 0;
const int MODE_CHASE = 1;
const int MODE_RANDOM = 2;

enum MYKEYS {
    KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT
};

enum DIRECTION {
    DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT, DIR_NONE
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
ALLEGRO_TIMER *logic_timer = nullptr;
ALLEGRO_BITMAP *map_bitmap = nullptr;
ALLEGRO_BITMAP *pacmon_bitmap[4] = {nullptr}; // Array for 4 directional images
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
int pacmon_direction = DIR_RIGHT; // Default direction

// Hunters positions
int hunter_move_counter = 0;

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

int hunter_modes[4] = {MODE_SCATTER, MODE_SCATTER, MODE_SCATTER, MODE_SCATTER};
int mode_timer = 0;
int scatter_targets_x[4] = {3, 3, 23, 23};  // Corners of the map
int scatter_targets_y[4] = {3, 23, 3, 23};  // Corners of the map

bool key[4] = { false, false, false, false };
bool redraw = true;
bool exit_game = false;
bool victory = false;
bool defeat = false;
bool update_logic = false;

int last_random_pos = -1;
int random_chase = -1;

// Game state control
bool game_over = false;
int end_game_timer = 0;
const int END_GAME_DELAY = 180; // About 3 seconds at 60 FPS

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

void hunterMovement(char map[][26], int &x, int &y, int &pos_x, int &pos_y, int hunter_id) {
    // Chance to change movement mode
    if (rand() % 100 < 5) { // 5% chance each move
        hunter_modes[hunter_id] = rand() % 3; // Choose between scatter, chase, random
    }
    
    // Change target based on mode
    int target_x = pacmon_i; // Default chase mode
    int target_y = pacmon_j;
    
    if (hunter_modes[hunter_id] == MODE_SCATTER) {
        // Scatter to corners
        target_x = scatter_targets_x[hunter_id];
        target_y = scatter_targets_y[hunter_id];
    }
    else if (hunter_modes[hunter_id] == MODE_RANDOM) {
        // Create temporary random target
        target_x = 5 + (rand() % 15);
        target_y = 5 + (rand() % 15);
    }
    
    // Special case for escaping the house
    if (x >= 11 && x <= 14 && y >= 9 && y <= 15) {
        // In the house - prioritize moving upward or toward exits
        if (map[x-1][y] != '1') {
            x--;
            pos_y = x * cell_size;
            return;
        }
        // Try sideways if up is blocked
        else if (y < 12 && map[x][y+1] != '1') {
            y++;
            pos_x = y * cell_size;
            return;
        }
        else if (y > 12 && map[x][y-1] != '1') {
            y--;
            pos_x = y * cell_size;
            return;
        }
    }

    // Teleport functionality
    if (x == 12 && y == -1) {
        x = 12; y = 24;
        pos_y = x * cell_size; pos_x = y * cell_size;
        return;
    } 
    else if (x == 12 && y == 25) {
        x = 12; y = 0;
        pos_y = x * cell_size; pos_x = y * cell_size;
        return;
    }

    // Store current position to detect being stuck
    static int last_positions[4][2][10]; // Store last 10 positions for each hunter
    static int pos_index[4] = {0, 0, 0, 0};
    
    // Update position history
    last_positions[hunter_id][0][pos_index[hunter_id]] = x;
    last_positions[hunter_id][1][pos_index[hunter_id]] = y;
    pos_index[hunter_id] = (pos_index[hunter_id] + 1) % 10;
    
    // Check if stuck (revisiting same positions)
    bool is_stuck = false;
    int same_pos_count = 0;
    for (int i = 0; i < 10; i++) {
        if (last_positions[hunter_id][0][i] == x && last_positions[hunter_id][1][i] == y) {
            same_pos_count++;
        }
    }
    
    if (same_pos_count > 3) {
        is_stuck = true;
    }

    // Calculate possible moves
    int possible_moves[4][2] = {{1, 0}, {0, -1}, {-1, 0}, {0, 1}}; // Down, Left, Up, Right
    double move_scores[4];
    bool move_valid[4] = {false, false, false, false};
    int valid_count = 0;
    
    // Get last move direction
    static int last_dir[4] = {-1, -1, -1, -1};
    
    for (int i = 0; i < 4; i++) {
        int new_x = x + possible_moves[i][0];
        int new_y = y + possible_moves[i][1];
        
        // Check if move is valid (not a wall)
        if (map[new_x][new_y] != '1') {
            move_valid[i] = true;
            valid_count++;
            
            // Calculate base score as distance to target
            double dist_to_target = distance(new_x, new_y, target_x, target_y);
            
            // In scatter mode, we want to maximize distance from PacMon when close
            if (hunter_modes[hunter_id] == MODE_SCATTER && 
                distance(x, y, pacmon_i, pacmon_j) < 5) {
                dist_to_target = 100 - distance(new_x, new_y, pacmon_i, pacmon_j);
            }
            
            move_scores[i] = 1000 - dist_to_target; // Higher score is better
            
            // Avoid reversing direction unless necessary
            if ((i == 0 && last_dir[hunter_id] == 2) || 
                (i == 2 && last_dir[hunter_id] == 0) ||
                (i == 1 && last_dir[hunter_id] == 3) ||
                (i == 3 && last_dir[hunter_id] == 1)) {
                move_scores[i] -= 500;
            }
            
            // Add randomness to avoid predictable patterns
            move_scores[i] += (rand() % 100) * (is_stuck ? 10 : 1);
            
            // Prefer continuing in same direction slightly
            if (i == last_dir[hunter_id]) {
                move_scores[i] += 50;
            }
        }
    }
    
    // Choose best valid move
    int best_move = -1;
    double best_score = -99999;
    
    for (int i = 0; i < 4; i++) {
        if (move_valid[i] && move_scores[i] > best_score) {
            best_score = move_scores[i];
            best_move = i;
        }
    }
    
    // If no good move, choose random valid move
    if (best_move == -1 && valid_count > 0) {
        while (best_move == -1 || !move_valid[best_move]) {
            best_move = rand() % 4;
        }
    }
    
    // Execute the chosen move
    if (best_move != -1) {
        x += possible_moves[best_move][0];
        y += possible_moves[best_move][1];
        pos_y = x * cell_size;
        pos_x = y * cell_size;
        last_dir[hunter_id] = best_move;
    }
    
    // If still stuck, teleport a little bit
    if (is_stuck && rand() % 5 == 0) {
        for (int i = 0; i < 10; i++) {
            int test_x = x + (rand() % 7) - 3;
            int test_y = y + (rand() % 7) - 3;
            if (test_x > 0 && test_x < 24 && test_y > 0 && test_y < 24 && 
                map[test_x][test_y] != '1') {
                x = test_x;
                y = test_y;
                pos_y = x * cell_size;
                pos_x = y * cell_size;
                break;
            }
        }
    }
}

// Improved boundary checking function
bool isValidMove(int i, int j) {
    // Check map boundaries
    if (i < 0 || i >= 25 || j < 0 || j >= 25) return false;
    
    // Check if the cell is not a wall
    return MAP[i][j] != '1';
}

// Check if hunter catches pacmon
bool checkCollision() {
    return (
        (pacmon_i == hunter_red_i && pacmon_j == hunter_red_j) ||
        (pacmon_i == hunter_blue_i && pacmon_j == hunter_blue_j) ||
        (pacmon_i == hunter_pink_i && pacmon_j == hunter_pink_j) ||
        (pacmon_i == hunter_orange_i && pacmon_j == hunter_orange_j)
    );
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
    
    logic_timer = al_create_timer(1.0 / LOGIC_FPS);
    if (!logic_timer) {
        cout << "Failed to create logic timer!" << endl;
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
        al_destroy_timer(logic_timer);
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

    // Load pacmon directional images
    pacmon_bitmap[DIR_RIGHT] = al_load_bitmap("assets/characters/pacright.png");
    pacmon_bitmap[DIR_DOWN] = al_load_bitmap("assets/characters/pacdown.png");  // Replace with correct files
    pacmon_bitmap[DIR_LEFT] = al_load_bitmap("assets/characters/pacleft.png");  // when you have them
    pacmon_bitmap[DIR_UP] = al_load_bitmap("assets/characters/pacup.png");
    
    if (!pacmon_bitmap[DIR_RIGHT]) {
        cout << "Failed to load pacmon right!" << endl;
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
        al_destroy_timer(logic_timer);
        return 0;
    }

    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_timer_event_source(logic_timer));
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    al_clear_to_color(al_map_rgb(91, 141, 91));
    al_flip_display();
    al_start_timer(timer);
    al_start_timer(logic_timer);

    return 1;
}

int main(int argc, char **argv) {
    int game_time = 0;

    if (!initialize()) return -1;

    al_attach_audio_stream_to_mixer(music, al_get_default_mixer());
    al_set_audio_stream_playing(music, true);

    while (!exit_game) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_TIMER) {
            if (ev.timer.source == logic_timer && !game_over) {
                game_time++;
                update_logic = true;
                
                // Process movement only on logic timer ticks
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
                    pacmon_i = 12;
                    pacmon_j = 0;
                    pacmon_y = pacmon_i * cell_size;
                    pacmon_x = pacmon_j * cell_size;
                }
                else if (pacmon_i == 12 && pacmon_j <= -1) {
                    pacmon_i = 12;
                    pacmon_j = 24;
                    pacmon_y = pacmon_i * cell_size;
                    pacmon_x = pacmon_j * cell_size;
                }
                else if (pacmon_i == 12 && pacmon_j == 0 && key[KEY_LEFT]) {
                    pacmon_j = 24;
                    pacmon_x = pacmon_j * cell_size;
                }
                else if (pacmon_i == 12 && pacmon_j == 24 && key[KEY_RIGHT]) {
                    pacmon_j = 0;
                    pacmon_x = pacmon_j * cell_size;
                }
                
                // Move hunters based on game time
                if (update_logic) {
                    mode_timer++;
                    
                    // Periodically force mode changes
                    if (mode_timer % 150 == 0) {
                        for (int i = 0; i < 4; i++) {
                            hunter_modes[i] = (hunter_modes[i] + 1) % 3;
                        }
                    }
                    
                    // Blue hunter always moves
                    hunterMovement(MAP, hunter_blue_i, hunter_blue_j, hunter_blue_x, hunter_blue_y, 0);
                    
                    // Other hunters come out over time
                    if (game_time >= 30)
                        hunterMovement(MAP, hunter_orange_i, hunter_orange_j, hunter_orange_x, hunter_orange_y, 1);
                    
                    if (game_time >= 50)
                        hunterMovement(MAP, hunter_pink_i, hunter_pink_j, hunter_pink_x, hunter_pink_y, 2);
                    
                    if (game_time >= 70)
                        hunterMovement(MAP, hunter_red_i, hunter_red_j, hunter_red_x, hunter_red_y, 3);
                }
                
                // pacmon eats a banana
                if (MAP[pacmon_i][pacmon_j] == '2') {
                    MAP[pacmon_i][pacmon_j] = '0';
                    al_play_sample(banana_sound, 0.5, 0.0, 2.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
                    bananas--;
                    score++;
                }
                
                // Check for victory
                if (bananas == 0) {
                    victory = true;
                    game_over = true;
                    al_stop_samples();
                    al_destroy_audio_stream(music);
                    al_play_sample(victory_sound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
                }
                
                // Check for defeat
                if (checkCollision()) {
                    defeat = true;
                    game_over = true;
                    al_stop_samples();
                    al_destroy_audio_stream(music);
                    music = nullptr; // Prevent double free
                    al_play_sample(defeat_sound, 1.0, 0.0, 1.0, ALLEGRO_PLAYMODE_ONCE, nullptr);
                }
            }
            else if (ev.timer.source == timer) {
                // Render frame updates on the display timer (higher FPS)
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
        }
        else if (ev.type == ALLEGRO_EVENT_KEY_UP) {
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
                // Normal gameplay rendering
                al_draw_bitmap(map_bitmap, 0, 0, 0);
                
                // Draw PacMon with appropriate directional sprite
                al_draw_bitmap(pacmon_bitmap[pacmon_direction], pacmon_x, pacmon_y, 0);
                
                // Draw hunters
                al_draw_bitmap(hunter_red, hunter_red_x, hunter_red_y, 0);
                al_draw_bitmap(hunter_blue, hunter_blue_x, hunter_blue_y, 0);
                al_draw_bitmap(hunter_pink, hunter_pink_x, hunter_pink_y, 0);
                al_draw_bitmap(hunter_orange, hunter_orange_x, hunter_orange_y, 0);
                
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
                
                // Draw score
                al_draw_textf(font, al_map_rgb(255, 255, 255), 0, 515, 0, "Score: %d", score);
            }
            
            al_flip_display();
        }
    }

    // Clean up resources properly
    if (banana_sound) al_destroy_sample(banana_sound);
    if (victory_sound) al_destroy_sample(victory_sound);
    if (defeat_sound) al_destroy_sample(defeat_sound);
    if (music) al_destroy_audio_stream(music);
    
    for (int i = 0; i < 4; i++) {
        if (pacmon_bitmap[i]) al_destroy_bitmap(pacmon_bitmap[i]);
    }
    
    if (map_bitmap) al_destroy_bitmap(map_bitmap);
    if (hunter_red) al_destroy_bitmap(hunter_red);
    if (hunter_blue) al_destroy_bitmap(hunter_blue);
    if (hunter_pink) al_destroy_bitmap(hunter_pink);
    if (hunter_orange) al_destroy_bitmap(hunter_orange);
    if (banana_bitmap) al_destroy_bitmap(banana_bitmap);
    if (victory_bitmap) al_destroy_bitmap(victory_bitmap);
    if (defeat_bitmap) al_destroy_bitmap(defeat_bitmap);
    if (font) al_destroy_font(font);
    if (timer) al_destroy_timer(timer);
    if (logic_timer) al_destroy_timer(logic_timer);
    if (display) al_destroy_display(display);
    if (event_queue) al_destroy_event_queue(event_queue);

    return 0;
}