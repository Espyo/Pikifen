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
 * Creates a leader.
 */
leader::leader(const float x, const float y, leader_type* type, const float angle, const string &vars) :
    mob(x, y, type, angle, vars),
    lea_type(type),
    holding_pikmin(nullptr),
    auto_pluck_pikmin(nullptr),
    is_in_walking_anim(false) {
    
    team = MOB_TEAM_PLAYER_1; //TODO.
    invuln_period = timer(LEADER_INVULN_PERIOD);
    
    party_spot_info* ps = new party_spot_info(max_pikmin_in_field, 12);
    party = new party_info(ps, x, y);
}


/* ----------------------------------------------------------------------------
 * Makes the current leader dismiss their party.
 * The party is then organized in groups, by type,
 * and is dismissed close to the leader.
 */
void dismiss() {
    float base_angle; //They are dismissed towards this angle. This is then offset a bit depending on the Pikmin type, so they spread out.
    
    //TODO what if there are a lot of Pikmin types?
    size_t n_party_members = cur_leader_ptr->party->members.size();
    if(n_party_members == 0) return;
    
    //First, calculate what direction the party should be dismissed to.
    if(group_move_intensity > 0) {
        //If the leader's moving the group, they should be dismissed towards the cursor.
        base_angle = group_move_angle + M_PI;
    } else {
        float min_x = 0, min_y = 0, max_x = 0, max_y = 0; //Leftmost member coordinate, rightmost, etc.
        
        for(size_t m = 0; m < n_party_members; ++m) {
            mob* member_ptr = cur_leader_ptr->party->members[m];
            
            if(member_ptr->x < min_x || m == 0) min_x = member_ptr->x;
            if(member_ptr->x > max_x || m == 0) max_x = member_ptr->x;
            if(member_ptr->y < min_y || m == 0) min_y = member_ptr->y;
            if(member_ptr->y > max_y || m == 0) max_y = member_ptr->y;
        }
        
        base_angle =
            atan2(
                ((min_y + max_y) / 2) - cur_leader_ptr->y,
                ((min_x + max_x) / 2) - cur_leader_ptr->x
            ) + M_PI;
    }
    
    //Then, calculate how many Pikmin types there are in the party.
    map<pikmin_type*, float> type_dismiss_angles;
    for(size_t m = 0; m < n_party_members; ++m) {
    
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
        for(auto t = type_dismiss_angles.begin(); t != type_dismiss_angles.end(); ++t) {
            t->second = current_type_nr * (M_PI_2 / (n_types - 1));
            current_type_nr++;
        }
    }
    
    //Now, dismiss them.
    for(size_t m = 0; m < n_party_members; ++m) {
        mob* member_ptr = cur_leader_ptr->party->members[0];
        remove_from_party(member_ptr);
        
        float angle = 0;
        
        if(typeid(*member_ptr) == typeid(pikmin)) {
            pikmin* pikmin_ptr = dynamic_cast<pikmin*>(member_ptr);
            
            angle = base_angle + type_dismiss_angles[pikmin_ptr->pik_type] - M_PI_4 + M_PI;
            pikmin_ptr->fsm.run_event(MOB_EVENT_DISMISSED, (void*) &angle);
            
        }
    }
    
    sfx_pikmin_idle.play(0, false);
    cur_leader_ptr->lea_type->sfx_dismiss.play(0, false);
    cur_leader_ptr->set_animation(LEADER_ANIM_DISMISS);
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


void swap_pikmin(mob* new_pik) {
    leader* lea = cur_leader_ptr;
    if(lea->holding_pikmin) {
        lea->holding_pikmin->fsm.run_event(MOB_EVENT_RELEASED);
    }
    lea->holding_pikmin = new_pik;
    new_pik->fsm.run_event(MOB_EVENT_GRABBED_BY_FRIEND);
    
    sfx_switch_pikmin.play(0, false);
}

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

void leader::draw() {
    mob::draw();
    
    frame* f_ptr = anim.get_frame();
    float draw_x, draw_y;
    float draw_w, draw_h;
    get_sprite_center(this, f_ptr, &draw_x, &draw_y);
    get_sprite_dimensions(this, f_ptr, &draw_w, &draw_h);
    
    if(invuln_period.time_left > 0.0f) {
        unsigned char anim_part = invuln_period.get_ratio_left() * LEADER_ZAP_ANIM_PARTS;
        float zap_x[4], zap_y[4];
        for(unsigned char p = 0; p < 4; ++p) {
            if(anim_part % 2 == 0) {
                zap_x[p] = draw_x + draw_w * (deterministic_random(anim_part * 3 + p) - 0.5);
                zap_y[p] = draw_y + draw_h * 0.5 * ((p % 2 == 0) ? 1 : -1);
            } else {
                zap_x[p] = draw_x + draw_w * 0.5 * ((p % 2 == 0) ? 1 : -1);
                zap_y[p] = draw_y + draw_h * (deterministic_random(anim_part * 3 + p) - 0.5);
            }
        }
        
        static const ALLEGRO_COLOR LEADER_ZAP_COLOR = al_map_rgba(128, 255, 255, 128);
        
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
 * Signals the party members that the group move mode stopped.
 */
void leader::signal_group_move_end() {
    for(size_t m = 0; m < party->members.size(); ++m) {
        party->members[m]->fsm.run_event(MOB_EVENT_GROUP_MOVE_ENDED);
    }
}


/* ----------------------------------------------------------------------------
 * Signals the party members that the group move mode started.
 */
void leader::signal_group_move_start() {
    for(size_t m = 0; m < party->members.size(); ++m) {
        party->members[m]->fsm.run_event(MOB_EVENT_GROUP_MOVE_STARTED);
    }
}
