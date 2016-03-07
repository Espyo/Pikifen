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

const float PIKMIN_GOTO_TIMEOUT = 5.0f;

/* ----------------------------------------------------------------------------
 * The eponymous Pikmin.
 */
class pikmin : public mob {
public:
    pikmin(const float x, const float y, pikmin_type* type, const float angle, const string &vars);
    ~pikmin();
    
    virtual void draw();
    virtual float get_base_speed();
    
    pikmin_type* pik_type;
    timer hazard_timer;  //Time it has left until it drowns/chokes/etc.
    
    size_t connected_hitbox_nr;   //Number of the hitbox the Pikmin is attached to.
    float connected_hitbox_dist;  //Distance percentage from the center of the hitbox to the Pikmin's position.
    float connected_hitbox_angle; //Angle the Pikmin makes with the center of the hitbox (with the hitbox' owner at 0 degrees).
    float attack_time;            //Time left until the strike.
    
    size_t carrying_spot;        //Carrying spot reserved for it.
    
    unsigned char maturity;  //0: leaf. 1: bud. 2: flower.
    bool pluck_reserved;     //If true, someone's already coming to pluck this Pikmin. This is to let other leaders know that they should pick another one.
    
    void do_attack(mob* m, hitbox_instance* victim_hitbox_i);
    void set_connected_hitbox_info(hitbox_instance* hi_ptr, mob* mob_ptr);
    void teleport_to_connected_hitbox();
    
    static void fsm_become_buried(          mob* m, void* info1, void* info2);
    static void fsm_become_idle(            mob* m, void* info1, void* info2);
    static void fsm_begin_pluck(            mob* m, void* info1, void* info2);
    static void fsm_be_dismissed(           mob* m, void* info1, void* info2);
    static void fsm_be_grabbed_by_enemy(    mob* m, void* info1, void* info2);
    static void fsm_be_grabbed_by_friend(   mob* m, void* info1, void* info2);
    static void fsm_be_released(            mob* m, void* info1, void* info2);
    static void fsm_be_thrown(              mob* m, void* info1, void* info2);
    static void fsm_called(                 mob* m, void* info1, void* info2);
    static void fsm_chase_leader(           mob* m, void* info1, void* info2);
    static void fsm_end_pluck(              mob* m, void* info1, void* info2);
    static void fsm_fall_down_pit(          mob* m, void* info1, void* info2);
    static void fsm_forget_carriable_object(mob* m, void* info1, void* info2);
    static void fsm_get_knocked_down(       mob* m, void* info1, void* info2);
    static void fsm_go_to_carriable_object( mob* m, void* info1, void* info2);
    static void fsm_go_to_opponent(         mob* m, void* info1, void* info2);
    static void fsm_land(                   mob* m, void* info1, void* info2);
    static void fsm_land_on_mob(            mob* m, void* info1, void* info2);
    static void fsm_lose_latched_mob(       mob* m, void* info1, void* info2);
    static void fsm_prepare_to_attack(      mob* m, void* info1, void* info2);
    static void fsm_reach_carriable_object( mob* m, void* info1, void* info2);
    static void fsm_reach_dismiss_spot(     mob* m, void* info1, void* info2);
    static void fsm_rechase_opponent(       mob* m, void* info1, void* info2);
    static void fsm_stand_still(            mob* m, void* info1, void* info2);
    static void fsm_stop_being_idle(        mob* m, void* info1, void* info2);
    static void fsm_stop_carrying(          mob* m, void* info1, void* info2);
    static void fsm_stop_in_group(          mob* m, void* info1, void* info2);
    static void fsm_tick_attacking_grounded(mob* m, void* info1, void* info2);
    static void fsm_tick_grabbed_by_enemy(  mob* m, void* info1, void* info2);
    static void fsm_tick_latched(           mob* m, void* info1, void* info2);
    
};



pikmin* get_closest_buried_pikmin(const float x, const float y, dist* d, const bool ignore_reserved);



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
    PIKMIN_STATE_CARRYING,
    PIKMIN_STATE_ATTACKING_GROUNDED,
    PIKMIN_STATE_ATTACKING_LATCHED,
    PIKMIN_STATE_CELEBRATING,
    PIKMIN_STATE_GOING_TO_CARRIABLE_OBJECT,
    PIKMIN_STATE_GOING_TO_OPPONENT,
    
    N_PIKMIN_STATES
};

enum PIKMIN_ANIMATIONS {
    PIKMIN_ANIM_IDLE,
    PIKMIN_ANIM_WALK,
    PIKMIN_ANIM_THROWN,
    PIKMIN_ANIM_ATTACK,
    PIKMIN_ANIM_GRAB,
    PIKMIN_ANIM_CARRY,
    PIKMIN_ANIM_BURROWED,
    PIKMIN_ANIM_PLUCKING,
    PIKMIN_ANIM_LYING,
};

#endif //ifndef PIKMIN_INCLUDED
