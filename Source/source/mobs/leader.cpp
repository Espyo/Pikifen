/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
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
#include "../drawing.h"
#include "leader.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a leader mob.
 */
leader::leader(
    const point pos, leader_type* type,
    const float angle, const string &vars
) :
    mob(pos, type, angle, vars),
    lea_type(type),
    holding_pikmin(nullptr),
    auto_pluck_pikmin(nullptr),
    queued_pluck_cancel(false),
    is_in_walking_anim(false) {
    
    team = MOB_TEAM_PLAYER_1; //TODO.
    invuln_period = timer(LEADER_INVULN_PERIOD);
    
    group = new group_info(this);
    subgroup_type_ptr =
        subgroup_types.get_type(SUBGROUP_TYPE_CATEGORY_LEADER);
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
        point min_coords, max_coords;
        
        for(size_t m = 0; m < n_group_members; ++m) {
            mob* member_ptr = group->members[m];
            
            if(member_ptr->pos.x < min_coords.x || m == 0)
                min_coords.x = member_ptr->pos.x;
            if(member_ptr->pos.x > max_coords.x || m == 0)
                max_coords.x = member_ptr->pos.x;
            if(member_ptr->pos.y < min_coords.y || m == 0)
                min_coords.y = member_ptr->pos.y;
            if(member_ptr->pos.y > max_coords.y || m == 0)
                max_coords.y = member_ptr->pos.y;
        }
        
        point group_center(
            (min_coords.x + max_coords.x) / 2,
            (min_coords.y + max_coords.y) / 2
        );
        base_angle = get_angle(pos, group_center) + M_PI;
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
        
        point destination(
            pos.x + cos(angle) * DISMISS_DISTANCE,
            pos.y + sin(angle) * DISMISS_DISTANCE
        );
        
        member_ptr->fsm.run_event(MOB_EVENT_DISMISSED, (void*) &destination);
    }
    
    lea_type->sfx_dismiss.play(0, false);
    set_animation(LEADER_ANIM_DISMISSING);
}


/* ----------------------------------------------------------------------------
 * Swaps out the currently held Pikmin for a different one.
 */
void leader::swap_held_pikmin(mob* new_pik) {
    if(!holding_pikmin) return;
    
    mob_event* old_pik_ev = holding_pikmin->fsm.get_event(MOB_EVENT_RELEASED);
    mob_event* new_pik_ev = new_pik->fsm.get_event(MOB_EVENT_GRABBED_BY_FRIEND);
    
    group->sort(new_pik->subgroup_type_ptr);
    
    if(!old_pik_ev || !new_pik_ev) return;
    
    old_pik_ev->run(holding_pikmin);
    new_pik_ev->run(new_pik);
    holding_pikmin = new_pik;
    
    sfx_switch_pikmin.play(0, false);
}


/* ----------------------------------------------------------------------------
 * Draw a leader mob.
 */
void leader::draw(sprite_effect_manager* effect_manager) {
    sprite_effect_manager effects;
    
    mob::draw(&effects);
    
    sprite* s_ptr = anim.get_cur_sprite();
    point draw_pos = get_sprite_center(this, s_ptr);
    point draw_size = get_sprite_dimensions(this, s_ptr);
    
    if(invuln_period.time_left > 0.0f) {
        sprite* spark_s = spark_animation.instance.get_cur_sprite();
        if(spark_s && spark_s->bitmap) {
            draw_sprite(
                spark_s->bitmap, draw_pos,
                draw_size
            );
        }
    }
    
    draw_status_effect_bmp(this, &effects);
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


/* ----------------------------------------------------------------------------
 * Ticks leader-related logic for this frame.
 */
void leader::tick_class_specifics() {
    if(group && group->members.size()) {
    
        bool must_reassign_spots = false;
        
        bool is_moving_group =
            (group_move_intensity && cur_leader_ptr == this);
            
        if(
            dist(group->get_average_member_pos(), pos) >
            GROUP_SHUFFLE_DIST + (group->radius + type->radius)
        ) {
            if(!group->follow_mode) {
                must_reassign_spots = true;
            }
            group->follow_mode = true;
            
        } else if(is_moving_group || holding_pikmin) {
            group->follow_mode = true;
            
        } else {
            group->follow_mode = false;
            
        }
        
        group->transform = identity_transform;
        
        if(group->follow_mode) {
            //Follow mode. Try to stay on the leader's back.
            
            if(is_moving_group) {
            
                point move_anchor_offset =
                    rotate_point(
                        point(
                            -(type->radius + GROUP_SPOT_INTERVAL * 2),
                            0
                        ), group_move_angle + M_PI
                    );
                group->anchor = pos + move_anchor_offset;
                
                float intensity_dist = cursor_max_dist * group_move_intensity;
                al_scale_transform(
                    &group->transform,
                    intensity_dist / (group->radius * 2),
                    1 - (GROUP_MOVE_VERTICAL_SCALE * group_move_intensity)
                );
                al_rotate_transform(&group->transform, group_move_angle + M_PI);
                
            } else {
            
                point leader_back_offset =
                    rotate_point(
                        point(
                            -(type->radius + GROUP_SPOT_INTERVAL * 2),
                            0
                        ), angle
                    );
                group->anchor = pos + leader_back_offset;
                
                al_rotate_transform(&group->transform, angle);
                
            }
            
            if(must_reassign_spots) group->reassing_spots();
            
        } else {
            //Shuffle mode. Keep formation, but shuffle with the leader,
            //if needed.
            point mov;
            move_point(
                group->anchor - point(group->radius, 0),
                pos,
                type->move_speed,
                group->radius + type->radius + GROUP_SPOT_INTERVAL * 2,
                &mov, NULL, NULL, delta_t
            );
            group->anchor += mov * delta_t;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Makes the current leader grab the closest group member of the standby type.
 * Returns true on success, false on failure.
 */
bool grab_closest_group_member() {
    if(closest_group_member) {
        mob_event* grabbed_ev =
            closest_group_member->fsm.get_event(
                MOB_EVENT_GRABBED_BY_FRIEND
            );
        mob_event* grabber_ev =
            cur_leader_ptr->fsm.get_event(
                LEADER_EVENT_HOLDING
            );
        if(grabber_ev && grabbed_ev) {
            cur_leader_ptr->fsm.run_event(
                LEADER_EVENT_HOLDING,
                (void*) closest_group_member
            );
            grabbed_ev->run(
                closest_group_member,
                (void*) closest_group_member
            );
            return true;
        }
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * Updates the variable that indicates what the closest
 * group member of the standby subgroup is.
 * In the case all candidate members are out of reach,
 * this gets set to the closest. Otherwise, it gets set to the closest
 * and more mature one.
 * NULL if there is no member of that subgroup available.
 */
void update_closest_group_member() {
    //Closest members so far for each maturity.
    dist closest_dists[N_MATURITIES];
    mob* closest_ptrs[N_MATURITIES];
    for(unsigned char m = 0; m < N_MATURITIES; ++m) {
        closest_ptrs[m] = NULL;
    }
    
    closest_group_member = NULL;
    
    //Fetch the closest, for each maturity.
    size_t n_members = cur_leader_ptr->group->members.size();
    for(size_t m = 0; m < n_members; ++m) {
    
        mob* member_ptr = cur_leader_ptr->group->members[m];
        if(
            member_ptr->subgroup_type_ptr !=
            cur_leader_ptr->group->cur_standby_type
        ) {
            continue;
        }
        
        unsigned char maturity = 0;
        if(member_ptr->type->category->id == MOB_CATEGORY_PIKMIN) {
            maturity = ((pikmin*) member_ptr)->maturity;
        }
        
        dist d(cur_leader_ptr->pos, member_ptr->pos);
        
        if(!closest_ptrs[maturity] || d < closest_dists[maturity]) {
            closest_dists[maturity] = d;
            closest_ptrs[maturity] = member_ptr;
        }
    }
    
    //Now, try to get the one with the highest maturity within reach.
    dist closest_dist;
    for(unsigned char m = 0; m < N_MATURITIES; ++m) {
        if(!closest_ptrs[2 - m]) continue;
        if(closest_dists[2 - m] > pikmin_grab_range) continue;
        closest_group_member = closest_ptrs[2 - m];
        closest_dist = closest_dists[2 - m];
        break;
    }
    
    if(!closest_group_member) {
        //Couldn't find any within reach? Then just set it to the closest one.
        //Maturity is irrelevant for this case.
        for(unsigned char m = 0; m < N_MATURITIES; ++m) {
            if(!closest_ptrs[m]) continue;
            
            if(!closest_group_member || closest_dists[m] < closest_dist) {
                closest_group_member = closest_ptrs[m];
                closest_dist = closest_dists[m];
            }
        }
    }
    
    closest_group_member_distant = closest_dist > pikmin_grab_range;
}
