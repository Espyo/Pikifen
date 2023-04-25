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
#include "../libs/data_file.h"
#include "mob_type.h"


//Pikmin object states.
enum PIKMIN_STATES {
    //In group, chasing.
    PIKMIN_STATE_IN_GROUP_CHASING,
    //In group, stopped.
    PIKMIN_STATE_IN_GROUP_STOPPED,
    //Swarming, chasing.
    PIKMIN_STATE_SWARM_CHASING,
    //Swarming, stopped.
    PIKMIN_STATE_SWARM_STOPPED,
    //Idling.
    PIKMIN_STATE_IDLING,
    //As a seed.
    PIKMIN_STATE_SEED,
    //As a sprout.
    PIKMIN_STATE_SPROUT,
    //Being plucked.
    PIKMIN_STATE_PLUCKING,
    //Thrown backwards after being plucked.
    PIKMIN_STATE_PLUCKING_THROWN,
    //Leaving Onion.
    PIKMIN_STATE_LEAVING_ONION,
    //Entering Onion.
    PIKMIN_STATE_ENTERING_ONION,
    //Grabbed by leader.
    PIKMIN_STATE_GRABBED_BY_LEADER,
    //Grabbed by enemy.
    PIKMIN_STATE_GRABBED_BY_ENEMY,
    //Getting knocked back.
    PIKMIN_STATE_KNOCKED_BACK,
    //Knocked down on the floor.
    PIKMIN_STATE_KNOCKED_DOWN,
    //Getting up.
    PIKMIN_STATE_GETTING_UP,
    //Bouncing after performing an impact attack.
    PIKMIN_STATE_IMPACT_BOUNCE,
    //Lunging to perform an impact attack.
    PIKMIN_STATE_IMPACT_LUNGE,
    //Thrown.
    PIKMIN_STATE_THROWN,
    //Landing on mob.
    PIKMIN_STATE_MOB_LANDING,
    //Going to dismiss spot.
    PIKMIN_STATE_GOING_TO_DISMISS_SPOT,
    //Picking something up.
    PIKMIN_STATE_PICKING_UP,
    //Working on grop task.
    PIKMIN_STATE_ON_GROUP_TASK,
    //Sighing.
    PIKMIN_STATE_SIGHING,
    //Carrying.
    PIKMIN_STATE_CARRYING,
    //Returning to a pile.
    PIKMIN_STATE_RETURNING,
    //Attacking, grounded.
    PIKMIN_STATE_ATTACKING_GROUNDED,
    //Attacking, latched.
    PIKMIN_STATE_ATTACKING_LATCHED,
    //Goign to a carriable object.
    PIKMIN_STATE_GOING_TO_CARRIABLE_OBJECT,
    //Going to a tool.
    PIKMIN_STATE_GOING_TO_TOOL,
    //Going to an opponent.
    PIKMIN_STATE_GOING_TO_OPPONENT,
    //Circling around opponent.
    PIKMIN_STATE_CIRCLING_OPPONENT,
    //Going to a group task.
    PIKMIN_STATE_GOING_TO_GROUP_TASK,
    //Going to an Onion.
    PIKMIN_STATE_GOING_TO_ONION,
    //Riding a track.
    PIKMIN_STATE_RIDING_TRACK,
    //Helpless.
    PIKMIN_STATE_HELPLESS,
    //Flailing.
    PIKMIN_STATE_FLAILING,
    //Panicking.
    PIKMIN_STATE_PANICKING,
    //Drinking a drop.
    PIKMIN_STATE_DRINKING,
    //Celebrating.
    PIKMIN_STATE_CELEBRATING,
    //In group, chasing, whilst holding a tool.
    PIKMIN_STATE_IN_GROUP_CHASING_H,
    //In group, stopped, whilst holding a tool.
    PIKMIN_STATE_IN_GROUP_STOPPED_H,
    //Swarming, chasing, whilst holding a tool.
    PIKMIN_STATE_SWARM_CHASING_H,
    //Swarming, stopped, whilst holding a tool.
    PIKMIN_STATE_SWARM_STOPPED_H,
    //Idling, whilst holding a tool.
    PIKMIN_STATE_IDLING_H,
    //Grabbed by a leader, whilst holding a tool.
    PIKMIN_STATE_GRABBED_BY_LEADER_H,
    //Thrown, whilst holding a tool.
    PIKMIN_STATE_THROWN_H,
    //Going to dismiss spot, whilst holding a tool.
    PIKMIN_STATE_GOING_TO_DISMISS_SPOT_H,
    
    //Total amount of Pikmin object states.
    N_PIKMIN_STATES
};


//Pikmin object animations.
enum PIKMIN_ANIMATIONS {
    //Idling.
    PIKMIN_ANIM_IDLING,
    //Walking.
    PIKMIN_ANIM_WALKING,
    //Thrown.
    PIKMIN_ANIM_THROWN,
    //Landing on mob.
    PIKMIN_ANIM_MOB_LANDING,
    //Attacking.
    PIKMIN_ANIM_ATTACKING,
    //Grabbing.
    PIKMIN_ANIM_GRABBING,
    //Carrying.
    PIKMIN_ANIM_CARRYING,
    //Sighing.
    PIKMIN_ANIM_SIGHING,
    //As a sprout.
    PIKMIN_ANIM_SPROUT,
    //Being plucked.
    PIKMIN_ANIM_PLUCKING,
    //Getting knocked back.
    PIKMIN_ANIM_KNOCKED_BACK,
    //Lying on the ground.
    PIKMIN_ANIM_LYING,
    //Getting up.
    PIKMIN_ANIM_GETTING_UP,
    //Drinking a drop.
    PIKMIN_ANIM_DRINKING,
    //Picking something up.
    PIKMIN_ANIM_PICKING_UP,
    //Sliding.
    PIKMIN_ANIM_SLIDING,
};


//Pikmin type attack methods.
enum PIKMIN_ATTACK_METHODS {
    //Latches on and attacks.
    PIKMIN_ATTACK_LATCH,
    //Lunges forward for an impact.
    PIKMIN_ATTACK_IMPACT,
};


/* ----------------------------------------------------------------------------
 * Pikmin types, almost the basic meat of the fangames.
 * The canon ones (at the time of writing this) are
 * Red, Yellow, Blue, White, Purple, Bulbmin, Winged, and Rock,
 * but with the engine, loads of fan-made ones can be made.
 */
class pikmin_type : public mob_type {
public:
    //How many Pikmin they are worth when carrying.
    float carry_strength;
    //How many Pikmin they are worth when pushing.
    float push_strength;
    //Maximum height that the peak of their throw arc can reach.
    float max_throw_height;
    //What the main method of attack is.
    PIKMIN_ATTACK_METHODS attack_method;
    //How long it stays on the floor for after knocked down, if left alone.
    float knocked_down_duration;
    //A whistled Pikmin that got knocked down loses this much in lie-down time.
    float knocked_down_whistle_bonus;
    //Whether it can fly or not.
    bool can_fly;
    //Whether it can carry tool-type objects or not.
    bool can_carry_tools;
    //How long it takes to evolve in maturity, as a sprout.
    float sprout_evolution_time[N_MATURITIES];
    //Top (leaf/bud/flower) bitmap for each maturity.
    ALLEGRO_BITMAP* bmp_top[N_MATURITIES];
    //Standby icon.
    ALLEGRO_BITMAP* bmp_icon;
    //Standby maturity icons.
    ALLEGRO_BITMAP* bmp_maturity_icon[N_MATURITIES];
    //Icon for its Onion.
    ALLEGRO_BITMAP* bmp_onion_icon;
    
    pikmin_type();
    void load_properties(data_node* file) override;
    void load_resources(data_node* file) override;
    anim_conversion_vector get_anim_conversions() const override;
    void unload_resources() override;
    
};


#endif //ifndef PIKMIN_TYPE_INCLUDED
