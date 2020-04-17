/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Pikmin type class and Pikmin type-related functions.
 */

#ifndef PIKMIN_TYPE_INCLUDED
#define PIKMIN_TYPE_INCLUDED

#include <string>
#include <vector>

#include <allegro5/allegro.h>

#include "../hazard.h"
#include "../utils/data_file.h"
#include "mob_type.h"


enum PIKMIN_STATES {
    PIKMIN_STATE_IN_GROUP_CHASING,
    PIKMIN_STATE_IN_GROUP_STOPPED,
    PIKMIN_STATE_SWARM_CHASING,
    PIKMIN_STATE_SWARM_STOPPED,
    PIKMIN_STATE_IDLING,
    PIKMIN_STATE_SEED,
    PIKMIN_STATE_SPROUT,
    PIKMIN_STATE_PLUCKING,
    PIKMIN_STATE_GRABBED_BY_LEADER,
    PIKMIN_STATE_GRABBED_BY_ENEMY,
    PIKMIN_STATE_KNOCKED_BACK,
    PIKMIN_STATE_THROWN,
    PIKMIN_STATE_GOING_TO_DISMISS_SPOT,
    PIKMIN_STATE_PICKING_UP,
    PIKMIN_STATE_ON_GROUP_TASK,
    PIKMIN_STATE_SIGHING,
    PIKMIN_STATE_CARRYING,
    PIKMIN_STATE_RETURNING,
    PIKMIN_STATE_ATTACKING_GROUNDED,
    PIKMIN_STATE_ATTACKING_LATCHED,
    PIKMIN_STATE_GOING_TO_CARRIABLE_OBJECT,
    PIKMIN_STATE_GOING_TO_TOOL,
    PIKMIN_STATE_GOING_TO_GROUP_TASK,
    PIKMIN_STATE_GOING_TO_OPPONENT,
    PIKMIN_STATE_DISABLED,
    PIKMIN_STATE_FLAILING,
    PIKMIN_STATE_PANICKING,
    PIKMIN_STATE_DRINKING,
    PIKMIN_STATE_CELEBRATING,
    PIKMIN_STATE_IN_GROUP_CHASING_H,
    PIKMIN_STATE_IN_GROUP_STOPPED_H,
    PIKMIN_STATE_SWARM_CHASING_H,
    PIKMIN_STATE_SWARM_STOPPED_H,
    PIKMIN_STATE_IDLING_H,
    PIKMIN_STATE_GRABBED_BY_LEADER_H,
    PIKMIN_STATE_THROWN_H,
    PIKMIN_STATE_GOING_TO_DISMISS_SPOT_H,
    PIKMIN_STATE_RIDING_TRACK,
    
    N_PIKMIN_STATES
};

enum PIKMIN_ANIMATIONS {
    PIKMIN_ANIM_IDLING,
    PIKMIN_ANIM_WALKING,
    PIKMIN_ANIM_THROWN,
    PIKMIN_ANIM_ATTACKING,
    PIKMIN_ANIM_GRABBING,
    PIKMIN_ANIM_CARRYING,
    PIKMIN_ANIM_SIGHING,
    PIKMIN_ANIM_SPROUT,
    PIKMIN_ANIM_PLUCKING,
    PIKMIN_ANIM_LYING,
    PIKMIN_ANIM_DRINKING,
    PIKMIN_ANIM_PICKING_UP,
};

const float PIKMIN_GOTO_TIMEOUT = 5.0f;
const float PIKMIN_INVULN_PERIOD = 0.7f;
const float PIKMIN_PANIC_CHASE_INTERVAL = 0.2f;


/* ----------------------------------------------------------------------------
 * Pikmin types, almost the basic meat of the fangames.
 * The canon ones (at the time of writing this) are
 * Red, Yellow, Blue, White, Purple, Bulbmin, Winged, and Rock,
 * but with the engine, loads of fan-made ones can be made.
 */
class pikmin_type : public mob_type {
public:
    float carry_strength;
    float push_strength;
    float max_throw_height;
    bool can_carry_tools;
    float sprout_evolution_time[N_MATURITIES];
    //Top (leaf/bud/flower) bitmap for each maturity.
    ALLEGRO_BITMAP* bmp_top[N_MATURITIES];
    //Standby icon.
    ALLEGRO_BITMAP* bmp_icon;
    //Standby maturity icons.
    ALLEGRO_BITMAP* bmp_maturity_icon[N_MATURITIES];
    
    pikmin_type();
    void load_properties(data_node* file);
    void load_resources(data_node* file);
    anim_conversion_vector get_anim_conversions() const;
    void unload_resources();
    
};

#endif //ifndef PIKMIN_TYPE_INCLUDED
