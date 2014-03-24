#ifndef PIKMIN_INCLUDED
#define PIKMIN_INCLUDED

class leader;

#include "enemy.h"
#include "leader.h"
#include "pikmin_type.h"

class pikmin : public mob {
public:
    pikmin(float x, float y, sector* sec, pikmin_type* type);
    ~pikmin();
    
    pikmin_type* pik_type;
    float hazard_time_left;     //Time it has left until it drowns/chokes/etc.
    enemy* attacking_enemy;     //Enemy it's attacking.
    
    mob* wants_to_carry;        //Mob it wants to carry.
    mob* carrying_mob;          //Mob it's carrying.
    size_t carrying_spot;       //Carrying spot reserved for it.
    
    unsigned char maturity;     //0: leaf. 1: bud. 2: flower.
    bool pluck_reserved;        //If true, someone's already coming to pluck this Pikmin. This is to let other leaders now that they should pick another one.
};

#endif //ifndef PIKMIN_INCLUDED
