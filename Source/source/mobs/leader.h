/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the leader class and leader-related functions.
 */

#ifndef LEADER_INCLUDED
#define LEADER_INCLUDED

#include <vector>

#include "../mob_types/leader_type.h"
#include "mob.h"

class pikmin;

using namespace std;


/* ----------------------------------------------------------------------------
 * A leader controls Pikmin, and
 * is controlled by the player.
 */
class leader : public mob {
public:
    //What type of leader it is.
    leader_type* lea_type;
    
    //Is it currently auto-plucking?
    bool auto_plucking;
    //Pikmin it wants to pluck.
    pikmin* pluck_target;
    //Has the player asked for the auto-plucking to stop?
    bool queued_pluck_cancel;
    //Is the leader currently in the walking animation?
    bool is_in_walking_anim;
    
    //Dismiss current group.
    void dismiss();
    //Signal to every group member that swarm mode started.
    void signal_swarm_start();
    //Signal to every group member that swarm mode ended.
    void signal_swarm_end();
    //Start whistling.
    void start_whistling();
    //Stop whistling.
    void stop_whistling();
    //Change the current held Pikmin for another.
    void swap_held_pikmin(mob* new_pik);
    
    //Constructor.
    leader(const point &pos, leader_type* type, const float angle);
    
    //Can the mob currently receive the specified status effect?
    virtual bool can_receive_status(status_type* s);
    //Mob drawing routine.
    virtual void draw_mob(bitmap_effect_manager* effect_manager = NULL);
    
protected:
    //Tick class-specific logic.
    virtual void tick_class_specifics();
    
private:
    //Returns how many rows are needed for all members' dismissal.
    size_t get_dismiss_rows(const size_t n_members);
};


void change_to_next_leader(const bool forward, const bool force_success);
bool grab_closest_group_member();
void update_closest_group_member();

#endif //ifndef LEADER_INCLUDED
