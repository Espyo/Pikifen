/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
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

/* ----------------------------------------------------------------------------
 * The eponymous Pikmin.
 */
class pikmin : public mob {
public:
    pikmin(const float x, const float y, pikmin_type* type, const float angle, const string &vars);
    ~pikmin();
    
    pikmin_type* pik_type;
    float hazard_time_left;  //Time it has left until it drowns/chokes/etc.
    
    size_t enemy_hitbox_nr;   //Number of the hitbox the Pikmin is attached to.
    float enemy_hitbox_dist;  //Distance percentage from the center of the hitbox to the Pikmin's position.
    float enemy_hitbox_angle; //Angle the Pikmin makes with the center of the hitbox (with the hitbox' owner at 0 degrees).
    float attack_time;        //Time left until the strike.
    bool being_chomped;       //Is the Pikmin stuck in the enemy's jaws?
    
    bool grabbing_carriable_mob; //Is it actually grasping the carriable mob, or just trying to reach it?
    size_t carrying_spot;        //Carrying spot reserved for it.
    bool is_idle;                //Is the Pikmin idling?
    
    unsigned char maturity;  //0: leaf. 1: bud. 2: flower.
    bool pluck_reserved;     //If true, someone's already coming to pluck this Pikmin. This is to let other leaders know that they should pick another one.
    
    float get_base_speed();
    
    static void become_buried(          mob* m, void* info1, void* info2);
    static void become_idle(            mob* m, void* info1, void* info2);
    static void be_plucked(             mob* m, void* info1, void* info2);
    static void be_grabbed_by_friend(   mob* m, void* info1, void* info2);
    static void be_grabbed_by_enemy(    mob* m, void* info1, void* info2);
    static void be_dismissed(           mob* m, void* info1, void* info2);
    static void be_thrown(              mob* m, void* info1, void* info2);
    static void be_released(            mob* m, void* info1, void* info2);
    static void land(                   mob* m, void* info1, void* info2);
    static void go_to_task(             mob* m, void* info1, void* info2);
    static void called(                 mob* m, void* info1, void* info2);
    static void work_on_task(           mob* m, void* info1, void* info2);
    static void chase_leader(           mob* m, void* info1, void* info2);
    static void stop_being_idle(        mob* m, void* info1, void* info2);
    static void stop_in_group(          mob* m, void* info1, void* info2);
    static void reach_dismiss_spot(     mob* m, void* info1, void* info2);
    static void go_to_carriable_object( mob* m, void* info1, void* info2);
    static void grab_carriable_object(  mob* m, void* info1, void* info2);
    static void finish_carrying(        mob* m, void* info1, void* info2);
    static void forget_about_carrying(  mob* m, void* info1, void* info2);
    static void go_to_opponent(         mob* m, void* info1, void* info2);
    static void rechase_opponent(       mob* m, void* info1, void* info2);
    static void attack(                 mob* m, void* info1, void* info2);
    static void land_on_mob(            mob* m, void* info1, void* info2);
    static void tick_latched(           mob* m, void* info1, void* info2);
    static void tick_attacking_grounded(mob* m, void* info1, void* info2);
    static void tick_grabbed_by_enemy(  mob* m, void* info1, void* info2);
    static void prepare_to_attack(      mob* m, void* info1, void* info2);
    static void get_knocked_down(        mob* m, void* info1, void* info2);
    
};



pikmin* get_closest_buried_pikmin(const float x, const float y, float* d, const bool ignore_reserved);
void give_pikmin_to_onion(onion* o, const unsigned amount);
void start_moving_carried_object(mob* m, pikmin* np, pikmin* lp);
void swap_pikmin(mob* new_pik);


#endif //ifndef PIKMIN_INCLUDED
