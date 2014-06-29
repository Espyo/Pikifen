/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the leader type class and leader type-related functions.
 */

#ifndef LEADER_TYPE_INCLUDED
#define LEADER_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "misc_structs.h"
#include "mob_type.h"

/*
 * A type of leader.
 */
class leader_type : public mob_type {
public:
    float whistle_range;
    unsigned int punch_strength;
    float pluck_delay; //Time until the Pikmin is actually popped out of the ground.
    
    sample_struct sfx_whistle;
    sample_struct sfx_dismiss;
    sample_struct sfx_name_call;
    
    ALLEGRO_BITMAP* bmp_icon; //Standby icon.
    
    leader_type();
};

enum LEADER_ANIMATIONS {
    LEADER_ANIM_IDLE,
    LEADER_ANIM_WALK,
    LEADER_ANIM_PLUCK,
    LEADER_ANIM_GET_UP,
    LEADER_ANIM_DISMISS,
    LEADER_ANIM_THROW,
    LEADER_ANIM_WHISTLING,
    LEADER_ANIM_LIE,
};

#endif //ifndef LEADER_TYPE_INCLUDED