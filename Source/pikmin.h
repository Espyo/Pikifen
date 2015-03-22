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
    float hazard_time_left;  // Time it has left until it drowns/chokes/etc.
    
    mob* attacking_mob;       // Enemy it's attacking.
    bool latched;             // Is the Pikmin latched onto the enemy it's attacking?
    size_t enemy_hitbox_nr;   // Number of the hitbox the Pikmin is attached to.
    float enemy_hitbox_dist;  // Distance percentage from the center of the hitbox to the Pikmin's position.
    float enemy_hitbox_angle; // Angle the Pikmin makes with the center of the hitbox (with the hitbox' owner at 0 degrees).
    float attack_time;        // Time left until the strike.
    bool being_chomped;       // Is the Pikmin stuck in the enemy's jaws?
    
    mob* wants_to_carry;     // Mob it wants to carry.
    mob* carrying_mob;       // Mob it's carrying.
    size_t carrying_spot;    // Carrying spot reserved for it.
    
    unsigned char maturity;  // 0: leaf. 1: bud. 2: flower.
    bool pluck_reserved;     // If true, someone's already coming to pluck this Pikmin. This is to let other leaders know that they should pick another one.
    
    float get_base_speed();
    
    static void become_buried(mob* m, void* info);
    static void be_plucked(mob* m, void* info);
    static void be_grabbed_by_friend(mob* m, void* info);
    static void be_dismissed(mob* m, void* info);
    static void be_thrown(mob* m, void* info);
    static void land(mob* m, void* info);
    static void go_to_task(mob* m, void* info);
    static void called(mob* m, void* info);
    static void work_on_task(mob* m, void* info);
    static void chase_leader(mob* m, void* info);
    static void stop_in_group(mob* m, void* info);
};



void drop_mob(pikmin* p);
pikmin* get_closest_buried_pikmin(const float x, const float y, float* d, const bool ignore_reserved);
void give_pikmin_to_onion(onion* o, const unsigned amount);
void start_carrying(mob* m, pikmin* np, pikmin* lp);


struct pluck_info {
    leader* new_leader;
    leader* leader_who_plucked;
    pikmin* pik;
    pluck_info(leader* new_leader, leader* leader_who_plucked, pikmin* pik) {
        this->new_leader = new_leader;
        this->leader_who_plucked = leader_who_plucked;
        this->pik = pik;
    }
};

struct dismiss_info {
    float angle;
    pikmin* pik;
    dismiss_info(const float angle, pikmin* pik) {
        this->angle = angle; this->pik = pik;
    }
};

#endif // ifndef PIKMIN_INCLUDED
