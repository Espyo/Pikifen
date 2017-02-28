/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the leader class and leader-related functions.
 */

#ifndef LEADER_INCLUDED
#define LEADER_INCLUDED

#include <vector>

#include "leader_type.h"
#include "mob.h"

class pikmin;

using namespace std;

enum LEADER_STATES {
    LEADER_STATE_IDLING,
    LEADER_STATE_ACTIVE,
    LEADER_STATE_WHISTLING,
    LEADER_STATE_HOLDING,
    LEADER_STATE_DISMISSING,
    LEADER_STATE_SPRAYING,
    LEADER_STATE_PAIN,
    LEADER_STATE_INACTIVE_PAIN,
    LEADER_STATE_KNOCKED_BACK,
    LEADER_STATE_INACTIVE_KNOCKED_BACK,
    LEADER_STATE_DYING,
    LEADER_STATE_INACTIVE_DYING,
    LEADER_STATE_IN_GROUP_CHASING,
    LEADER_STATE_IN_GROUP_STOPPED,
    LEADER_STATE_GOING_TO_PLUCK,
    LEADER_STATE_PLUCKING,
    LEADER_STATE_INACTIVE_GOING_TO_PLUCK,
    LEADER_STATE_INACTIVE_PLUCKING,
    LEADER_STATE_SLEEPING_WAITING,
    LEADER_STATE_SLEEPING_MOVING,
    LEADER_STATE_INACTIVE_SLEEPING_WAITING,
    LEADER_STATE_INACTIVE_SLEEPING_MOVING,
    //Time during which the leader is getting up.
    LEADER_STATE_WAKING_UP,
    //Time during which the leader is getting up.
    LEADER_STATE_INACTIVE_WAKING_UP,
    LEADER_STATE_HELD,
    LEADER_STATE_THROWN,
    
    N_LEADER_STATES,
    
};

enum LEADER_ANIMATIONS {
    LEADER_ANIM_IDLING,
    LEADER_ANIM_WALKING,
    LEADER_ANIM_PLUCKING,
    LEADER_ANIM_GETTING_UP,
    LEADER_ANIM_DISMISSING,
    LEADER_ANIM_THROWING,
    LEADER_ANIM_WHISTLING,
    LEADER_ANIM_LYING,
    LEADER_ANIM_PAIN,
    LEADER_ANIM_KNOCKED_DOWN,
    LEADER_ANIM_SPRAYING,
};

const float LEADER_INVULN_PERIOD = 1.5f;


/* ----------------------------------------------------------------------------
 * A leader controls Pikmin, and
 * is controlled by the player.
 */
class leader : public mob {
public:
    leader_type* lea_type;
    
    mob* holding_pikmin;
    
    pikmin* auto_pluck_pikmin;
    bool queued_pluck_cancel;
    
    bool is_in_walking_anim;
    
    leader(
        const point pos, leader_type* type,
        const float angle, const string &vars
    );
    
    virtual void draw(sprite_effect_manager* effect_manager = NULL);
    
    void signal_group_move_start();
    void signal_group_move_end();
    void dismiss();
    void swap_held_pikmin(mob* new_pik);
    
    virtual bool can_receive_status(status_type* s);
    
};


bool grab_closest_group_member();
float get_leader_to_group_center_dist(mob* l);
void switch_to_leader(leader* new_leader_ptr);
void update_closest_group_member();

#endif //ifndef LEADER_INCLUDED
