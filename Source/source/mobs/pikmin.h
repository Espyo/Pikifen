/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Pikmin class and Pikmin-related functions.
 */

#ifndef PIKMIN_INCLUDED
#define PIKMIN_INCLUDED

class leader;

#include "../mob_types/pikmin_type.h"
#include "enemy.h"
#include "leader.h"
#include "onion.h"

enum PIKMIN_STATES {
    PIKMIN_STATE_IN_GROUP_CHASING,
    PIKMIN_STATE_IN_GROUP_STOPPED,
    PIKMIN_STATE_GROUP_MOVE_CHASING,
    PIKMIN_STATE_GROUP_MOVE_STOPPED,
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
    PIKMIN_STATE_SIGHING,
    PIKMIN_STATE_CARRYING,
    PIKMIN_STATE_RETURNING,
    PIKMIN_STATE_ATTACKING_GROUNDED,
    PIKMIN_STATE_ATTACKING_LATCHED,
    PIKMIN_STATE_GOING_TO_CARRIABLE_OBJECT,
    PIKMIN_STATE_GOING_TO_TOOL,
    PIKMIN_STATE_GOING_TO_OPPONENT,
    PIKMIN_STATE_DISABLED,
    PIKMIN_STATE_FLAILING,
    PIKMIN_STATE_PANICKING,
    PIKMIN_STATE_DRINKING,
    PIKMIN_STATE_CELEBRATING,
    PIKMIN_STATE_IN_GROUP_CHASING_H,
    PIKMIN_STATE_IN_GROUP_STOPPED_H,
    PIKMIN_STATE_GROUP_MOVE_CHASING_H,
    PIKMIN_STATE_GROUP_MOVE_STOPPED_H,
    PIKMIN_STATE_IDLING_H,
    PIKMIN_STATE_GRABBED_BY_LEADER_H,
    PIKMIN_STATE_THROWN_H,
    PIKMIN_STATE_GOING_TO_DISMISS_SPOT_H,
    
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
const float PIKMIN_INVULN_PERIOD = 1.0f;
const float PIKMIN_PANIC_CHASE_INTERVAL = 0.2f;


/* ----------------------------------------------------------------------------
 * The eponymous Pikmin.
 */
class pikmin : public mob {
public:
    pikmin(const point &pos, pikmin_type* type, const float angle);
    ~pikmin();
    
    pikmin_type* pik_type;
    
    //Mob that it is carrying.
    mob* carrying_mob;
    //Carrying spot reserved for it.
    size_t carrying_spot;
    //The Pikmin is considering this attack animation as having "missed".
    animation* missed_attack_ptr;
    //The Pikmin will consider the miss for this long.
    timer missed_attack_timer;
    
    //0: leaf. 1: bud. 2: flower.
    unsigned char maturity;
    //Is this Pikmin currently a seed or a sprout?
    bool is_seed_or_sprout;
    //If true, someone's already coming to pluck this Pikmin.
    //This is to let other leaders know that they should pick a different one.
    bool pluck_reserved;
    //Is this Pikmin latched on to a mob?
    bool latched;
    //Is the Pikmin holding a tool and ready to drop it on whistle?
    bool is_tool_primed_for_whistle;
    
    void force_carry(mob* m);
    bool process_attack_miss(hitbox_interaction* info);
    void increase_maturity(const int amount);
    
    virtual bool can_receive_status(status_type* s);
    virtual void draw_mob(bitmap_effect_manager* effect_manager = NULL);
    virtual float get_base_speed();
    virtual void lose_panic_from_status();
    virtual void handle_status_effect(status_type* s);
    virtual void read_script_vars(const string &vars);
    virtual void tick_class_specifics();
};


pikmin* get_closest_sprout(
    const point &pos, dist* d, const bool ignore_reserved
);

#endif //ifndef PIKMIN_INCLUDED
