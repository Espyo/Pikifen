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
private:
    size_t get_dismiss_rows(const size_t n_members);
    
public:
    leader_type* lea_type;
    
    bool auto_plucking;
    pikmin* pluck_target;
    bool queued_pluck_cancel;
    
    bool is_in_walking_anim;
    
    leader(const point &pos, leader_type* type, const float angle);
    
    virtual void draw_mob(bitmap_effect_manager* effect_manager = NULL);
    
    void signal_group_move_start();
    void signal_group_move_end();
    void dismiss();
    void start_whistling();
    void stop_whistling();
    void swap_held_pikmin(mob* new_pik);
    
    virtual bool can_receive_status(status_type* s);
    virtual void tick_class_specifics();
    
};


void change_to_next_leader(const bool forward, const bool force_success);
bool grab_closest_group_member();
void update_closest_group_member();

#endif //ifndef LEADER_INCLUDED
