#ifndef ENTITIES_H
#define ENTITIES_H

#include "game.h"

struct Chase {
    int x, y;
    char path;
    double dist;
};

double distance(int x1, int y1, int x2, int y2);
void hunterMovement(char map[][26], int &x, int &y, int &pos_x, int &pos_y, int hunter_id);
bool isValidMove(int i, int j);
bool checkCollision();

#endif