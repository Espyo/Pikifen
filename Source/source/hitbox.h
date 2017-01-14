/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the hitbox class and hitbox-related functions.
 */

#ifndef HITBOX_INCLUDED
#define HITBOX_INCLUDED

#include <vector>

#include "const.h"

/*
 * You can read more about hitboxes
 * on animation.h.
 */

enum HITBOX_TYPES {
    HITBOX_TYPE_NORMAL,
    HITBOX_TYPE_ATTACK,
    HITBOX_TYPE_DISABLED,
};

struct hazard;


/* ----------------------------------------------------------------------------
 * A body part.
 */
class body_part {
public:
    string name;
    
    body_part(const string &name = "");
};


/* ----------------------------------------------------------------------------
 * A hitbox in a sprite. Despite the name, it is a cilinder.
 */
class hitbox {
public:
    string body_part_name;
    size_t body_part_index;   //Cache for performance.
    body_part* body_part_ptr; //Cache for performance.
    float x, y;  //Center of the hitbox (relative coordinates).
    float z;     //Bottom of the hitbox (relative coordinates).
    float height;
    float radius;
    
    unsigned char type;
    string hazards_str;
    vector<hazard*> hazards;
    //If it's a normal hitbox, this is the defense multiplier.
    //If it's an attack one, the attack multiplier.
    float multiplier;
    //If true, the Pikmin is knocked away from the center.
    bool knockback_outward;
    //Knockback angle.
    float knockback_angle;
    //Knockback strength.
    float knockback;
    //Can the Pikmin latch on to this hitbox to continue inflicting damage?
    //Example of a non-latchable hitbox: Goolix' larger core.
    bool can_pikmin_latch;
    
    hitbox(
        const string &bpn = "", size_t bpi = INVALID, body_part* bpp = NULL,
        const float x = 0, const float y = 0,
        const float z = 0, const float height = 128, const float radius = 32
    );
};

#endif //ifndef HITBOX_INCLUDED
