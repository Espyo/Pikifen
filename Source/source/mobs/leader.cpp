/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Leader class and leader-related functions.
 */

#include <algorithm>

#include "../const.h"
#include "../functions.h"
#include "../drawing.h"
#include "leader.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a leader mob.
 */
leader::leader(
    const point &pos, leader_type* type,
    const float angle, const string &vars
) :
    mob(pos, type, angle, vars),
    lea_type(type),
    auto_plucking(false),
    pluck_target(nullptr),
    queued_pluck_cancel(false),
    is_in_walking_anim(false) {
    
    team = MOB_TEAM_PLAYER_1; //TODO.
    invuln_period = timer(LEADER_INVULN_PERIOD);
    
    group = new group_info(this);
    subgroup_type_ptr =
        subgroup_types.get_type(SUBGROUP_TYPE_CATEGORY_LEADER);
}


//Members cannot go past this range from the angle of dismissal.
const float DISMISS_ANGLE_RANGE = TAU / 2;
//Multiply the space members take up by this. Lower = more compact subgroups.
const float DISMISS_MEMBER_SIZE_MULTIPLIER = 0.75f;
//Dismissed groups must have this much distance between them/the leader.
const float DISMISS_SUBGROUP_DISTANCE = 48.0f;
/* ----------------------------------------------------------------------------
 * Makes a leader dismiss their group.
 * The group is then organized in groups, by type,
 * and is dismissed close to the leader.
 */
void leader::dismiss() {
    size_t n_group_members = group->members.size();
    if(n_group_members == 0) return;
    
    //They are dismissed towards this angle.
    //This is then offset a bit for each subgroup, depending on a few factors.
    float base_angle;
    
    //First, calculate what direction the group should be dismissed to.
    if(group_move_magnitude > 0) {
        //If the leader's moving the group,
        //they should be dismissed in that direction.
        base_angle = group_move_angle;
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
        base_angle = get_angle(pos, group_center);
    }
    
    
    struct subgroup_dismiss_info {
        subgroup_type* type;
        mob_type* m_type;
        float radius;
        vector<mob*> members;
        size_t row;
        point center;
    };
    vector<subgroup_dismiss_info> subgroups_info;
    
    //Go through all subgroups and populate the vector of data.
    subgroup_type* first_type = subgroup_types.get_first_type();
    subgroup_type* cur_type = first_type;
    
    do {
    
        if(
            cur_type !=
            subgroup_types.get_type(SUBGROUP_TYPE_CATEGORY_LEADER)
        ) {
        
            bool subgroup_exists = false;
            
            for(size_t m = 0; m < n_group_members; ++m) {
                mob* m_ptr = group->members[m];
                if(m_ptr->subgroup_type_ptr != cur_type) continue;
                
                if(!subgroup_exists) {
                    subgroups_info.push_back(subgroup_dismiss_info());
                    subgroups_info.back().m_type = m_ptr->type;
                    subgroup_exists = true;
                }
                
                subgroups_info.back().members.push_back(m_ptr);
            }
            
        }
        
        cur_type = subgroup_types.get_next_type(cur_type);
        
    } while(cur_type != first_type);
    
    //Let's figure out each subgroup's size.
    //Subgroups will be made by placing the members in
    //rows of circles surrounding a central point.
    //The first row is just one spot.
    //The second row is 6 spots around that one.
    //The third is 12 spots around those 6.
    //And so on. Each row fits an additional 6.
    for(size_t s = 0; s < subgroups_info.size(); ++s) {
        size_t n_rows = get_dismiss_rows(subgroups_info[s].members.size());
        
        //Since each row loops all around,
        //it appears to the left and right of the center.
        //So count each one twice. Except for the central one.
        subgroups_info[s].radius =
            standard_pikmin_radius +
            standard_pikmin_radius * 2 *
            DISMISS_MEMBER_SIZE_MULTIPLIER * (n_rows - 1);
    }
    
    //We'll need to place the subgroups inside arched rows.
    //Like stripes on a rainbow.
    //For each row, we must fit as many Pikmin subgroups as possible.
    //Each row can have a different thickness,
    //based on the size of the subgroups within.
    //Starts off on the row closest to the leader.
    //We place the first subgroup, then some padding, then the next group,
    //etc. For every subgroup we place, we must update the thickness.
    struct row_info {
        vector<size_t> subgroups;
        float dist_between_center;
        float thickness;
        float angle_occupation; //How much is taken up by Pikmin and padding.
        
        row_info() {
            dist_between_center = 0;
            thickness = 0;
            angle_occupation = 0;
        }
    };
    
    bool done = false;
    vector<row_info> rows;
    row_info cur_row;
    cur_row.dist_between_center = DISMISS_SUBGROUP_DISTANCE;
    size_t cur_row_nr = 0;
    size_t cur_subgroup_nr = 0;
    
    while(!done && !subgroups_info.empty()) {
        float new_thickness =
            max(cur_row.thickness, subgroups_info[cur_subgroup_nr].radius * 2);
            
        float new_angle_occupation = 0;
        for(size_t s = 0; s < cur_row.subgroups.size(); ++s) {
            new_angle_occupation +=
                linear_dist_to_angular(
                    subgroups_info[cur_row.subgroups[s]].radius * 2.0,
                    cur_row.dist_between_center +
                    cur_row.thickness / 2.0f
                );
            if(s < cur_row.subgroups.size() - 1) {
                new_angle_occupation +=
                    linear_dist_to_angular(
                        DISMISS_SUBGROUP_DISTANCE,
                        cur_row.dist_between_center +
                        cur_row.thickness / 2.0f
                    );
            }
        }
        if(!cur_row.subgroups.empty()) {
            new_angle_occupation +=
                linear_dist_to_angular(
                    DISMISS_SUBGROUP_DISTANCE,
                    cur_row.dist_between_center +
                    new_thickness / 2.0f
                );
        }
        new_angle_occupation +=
            linear_dist_to_angular(
                subgroups_info[cur_subgroup_nr].radius * 2.0,
                cur_row.dist_between_center +
                new_thickness / 2.0f
            );
            
        //Will this group fit?
        if(new_angle_occupation <= DISMISS_ANGLE_RANGE) {
            //This subgroup still fits. Next!
            cur_row.thickness = new_thickness;
            cur_row.angle_occupation = new_angle_occupation;
            
            cur_row.subgroups.push_back(cur_subgroup_nr);
            subgroups_info[cur_subgroup_nr].row = cur_row_nr;
            cur_subgroup_nr++;
        }
        
        if(
            new_angle_occupation > DISMISS_ANGLE_RANGE ||
            cur_subgroup_nr == subgroups_info.size()
        ) {
            //This subgroup doesn't fit. It'll have to be put in the next row.
            //Or this is the last subgroup, and the row needs to be committed.
            
            rows.push_back(cur_row);
            cur_row_nr++;
            cur_row.dist_between_center +=
                cur_row.thickness + DISMISS_SUBGROUP_DISTANCE;
            cur_row.subgroups.clear();
            cur_row.thickness = 0;
            cur_row.angle_occupation = 0;
        }
        
        if(cur_subgroup_nr == subgroups_info.size()) done = true;
    }
    
    //Now that we know which subgroups go into which row,
    //simply decide the positioning.
    for(size_t r = 0; r < rows.size(); ++r) {
        float start_angle = -(rows[r].angle_occupation / 2.0f);
        float cur_angle = start_angle;
        
        for(size_t s = 0; s < rows[r].subgroups.size(); ++s) {
            size_t s_nr = rows[r].subgroups[s];
            float subgroup_angle = cur_angle;
            
            cur_angle +=
                linear_dist_to_angular(
                    subgroups_info[s_nr].radius * 2.0,
                    rows[r].dist_between_center + rows[r].thickness / 2.0
                );
            if(s < rows[r].subgroups.size() - 1) {
                cur_angle +=
                    linear_dist_to_angular(
                        DISMISS_SUBGROUP_DISTANCE,
                        rows[r].dist_between_center + rows[r].thickness / 2.0
                    );
            }
            
            //Center the subgroup's angle.
            subgroup_angle +=
                linear_dist_to_angular(
                    subgroups_info[s_nr].radius,
                    rows[r].dist_between_center + rows[r].thickness / 2.0
                );
                
            subgroups_info[s_nr].center =
                angle_to_coordinates(
                    base_angle + subgroup_angle,
                    rows[r].dist_between_center + rows[r].thickness / 2.0f
                );
                
        }
    }
    
    //Now, dismiss!
    for(size_t s = 0; s < subgroups_info.size(); ++s) {
        size_t cur_row_nr = 0;
        size_t cur_row_spot_nr = 0;
        size_t cur_row_spots = 1;
        
        for(size_t m = 0; m < subgroups_info[s].members.size(); ++m) {
        
            point destination;
            
            if(cur_row_nr == 0) {
                destination = subgroups_info[s].center;
            } else {
                float member_angle =
                    ((float) cur_row_spot_nr / cur_row_spots) * TAU;
                destination =
                    subgroups_info[s].center +
                    angle_to_coordinates(
                        member_angle,
                        cur_row_nr * standard_pikmin_radius * 2 *
                        DISMISS_MEMBER_SIZE_MULTIPLIER
                    );
            }
            
            destination +=
                point(
                    randomf(-5.0, 5.0),
                    randomf(-5.0, 5.0)
                );
                
            cur_row_spot_nr++;
            if(cur_row_spot_nr == cur_row_spots) {
                cur_row_nr++;
                cur_row_spot_nr = 0;
                if(cur_row_nr == 1) {
                    cur_row_spots = 6;
                } else {
                    cur_row_spots += 6;
                }
            }
            
            destination += this->pos;
            
            subgroups_info[s].members[m]->leave_group();
            subgroups_info[s].members[m]->fsm.run_event(
                MOB_EVENT_DISMISSED, (void*) &destination
            );
            
        }
    }
    
    //Dismiss leaders now.
    while(!group->members.empty()) {
        group->members[0]->fsm.run_event(MOB_EVENT_DISMISSED, NULL);
        group->members[0]->leave_group();
    }
    
    //Final things.
    lea_type->sfx_dismiss.play(0, false);
    set_animation(LEADER_ANIM_DISMISSING);
}


/* ----------------------------------------------------------------------------
 * Swaps out the currently held Pikmin for a different one.
 */
void leader::swap_held_pikmin(mob* new_pik) {
    if(holding.empty()) return;
    
    mob_event* old_pik_ev = holding[0]->fsm.get_event(MOB_EVENT_RELEASED);
    mob_event* new_pik_ev = new_pik->fsm.get_event(MOB_EVENT_GRABBED_BY_FRIEND);
    
    group->sort(new_pik->subgroup_type_ptr);
    
    if(!old_pik_ev || !new_pik_ev) return;
    
    old_pik_ev->run(holding[0]);
    new_pik_ev->run(new_pik);
    
    release(holding[0]);
    hold(new_pik, INVALID, LEADER_HELD_MOB_DIST, LEADER_HELD_MOB_ANGLE, false);
    
    sfx_switch_pikmin.play(0, false);
}


/* ----------------------------------------------------------------------------
 * Draw a leader mob.
 */
void leader::draw_mob(bitmap_effect_manager* effect_manager) {
    mob::draw_mob();
    
    sprite* s_ptr = anim.get_cur_sprite();
    point draw_pos = get_sprite_center(s_ptr);
    point draw_size = get_sprite_dimensions(s_ptr);
    
    if(invuln_period.time_left > 0.0f) {
        sprite* spark_s = spark_animation.instance.get_cur_sprite();
        if(spark_s && spark_s->bitmap) {
            draw_bitmap(
                spark_s->bitmap, draw_pos,
                draw_size
            );
        }
    }
    
    draw_status_effect_bmp(this, NULL);
}


/* ----------------------------------------------------------------------------
 * Returns how many rows will be needed to fit all of the members.
 * Used to calculate how subgroup members will be placed when dismissing.
 */
size_t leader::get_dismiss_rows(const size_t n_members) {
    size_t members_that_fit = 1;
    size_t rows_needed = 1;
    while(members_that_fit < n_members) {
        rows_needed++;
        members_that_fit += 6 * (rows_needed - 1);
    }
    return rows_needed;
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
void change_to_next_leader(const bool forward, const bool force_success) {
    if(leaders.size() == 1) return;
    
    if(
        !cur_leader_ptr->fsm.get_event(LEADER_EVENT_UNFOCUSED) &&
        !force_success
    ) {
        //This leader isn't ready to be switched out of. Forget it.
        return;
    }
    
    //We'll send the switch event to the next leader on the list.
    //If they accept, they run a function to change leaders.
    //If not, we try the next leader.
    //If we return to the current leader without anything being
    //changed, then stop trying; no leader can be switched to.
    
    size_t new_leader_nr = cur_leader_nr;
    leader* new_leader_ptr = NULL;
    bool searching = true;
    size_t original_leader_nr = cur_leader_nr;
    bool cant_find_new_leader = false;
    
    while(searching) {
        new_leader_nr =
            sum_and_wrap(new_leader_nr, (forward ? 1 : -1), leaders.size());
        new_leader_ptr = leaders[new_leader_nr];
        
        if(new_leader_nr == original_leader_nr) {
            //Back to the original; stop trying.
            cant_find_new_leader = true;
            searching = false;
        }
        
        new_leader_ptr->fsm.run_event(LEADER_EVENT_FOCUSED);
        
        //If after we called the event, the leader is the same,
        //then that means the leader can't be switched to.
        //Try a new one.
        if(cur_leader_nr != original_leader_nr) {
            searching = false;
        }
    }
    
    if(cant_find_new_leader && force_success) {
        //Ok, we need to force a leader to accept the focus. Let's do so.
        cur_leader_nr =
            sum_and_wrap(new_leader_nr, (forward ? 1 : -1), leaders.size());
        cur_leader_ptr = leaders[cur_leader_nr];
        
        cur_leader_ptr->fsm.set_state(LEADER_STATE_ACTIVE);
    }
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
            (group_move_magnitude && cur_leader_ptr == this);
            
        if(
            dist(group->get_average_member_pos(), pos) >
            GROUP_SHUFFLE_DIST + (group->radius + type->radius)
        ) {
            if(!group->follow_mode) {
                must_reassign_spots = true;
            }
            group->follow_mode = true;
            
        } else if(is_moving_group || !holding.empty()) {
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
                        ), group_move_angle + TAU / 2
                    );
                group->anchor = pos + move_anchor_offset;
                
                float intensity_dist = cursor_max_dist * group_move_magnitude;
                al_translate_transform(
                    &group->transform, -GROUP_MOVE_MARGIN, 0
                );
                al_scale_transform(
                    &group->transform,
                    intensity_dist / (group->radius * 2),
                    1 - (GROUP_MOVE_VERTICAL_SCALE * group_move_magnitude)
                );
                al_rotate_transform(
                    &group->transform,
                    group_move_angle + TAU / 2
                );
                
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
            
            if(must_reassign_spots) group->reassign_spots();
            
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
    
    if(fabs(closest_group_member->z - cur_leader_ptr->z) > SECTOR_STEP) {
        //If the group member is beyond a step, it's obviously above or below
        //a wall, compared to the leader. No grabbing allowed.
        closest_group_member_distant = true;
    } else {
        closest_group_member_distant = closest_dist > pikmin_grab_range;
    }
}
