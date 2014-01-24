#ifndef HITBOX_INCLUDED
#define HITBOX_INCLUDED

#include <vector>

#include "element.h"

enum HITBOX_TYPES {
    HITBOX_TYPE_NORMAL,
    HITBOX_TYPE_ATTACK,
    HITBOX_TYPE_SHAKE,
    HITBOX_TYPE_CHOMP,
};

class hitbox {
public:
    string name;
    unsigned char type;
    float x, y, z;  //Relative coordinates.
    float radius;
    vector<element*> elements;
    float multiplier; //If it's a normal hitbox, this is the defense multiplier. If it's an attack one, the attack multiplier.
    float shake_angle;
    bool can_pikmin_latch; //Can the Pikmin latch on to this hitbox to continue inflicting damage? Example of a non-latchable hitbox: Goolix' larger core.
    bool swallow; //Is this chomp hitbox swallowing the Pikmin?
    
    hitbox(float x = 0, float y = 0, float z = 0, float radius = 0) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->radius = radius;
        type = HITBOX_TYPE_NORMAL;
        multiplier = 1;
        shake_angle = 0;
        can_pikmin_latch = false;
        swallow = false;
    }
};

#endif //ifndef