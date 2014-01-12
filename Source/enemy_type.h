#ifndef ENEMY_TYPE_INCLUDED
#define ENEMY_TYPE_INCLUDED

#include <string>
#include <vector>

#include "element.h"
#include "mob_type.h"

using namespace std;

class enemy_type : public mob_type {
public:
    unsigned char pikmin_seeds;
    float value;
    float revive_speed;
    bool can_regenerate;
    bool is_boss;
    bool drops_corpse;
};

#endif //ifndef ENEMY_TYPE_INCLUDED
