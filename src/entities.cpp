#include "../include/entities.h"
#include <cmath>
#include <cstdlib>

double distance(int x1, int y1, int x2, int y2) {
    return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

void hunterMovement(char map[][26], int &x, int &y, int &pos_x, int &pos_y, int hunter_id) {
    if (rand() % 100 < 5) {
        hunter_modes[hunter_id] = rand() % 3;
    }

    int target_x = pacmon_i;
    int target_y = pacmon_j;

    if (hunter_modes[hunter_id] == MODE_SCATTER) {
        target_x = scatter_targets_x[hunter_id];
        target_y = scatter_targets_y[hunter_id];
    }
    else if (hunter_modes[hunter_id] == MODE_RANDOM) {
        target_x = 5 + (rand() % 15);
        target_y = 5 + (rand() % 15);
    }

    if (x >= 11 && x <= 14 && y >= 9 && y <= 15) {
        if (map[x-1][y] != '1') {
            x--; pos_y = x * cell_size; return;
        }
        else if (y < 12 && map[x][y+1] != '1') {
            y++; pos_x = y * cell_size; return;
        }
        else if (y > 12 && map[x][y-1] != '1') {
            y--; pos_x = y * cell_size; return;
        }
    }

    if (x == 12 && y == -1) {
        y = 24; pos_x = y * cell_size; return;
    }
    else if (x == 12 && y == 25) {
        y = 0; pos_x = y * cell_size; return;
    }

    static int last_positions[4][2][10];
    static int pos_index[4] = {0, 0, 0, 0};
    last_positions[hunter_id][0][pos_index[hunter_id]] = x;
    last_positions[hunter_id][1][pos_index[hunter_id]] = y;
    pos_index[hunter_id] = (pos_index[hunter_id] + 1) % 10;

    bool is_stuck = false;
    int same_pos_count = 0;
    for (int i = 0; i < 10; i++) {
        if (last_positions[hunter_id][0][i] == x && last_positions[hunter_id][1][i] == y) {
            same_pos_count++;
        }
    }
    if (same_pos_count > 3) is_stuck = true;

    int possible_moves[4][2] = {{1, 0}, {0, -1}, {-1, 0}, {0, 1}};
    double move_scores[4];
    bool move_valid[4] = {false, false, false, false};
    int valid_count = 0;
    static int last_dir[4] = {-1, -1, -1, -1};

    for (int i = 0; i < 4; i++) {
        int new_x = x + possible_moves[i][0];
        int new_y = y + possible_moves[i][1];
        if (map[new_x][new_y] != '1') {
            move_valid[i] = true;
            valid_count++;
            double dist_to_target = distance(new_x, new_y, target_x, target_y);
            if (hunter_modes[hunter_id] == MODE_SCATTER && distance(x, y, pacmon_i, pacmon_j) < 5) {
                dist_to_target = 100 - distance(new_x, new_y, pacmon_i, pacmon_j);
            }
            move_scores[i] = 1000 - dist_to_target;
            if ((i == 0 && last_dir[hunter_id] == 2) || (i == 2 && last_dir[hunter_id] == 0) ||
                (i == 1 && last_dir[hunter_id] == 3) || (i == 3 && last_dir[hunter_id] == 1)) {
                move_scores[i] -= 500;
            }
            move_scores[i] += (rand() % 100) * (is_stuck ? 10 : 1);
            if (i == last_dir[hunter_id]) move_scores[i] += 50;
        }
    }

    int best_move = -1;
    double best_score = -99999;
    for (int i = 0; i < 4; i++) {
        if (move_valid[i] && move_scores[i] > best_score) {
            best_score = move_scores[i];
            best_move = i;
        }
    }

    if (best_move == -1 && valid_count > 0) {
        do {
            best_move = rand() % 4;
        } while (!move_valid[best_move]);
    }

    if (best_move != -1) {
        x += possible_moves[best_move][0];
        y += possible_moves[best_move][1];
        pos_y = x * cell_size;
        pos_x = y * cell_size;
        last_dir[hunter_id] = best_move;
    }

    if (is_stuck && rand() % 5 == 0) {
        for (int i = 0; i < 10; i++) {
            int test_x = x + (rand() % 7) - 3;
            int test_y = y + (rand() % 7) - 3;
            if (test_x > 0 && test_x < 24 && test_y > 0 && test_y < 24 && map[test_x][test_y] != '1') {
                x = test_x;
                y = test_y;
                pos_y = x * cell_size;
                pos_x = y * cell_size;
                break;
            }
        }
    }
}

bool isValidMove(int i, int j) {
    if (i < 0 || i >= 25 || j < 0 || j >= 25) return false;
    return MAP[i][j] != '1';
}

bool checkCollision() {
    return (
        (pacmon_i == hunter_red_i && pacmon_j == hunter_red_j) ||
        (pacmon_i == hunter_blue_i && pacmon_j == hunter_blue_j) ||
        (pacmon_i == hunter_pink_i && pacmon_j == hunter_pink_j) ||
        (pacmon_i == hunter_orange_i && pacmon_j == hunter_orange_j)
    );
}