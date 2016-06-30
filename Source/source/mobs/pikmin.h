/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Pikmin class and Pikmin-related functions.
 */

#ifndef PIKMIN_INCLUDED
#define PIKMIN_INCLUDED

class leader;

#include "enemy.h"
#include "leader.h"
#include "onion.h"
#include "pikmin_type.h"

enum PIKMIN_STATES {
    PIKMIN_STATE_IN_GROUP_CHASING,
    PIKMIN_STATE_IN_GROUP_STOPPED,
    PIKMIN_STATE_GROUP_MOVE_CHASING,
    PIKMIN_STATE_GROUP_MOVE_STOPPED,
    PIKMIN_STATE_IDLE,
    PIKMIN_STATE_BURIED,
    PIKMIN_STATE_PLUCKING,
    PIKMIN_STATE_GRABBED_BY_LEADER,
    PIKMIN_STATE_GRABBED_BY_ENEMY,
    PIKMIN_STATE_KNOCKED_BACK,
    PIKMIN_STATE_THROWN,
    PIKMIN_STATE_GOING_TO_DISMISS_SPOT,
    PIKMIN_STATE_SIGHING,
    PIKMIN_STATE_CARRYING,
    PIKMIN_STATE_ATTACKING_GROUNDED,
    PIKMIN_STATE_ATTACKING_LATCHED,
    PIKMIN_STATE_CELEBRATING,
    PIKMIN_STATE_GOING_TO_CARRIABLE_OBJECT,
    PIKMIN_STATE_GOING_TO_OPPONENT,
    PIKMIN_STATE_DISABLED,
    PIKMIN_STATE_FLAILING,
    PIKMIN_STATE_PANIC,
    
    N_PIKMIN_STATES
};

enum PIKMIN_ANIMATIONS {
    PIKMIN_ANIM_IDLE,
    PIKMIN_ANIM_WALK,
    PIKMIN_ANIM_THROWN,
    PIKMIN_ANIM_ATTACK,
    PIKMIN_ANIM_GRAB,
    PIKMIN_ANIM_CARRY,
    PIKMIN_ANIM_SIGH,
    PIKMIN_ANIM_BURROWED,
    PIKMIN_ANIM_PLUCKING,
    PIKMIN_ANIM_LYING,
};

const float PIKMIN_GOTO_TIMEOUT = 5.0f;
const float PIKMIN_PANIC_CHASE_INTERVAL = 0.2f;


/* ----------------------------------------------------------------------------
 * The eponymous Pikmin.
 */
class pikmin : public mob {
protected:
    virtual void tick_class_specifics();
    
public:
    pikmin(
        const float x, const float y, pikmin_type* type,
        const float angle, const string &vars
    );
    ~pikmin();
    
    virtual void draw();
    virtual float get_base_speed();
    
    pikmin_type* pik_type;
    
    //Number of the hitbox the Pikmin is attached to.
    size_t connected_hitbox_nr;
    //Distance percentage from the center of the hitbox
    //to the Pikmin's position.
    float connected_hitbox_dist;
    //Angle the Pikmin makes with the center of the hitbox
    //(with the hitbox' owner at 0 degrees).
    float connected_hitbox_angle;
    //Time left until the strike.
    float attack_time;
    
    //Mob that it is carrying.
    mob* carrying_mob;
    //Carrying spot reserved for it.
    size_t carrying_spot;
    
    //0: leaf. 1: bud. 2: flower.
    unsigned char maturity;
    //If true, someone's already coming to pluck this Pikmin.
    //This is to let other leaders know that they should pick a different one.
    bool pluck_reserved;
    
    void do_attack(mob* m, hitbox_instance* victim_hitbox_i);
    void set_connected_hitbox_info(hitbox_instance* hi_ptr, mob* mob_ptr);
    void teleport_to_connected_hitbox();
    
    virtual bool can_receive_status(status_type* s);
    virtual void receive_disable_from_status();
    virtual void receive_flailing_from_status();
    virtual void receive_panic_from_status();
    virtual void lose_panic_from_status();
    virtual void change_maturity_amount_from_status(const int amount);
};


pikmin* get_closest_buried_pikmin(
    const float x, const float y, dist* d, const bool ignore_reserved
);

#endif //ifndef PIKMIN_INCLUDED
