/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Leader class and leader-related functions.
 */

#include "const.h"
#include "leader.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Creates a leader.
 */
leader::leader(const float x, const float y, sector* sec, leader_type* type)
    : mob(x, y, sec->floors[0].z, type, sec) {
    
    lea_type = type;
    
    holding_pikmin = NULL;
    auto_pluck_mode = false;
    auto_pluck_pikmin = NULL;
    pluck_time = -1;
    
    team = MOB_TEAM_PLAYER_1; //ToDo.
    
    party_spot_info* ps = new party_spot_info(max_pikmin_in_field, 12);
    party = new party_info(ps, x, y);
}

/* ----------------------------------------------------------------------------
 * Makes the current leader dismiss their party.
 * The party is then organized in groups, by type,
 * and is dismissed close to the leader.
 */
void dismiss() {
    leader* cur_leader_ptr = leaders[cur_leader_nr];
    
    float
    min_x = 0, min_y = 0, max_x = 0, max_y = 0, //Leftmost member coordinate, rightmost, etc.
    cx, cy, //Center of the group.
    base_angle; //They are dismissed towards this angle. This is then offset a bit depending on the Pikmin type, so they spread out.
    
    //ToDo what if there are a lot of Pikmin types?
    size_t n_party_members = cur_leader_ptr->party->members.size();
    if(n_party_members == 0) return;
    
    //First, calculate what direction the party should be dismissed to.
    if(moving_group_intensity > 0) {
        //If the leader's moving the group, they should be dismissed towards the cursor.
        base_angle = moving_group_angle + M_PI;
    } else {
        for(size_t m = 0; m < n_party_members; m++) {
            mob* member_ptr = cur_leader_ptr->party->members[m];
            
            if(member_ptr->x < min_x || m == 0) min_x = member_ptr->x;
            if(member_ptr->x > max_x || m == 0) max_x = member_ptr->x;
            if(member_ptr->y < min_y || m == 0) min_y = member_ptr->y;
            if(member_ptr->y > max_y || m == 0) max_y = member_ptr->y;
        }
        
        cx = (min_x + max_x) / 2;
        cy = (min_y + max_y) / 2;
        base_angle = atan2(cy - cur_leader_ptr->y, cx - cur_leader_ptr->x) + M_PI;
    }
    
    //Then, calculate how many Pikmin types there are in the party.
    map<pikmin_type*, float> type_dismiss_angles;
    for(size_t m = 0; m < n_party_members; m++) {
    
        if(typeid(*cur_leader_ptr->party->members[m]) == typeid(pikmin)) {
            pikmin* pikmin_ptr = dynamic_cast<pikmin*>(cur_leader_ptr->party->members[m]);
            
            type_dismiss_angles[pikmin_ptr->pik_type] = 0;
        }
    }
    
    //For each type, calculate the angle;
    size_t n_types = type_dismiss_angles.size();
    if(n_types == 1) {
        //Small hack. If there's only one Pikmin type, dismiss them directly towards the base angle.
        type_dismiss_angles.begin()->second = M_PI_4;
    } else {
        unsigned current_type_nr = 0;
        for(auto t = type_dismiss_angles.begin(); t != type_dismiss_angles.end(); t++) {
            t->second = current_type_nr * (M_PI_2 / (n_types - 1));
            current_type_nr++;
        }
    }
    
    //Now, dismiss them.
    for(size_t m = 0; m < n_party_members; m++) {
        mob* member_ptr = cur_leader_ptr->party->members[0];
        remove_from_party(member_ptr);
        
        float angle = 0;
        
        if(typeid(*member_ptr) == typeid(pikmin)) {
            pikmin* pikmin_ptr = dynamic_cast<pikmin*>(member_ptr);
            
            angle = base_angle + type_dismiss_angles[pikmin_ptr->pik_type] - M_PI_4 + M_PI;
            
            member_ptr->set_target(
                cur_leader_ptr->x + cos(angle) * DISMISS_DISTANCE,
                cur_leader_ptr->y + sin(angle) * DISMISS_DISTANCE,
                NULL,
                NULL,
                false);
        }
    }
    
    sfx_pikmin_idle.play(0, false);
    cur_leader_ptr->lea_type->sfx_dismiss.play(0, false);
    cur_leader_ptr->anim.change(LEADER_ANIM_DISMISS, true, false, false);
}

/* ----------------------------------------------------------------------------
 * Returns the distance between a leader and the center of its group.
 */
float get_leader_to_group_center_dist(mob* l) {
    return
        (l->party->party_spots->current_wheel + 1) *
        l->party->party_spots->spot_radius +
        (l->party->party_spots->current_wheel + 1) *
        PARTY_SPOT_INTERVAL;
}

/* ----------------------------------------------------------------------------
 * Makes a leader go pluck a Pikmin.
 */
void go_pluck(leader* l, pikmin* p) {
    l->auto_pluck_pikmin = p;
    l->pluck_time = -1;
    l->set_target(p->x, p->y, NULL, NULL, false);
    p->pluck_reserved = true;
}

/* ----------------------------------------------------------------------------
 * Plucks a Pikmin from the ground, if possible, and adds it to a leader's group.
 */
void pluck_pikmin(leader* new_leader, pikmin* p, leader* leader_who_plucked) {
    if(p->state != PIKMIN_STATE_BURIED) return;
    
    leader_who_plucked->pluck_time = -1;
    p->set_state(PIKMIN_STATE_IN_GROUP);
    add_to_party(new_leader, p);
    sfx_pikmin_plucked.play(0, false);
    sfx_pikmin_pluck.play(0, false);
}

/* ----------------------------------------------------------------------------
 * Makes a leader get out of auto-pluck mode.
 */
void stop_auto_pluck(leader* l) {
    if(l->auto_pluck_pikmin) l->remove_target(true);
    l->auto_pluck_mode = false;
    if(l->auto_pluck_pikmin) l->auto_pluck_pikmin->pluck_reserved = false;
    l->auto_pluck_pikmin = NULL;
    l->pluck_time = -1;
}

/* ----------------------------------------------------------------------------
 * Makes the current leader stop whistling.
 */
void stop_whistling() {
    if(!whistling) return;
    
    whistle_fade_time = WHISTLE_FADE_TIME;
    whistle_fade_radius = whistle_radius;
    
    whistling = false;
    whistle_radius = 0;
    whistle_max_hold = 0;
    
    leaders[cur_leader_nr]->lea_type->sfx_whistle.stop();
}
