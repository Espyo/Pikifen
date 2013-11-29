#ifndef ENEMY_STATE_INCLUDED
#define ENEMY_STATE_INCLUDED

#include <vector>

#include "hitbox.h"
#include "state.h"

using namespace std;

class enemy_state : public state {
    float speed_x, speed_y;     //Movement, units per second.
    vector<hitbox*> hitboxes;
};

#endif //ifndef ENEMY_STATE_INCLUDED
