/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Leader class and leader-related functions.
 */

#include "../const.h"
#include "../functions.h"
#include "leader.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a leader mob.
 */
leader::leader(
    const float x, const float y, leader_type* type,
    const float angle, const string &vars
) :
    mob(x, y, type, angle, vars),
    lea_type(type),
    holding_pikmin(nullptr),
    auto_pluck_pikmin(nullptr),
    is_in_walking_anim(false) {
    
    team = MOB_TEAM_PLAYER_1; //TODO.
    invuln_period = timer(LEADER_INVULN_PERIOD);
    
    group_spot_info* ps = new group_spot_info(max_pikmin_in_field, 12);
    group = new group_info(ps, x, y);
}


/* ----------------------------------------------------------------------------
 * Makes a leader dismiss their group.
 * The group is then organized in groups, by type,
 * and is dismissed close to the leader.
 */
void leader::dismiss() {
    //TODO what if there are a lot of Pikmin types?
    size_t n_group_members = group->members.size();
    if(n_group_members == 0) return;
    
    //They are dismissed towards this angle.
    //This is then offset a bit depending on the Pikmin type,
    //so they spread out.
    float base_angle;
    
    //First, calculate what direction the group should be dismissed to.
    if(group_move_intensity > 0) {
        //If the leader's moving the group,
        //they should be dismissed towards the cursor.
        base_angle = group_move_angle + M_PI;
    } else {
        //Leftmost member coordinate, rightmost, etc.
        float min_x = 0, min_y = 0, max_x = 0, max_y = 0;
        
        for(size_t m = 0; m < n_group_members; ++m) {
            mob* member_ptr = group->members[m];
            
            if(member_ptr->x < min_x || m == 0) min_x = member_ptr->x;
            if(member_ptr->x > max_x || m == 0) max_x = member_ptr->x;
            if(member_ptr->y < min_y || m == 0) min_y = member_ptr->y;
            if(member_ptr->y > max_y || m == 0) max_y = member_ptr->y;
        }
        
        base_angle =
            atan2(
                ((min_y + max_y) / 2) - y,
                ((min_x + max_x) / 2) - x
            ) + M_PI;
    }
    
    //Then, calculate how many Pikmin types there are in the group.
    map<pikmin_type*, float> type_angle_map;
    for(size_t m = 0; m < n_group_members; ++m) {
    
        if(typeid(*group->members[m]) == typeid(pikmin)) {
            pikmin* pikmin_ptr = dynamic_cast<pikmin*>(group->members[m]);
            
            type_angle_map[pikmin_ptr->pik_type] = 0;
        }
    }
    
    //For each type, calculate the angle;
    size_t n_types = type_angle_map.size();
    if(n_types == 1) {
        //Small hack. If there's only one Pikmin type,
        //dismiss them directly towards the base angle.
        type_angle_map.begin()->second = M_PI_4;
        
    } else {
        unsigned current_type_nr = 0;
        for(auto t = type_angle_map.begin(); t != type_angle_map.end(); ++t) {
            t->second = current_type_nr * (M_PI_2 / (n_types - 1));
            current_type_nr++;
        }
    }
    
    //Now, dismiss them.
    for(size_t m = 0; m < n_group_members; ++m) {
        mob* member_ptr = group->members[0];
        remove_from_group(member_ptr);
        
        float angle = 0;
        
        if(typeid(*member_ptr) == typeid(pikmin)) {
            angle =
                base_angle +
                type_angle_map[((pikmin*) member_ptr)->pik_type] -
                M_PI_4 + M_PI;
        }
        
        float x = cur_leader_ptr->x + cos(angle) * DISMISS_DISTANCE;
        float y = cur_leader_ptr->y + sin(angle) * DISMISS_DISTANCE;
        
        member_ptr->fsm.run_event(MOB_EVENT_DISMISSED, (void*) &x, (void*) &y);
    }
    
    lea_type->sfx_dismiss.play(0, false);
    set_animation(LEADER_ANIM_DISMISS);
}


/* ----------------------------------------------------------------------------
 * Swaps out the currently held Pikmin for a different one.
 */
void leader::swap_held_pikmin(mob* new_pik) {
    if(holding_pikmin) {
        holding_pikmin->fsm.run_event(MOB_EVENT_RELEASED);
    }
    holding_pikmin = new_pik;
    new_pik->fsm.run_event(MOB_EVENT_GRABBED_BY_FRIEND);
    
    sfx_switch_pikmin.play(0, false);
}


/* ----------------------------------------------------------------------------
 * Draw a leader mob.
 */
void leader::draw() {
    mob::draw();
    
    frame* f_ptr = anim.get_frame();
    float draw_x, draw_y;
    float draw_w, draw_h;
    get_sprite_center(this, f_ptr, &draw_x, &draw_y);
    get_sprite_dimensions(this, f_ptr, &draw_w, &draw_h);
    
    if(invuln_period.time_left > 0.0f) {
        unsigned char anim_part =
            invuln_period.get_ratio_left() * LEADER_ZAP_ANIM_PARTS;
        float zap_x[4], zap_y[4];
        for(unsigned char p = 0; p < 4; ++p) {
            if(anim_part % 2 == 0) {
                zap_x[p] =
                    draw_x + draw_w *
                    (deterministic_random(anim_part * 3 + p) - 0.5);
                zap_y[p] =
                    draw_y + draw_h * 0.5 * ((p % 2 == 0) ? 1 : -1);
            } else {
                zap_x[p] =
                    draw_x + draw_w * 0.5 * ((p % 2 == 0) ? 1 : -1);
                zap_y[p] =
                    draw_y + draw_h *
                    (deterministic_random(anim_part * 3 + p) - 0.5);
            }
        }
        
        static const ALLEGRO_COLOR LEADER_ZAP_COLOR =
            al_map_rgba(128, 255, 255, 128);
            
        al_draw_line(
            zap_x[0], zap_y[0], zap_x[1], zap_y[1],
            LEADER_ZAP_COLOR,
            2.0f
        );
        al_draw_line(
            zap_x[1], zap_y[1], zap_x[2], zap_y[2],
            LEADER_ZAP_COLOR,
            2.0f
        );
        al_draw_line(
            zap_x[2], zap_y[2], zap_x[3], zap_y[3],
            LEADER_ZAP_COLOR,
            2.0f
        );
        
    }
}


/* ----------------------------------------------------------------------------
 * Signals the group members that the group move mode stopped.
 */
void leader::signal_group_move_end() {
    for(size_t m = 0; m < group->members.size(); ++m) {
        group->members[m]->fsm.run_event(MOB_EVENT_GROUP_MOVE_ENDED);
    }
}


/* ----------------------------------------------------------------------------
 * Signals the group members that the group move mode started.
 */
void leader::signal_group_move_start() {
    for(size_t m = 0; m < group->members.size(); ++m) {
        group->members[m]->fsm.run_event(MOB_EVENT_GROUP_MOVE_STARTED);
    }
}


/* ----------------------------------------------------------------------------
 * Returns the distance between a leader and the center of its group.
 */
float get_leader_to_group_center_dist(mob* l) {
    return
        (l->group->group_spots->current_wheel + 1) *
        l->group->group_spots->spot_radius +
        (l->group->group_spots->current_wheel + 1) *
        GROUP_SPOT_INTERVAL;
}


/* ----------------------------------------------------------------------------
 * Switch active leader.
 */
void switch_to_leader(leader* new_leader_ptr) {

    cur_leader_ptr->fsm.run_event(LEADER_EVENT_UNFOCUSED);
    
    size_t new_leader_nr = cur_leader_nr;
    for(size_t l = 0; l < leaders.size(); ++l) {
        if(leaders[l] == new_leader_ptr) {
            new_leader_nr = l;
            break;
        }
    }
    
    cur_leader_ptr = new_leader_ptr;
    cur_leader_nr = new_leader_nr;
    
    new_leader_ptr->lea_type->sfx_name_call.play(0, false);
    
}


/* ----------------------------------------------------------------------------
 * Returns whether or not a leader can receive a given status effect.
 */
bool leader::can_receive_status(status_type* s) {
    return s->affects & STATUS_AFFECTS_LEADERS;
}
