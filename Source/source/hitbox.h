/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
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
 * An actual hitbox. This has the data about its type
 * and reactions to the game world.
 */
class hitbox {
public:
    string name;
    
    hitbox(const string &name = "");
};


/* ----------------------------------------------------------------------------
 * An instance of a hitbox in a frame.
 */
class hitbox_instance {
public:
    string hitbox_name;
    size_t hitbox_nr;   //Needed for performance.
    hitbox* hitbox_ptr; //Needed for performance.
    float x, y;         //Center of the hitbox (relative coordinates).
    float z;            //Bottom of the hitbox (relative coordinates).
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
    
    hitbox_instance(
        const string &hn = "", size_t hnr = INVALID, hitbox* hp = NULL,
        const float x = 0, const float y = 0,
        const float z = 0, const float height = 128, const float radius = 32
    );
};

#endif //ifndef HITBOX_INCLUDED
