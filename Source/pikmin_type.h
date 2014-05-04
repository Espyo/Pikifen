#ifndef PIKMIN_TYPE_INCLUDED
#define PIKMIN_TYPE_INCLUDED

#include <string>
#include <vector>

#include <allegro5/allegro.h>

#include "element.h"
#include "mob_type.h"

using namespace std;

class pikmin_type : public mob_type {
public:
    vector<element*> resistences;
    unsigned char attack_attribute; //ToDo What.
    float carry_strength;
    float attack_power;
    float attack_interval;
    float weight;
    float carry_speed;
    float size;
    bool has_onion;
    bool can_dig;
    bool can_fly;
    bool can_swim;
    bool can_latch;
    bool can_carry_bomb_rocks;
    ALLEGRO_BITMAP* bmp_top[3]; //Top (leaf/bud/flower) bitmap for each maturity.
    
    pikmin_type();
};

enum PIKMIN_ANIMATIONS {
    PIKMIN_ANIM_IDLE,
    PIKMIN_ANIM_WALK,
    PIKMIN_ANIM_THROWN,
    PIKMIN_ANIM_ATTACK,
    PIKMIN_ANIM_GRAB,
    PIKMIN_ANIM_BURROWED,
};

#endif //ifndef PIKMIN_TYPE_INCLUDED
