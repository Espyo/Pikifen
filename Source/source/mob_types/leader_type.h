/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the leader type class and leader type-related functions.
 */

#ifndef LEADER_TYPE_INCLUDED
#define LEADER_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "../data_file.h"
#include "../misc_structs.h"
#include "mob_type.h"

enum LEADER_STATES {
    LEADER_STATE_IDLING,
    LEADER_STATE_ACTIVE,
    LEADER_STATE_WHISTLING,
    LEADER_STATE_PUNCHING,
    LEADER_STATE_HOLDING,
    LEADER_STATE_DISMISSING,
    LEADER_STATE_SPRAYING,
    LEADER_STATE_PAIN,
    LEADER_STATE_INACTIVE_PAIN,
    LEADER_STATE_KNOCKED_BACK,
    LEADER_STATE_INACTIVE_KNOCKED_BACK,
    LEADER_STATE_DYING,
    LEADER_STATE_IN_GROUP_CHASING,
    LEADER_STATE_IN_GROUP_STOPPED,
    LEADER_STATE_GOING_TO_PLUCK,
    LEADER_STATE_PLUCKING,
    LEADER_STATE_INACTIVE_GOING_TO_PLUCK,
    LEADER_STATE_INACTIVE_PLUCKING,
    LEADER_STATE_SLEEPING_WAITING,
    LEADER_STATE_SLEEPING_MOVING,
    LEADER_STATE_SLEEPING_STUCK,
    LEADER_STATE_INACTIVE_SLEEPING_WAITING,
    LEADER_STATE_INACTIVE_SLEEPING_MOVING,
    LEADER_STATE_INACTIVE_SLEEPING_STUCK,
    //Time during which the leader is getting up.
    LEADER_STATE_WAKING_UP,
    //Time during which the leader is getting up.
    LEADER_STATE_INACTIVE_WAKING_UP,
    LEADER_STATE_HELD,
    LEADER_STATE_THROWN,
    LEADER_STATE_DRINKING,
    LEADER_STATE_RIDING_TRACK,
    LEADER_STATE_INACTIVE_RIDING_TRACK,
    
    N_LEADER_STATES,
    
};

enum LEADER_ANIMATIONS {
    LEADER_ANIM_IDLING,
    LEADER_ANIM_WALKING,
    LEADER_ANIM_PLUCKING,
    LEADER_ANIM_GETTING_UP,
    LEADER_ANIM_DISMISSING,
    LEADER_ANIM_THROWING,
    LEADER_ANIM_WHISTLING,
    LEADER_ANIM_PUNCHING,
    LEADER_ANIM_LYING,
    LEADER_ANIM_PAIN,
    LEADER_ANIM_KNOCKED_DOWN,
    LEADER_ANIM_SPRAYING,
    LEADER_ANIM_DRINKING,
};

const float LEADER_HELD_MOB_ANGLE = TAU / 2;
const float LEADER_HELD_MOB_DIST = 1.2f;
const float LEADER_INVULN_PERIOD = 1.5f;


/* ----------------------------------------------------------------------------
 * A type of leader. The "leader" class is a mob, so the walking Olimar,
 * walking Louie, etc. This leader type is actually the definition of
 * what the leader is like. Maybe this will be clearer:
 * The same way you have enemies and enemy types, you can have more
 * than one leader on the map that is of the same leader type;
 * this means you can have 3 Olimars, if you want.
 * Why would you do that, though?
 */
class leader_type : public mob_type {
public:
    float whistle_range;
    float max_throw_height;
    
    sample_struct sfx_whistle;
    sample_struct sfx_dismiss;
    sample_struct sfx_name_call;
    
    ALLEGRO_BITMAP* bmp_icon; //Standby icon.
    
    leader_type();
    void load_properties(data_node* file);
    void load_resources(data_node* file);
    anim_conversion_vector get_anim_conversions();
    void unload_resources();
};

#endif //ifndef LEADER_TYPE_INCLUDED
