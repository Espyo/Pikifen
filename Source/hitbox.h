/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the hitbox class and hitbox-related functions.
 */

#ifndef HITBOX_INCLUDED
#define HITBOX_INCLUDED

#include <vector>

#include "element.h"

/*
 * You can read more about hitboxes
 * on animation.h.
 */

enum HITBOX_TYPES {
    HITBOX_TYPE_NORMAL,
    HITBOX_TYPE_ATTACK,
};

class hitbox {
public:
    string name;
    unsigned char type;
    string elements;
    float multiplier;       //If it's a normal hitbox, this is the defense multiplier. If it's an attack one, the attack multiplier.
    bool knockback_outward; //If true, the Pikmin is knocked away from the center.
    float knockback_angle;  //Knockback angle.
    float knockback;        //Knockback strength.
    bool can_pikmin_latch;  //Can the Pikmin latch on to this hitbox to continue inflicting damage? Example of a non-latchable hitbox: Goolix' larger core.
    
    hitbox(const string &name = "");
};

class hitbox_instance {
public:
    string hitbox_name;
    size_t hitbox_nr;   //Needed for performance.
    hitbox* hitbox_ptr; //Needed for performance.
    float x, y, z;      //Relative coordinates.
    float radius;
    
    hitbox_instance(
        const string &hn = "", size_t hnr = string::npos, hitbox* hp = NULL, const float x = 0, const float y = 0,
        const float z = 0, const float radius = 32
    );
};

#endif //ifndef