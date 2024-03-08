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

#include "../misc_structs.h"
#include "../libs/data_file.h"
#include "mob_type.h"


namespace LEADER_TYPE {
extern const float DEF_WHISTLE_RANGE;
}


//Leader object states.
enum LEADER_STATES {

    //Idling.
    LEADER_STATE_IDLING,
    
    //Active.
    LEADER_STATE_ACTIVE,
    
    //Whistling.
    LEADER_STATE_WHISTLING,
    
    //Punching.
    LEADER_STATE_PUNCHING,
    
    //Holding a Pikmin.
    LEADER_STATE_HOLDING,
    
    //Dismissing.
    LEADER_STATE_DISMISSING,
    
    //Spraying.
    LEADER_STATE_SPRAYING,
    
    //In pain.
    LEADER_STATE_PAIN,
    
    //In pain, inactive.
    LEADER_STATE_INACTIVE_PAIN,
    
    //Knocked back.
    LEADER_STATE_KNOCKED_BACK,
    
    //Knocked back, inactive.
    LEADER_STATE_INACTIVE_KNOCKED_BACK,
    
    //Dying.
    LEADER_STATE_DYING,
    
    //In group, chasing.
    LEADER_STATE_IN_GROUP_CHASING,
    
    //In group, stopped.
    LEADER_STATE_IN_GROUP_STOPPED,
    
    //Going to pluck a Pikmin.
    LEADER_STATE_GOING_TO_PLUCK,
    
    //Plucking a Pikmin.
    LEADER_STATE_PLUCKING,
    
    //Deciding what Pikmin to pluck next.
    LEADER_STATE_PLUCK_DECIDING,
    
    //Going to pluck a Pikmin, inactive.
    LEADER_STATE_INACTIVE_GOING_TO_PLUCK,
    
    //Plucking a Pikmin, inactive.
    LEADER_STATE_INACTIVE_PLUCKING,
    
    //Deciding what Pikmin to pluck next, inactive.
    LEADER_STATE_INACTIVE_PLUCK_DECIDING,
    
    //Mid Go Here.
    LEADER_STATE_MID_GO_HERE,
    
    //Mid Go Here, inactive.
    LEADER_STATE_INACTIVE_MID_GO_HERE,
    
    //Sleeping, waiting to move.
    LEADER_STATE_SLEEPING_WAITING,
    
    //Sleeping, moving.
    LEADER_STATE_SLEEPING_MOVING,
    
    //Sleeping, stuck.
    LEADER_STATE_SLEEPING_STUCK,
    
    //Sleeping, waiting to move, inactive.
    LEADER_STATE_INACTIVE_SLEEPING_WAITING,
    
    //Sleeping, moving, inactive.
    LEADER_STATE_INACTIVE_SLEEPING_MOVING,
    
    //Sleeping, stuck, inactive.
    LEADER_STATE_INACTIVE_SLEEPING_STUCK,
    
    //Getting up from lying down.
    LEADER_STATE_WAKING_UP,
    
    //Getting up from lying down, inactive.
    LEADER_STATE_INACTIVE_WAKING_UP,
    
    //Held by another leader.
    LEADER_STATE_HELD,
    
    //Thrown.
    LEADER_STATE_THROWN,
    
    //Drinking a drop.
    LEADER_STATE_DRINKING,
    
    //Riding a track.
    LEADER_STATE_RIDING_TRACK,
    
    //Riding a track, inactive.
    LEADER_STATE_INACTIVE_RIDING_TRACK,
    
    //Total amount of leader object states.
    N_LEADER_STATES,
    
    
};


//Leader object animations.
enum LEADER_ANIMATIONS {

    //Idling.
    LEADER_ANIM_IDLING,
    
    //Walking.
    LEADER_ANIM_WALKING,
    
    //Plucking.
    LEADER_ANIM_PLUCKING,
    
    //Getting up.
    LEADER_ANIM_GETTING_UP,
    
    //Dismissing.
    LEADER_ANIM_DISMISSING,
    
    //Throwing.
    LEADER_ANIM_THROWING,
    
    //Whistling.
    LEADER_ANIM_WHISTLING,
    
    //Punching.
    LEADER_ANIM_PUNCHING,
    
    //Lying down.
    LEADER_ANIM_LYING,
    
    //In pain.
    LEADER_ANIM_PAIN,
    
    //Knocked down.
    LEADER_ANIM_KNOCKED_DOWN,
    
    //Spraying.
    LEADER_ANIM_SPRAYING,
    
    //Drinking a drop.
    LEADER_ANIM_DRINKING,
    
};


//Leader object sounds.
enum LEADER_SOUNDS {

    //Dismissing their group.
    LEADER_SOUND_DISMISSING,
    
    //Name call when they are swapped to.
    LEADER_SOUND_NAME_CALL,
    
    //Whistling.
    LEADER_SOUND_WHISTLING,
    
    //Total amount of sounds.
    N_LEADER_SOUNDS,
    
};


/**
 * @brief A type of leader.
 *
 * The "leader" class is a mob, so the walking Olimar,
 * walking Louie, etc. This leader type is actually the definition of
 * what the leader is like. Maybe this will be clearer:
 * The same way you have enemies and enemy types, you can have more
 * than one leader on the map that is of the same leader type;
 * this means you can have 3 Olimars, if you want.
 * Why would you do that, though?
 */
class leader_type : public mob_type {

public:

    //--- Members ---
    
    //How far its whistle reaches from the center point.
    float whistle_range = LEADER_TYPE::DEF_WHISTLE_RANGE;
    
    //How high it can reach when thrown.
    float max_throw_height = 0.0f;
    
    //Standby icon.
    ALLEGRO_BITMAP* bmp_icon = nullptr;
    
    //Sound data index for each sound. Cache for performance.
    size_t sfx_data_idxs[N_LEADER_SOUNDS];
    
    
    //--- Function declarations ---
    
    leader_type();
    void load_properties(data_node* file) override;
    void load_resources(data_node* file) override;
    anim_conversion_vector get_anim_conversions() const override;
    void unload_resources() override;
    
};


#endif //ifndef LEADER_TYPE_INCLUDED
