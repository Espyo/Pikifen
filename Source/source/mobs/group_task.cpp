/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Group task class and group task-related functions.
 */

#include "group_task.h"

#include "../game.h"


/* ----------------------------------------------------------------------------
 * Creates a new group task mob.
 * pos:
 *   Starting coordinates.
 * type:
 *   Group task type this mob belongs to.
 * angle:
 *   Starting angle.
 */
group_task::group_task(
    const point &pos, group_task_type* type, const float angle
):
    mob(pos, type, angle),
    tas_type(type),
    power_goal(type->power_goal),
    power(0),
    ran_task_finished_code(false) {
    
    //Initialize spots.
    float row_angle = get_angle(tas_type->first_row_p1, tas_type->first_row_p2);
    size_t needed_rows =
        ceil(tas_type->max_pikmin / (float) tas_type->pikmin_per_row);
    float point_dist =
        dist(tas_type->first_row_p1, tas_type->first_row_p2).to_float();
    float space_between_neighbors =
        point_dist / (float) (tas_type->pikmin_per_row - 1);
        
    //Create a transformation based on the anchor -- p1.
    ALLEGRO_TRANSFORM trans;
    al_identity_transform(&trans);
    al_rotate_transform(&trans, row_angle);
    al_translate_transform(
        &trans, tas_type->first_row_p1.x, tas_type->first_row_p1.y
    );
    
    for(size_t r = 0; r < needed_rows; ++r) {
    
        for(size_t s = 0; s < tas_type->pikmin_per_row; ++s) {
        
            float x;
            if(tas_type->pikmin_per_row % 2 == 0) {
                x =
                    space_between_neighbors / 2.0 +
                    space_between_neighbors * ceil((s - 1.0f) / 2.0);
                x *= (s % 2 == 0) ? 1 : -1;
            } else {
                if(s == 0) {
                    x = 0;
                } else {
                    x = space_between_neighbors * ceil(s / 2.0);
                    x *= (s % 2 == 0) ? 1 : -1;
                }
            }
            x += point_dist / 2.0f;
            
            point s_pos(x, r * tas_type->interval_between_rows);
            al_transform_coordinates(&trans, &s_pos.x, &s_pos.y);
            
            spots.push_back(group_task_spot(s_pos));
        }
    }
    
    update_spot_absolute_positions();
}


/* ----------------------------------------------------------------------------
 * Adds a Pikmin to the task as an actual worker.
 * who:
 *   Pikmin to add.
 */
void group_task::add_worker(pikmin* who) {
    for(size_t s = 0; s < spots.size(); ++s) {
        if(spots[s].pikmin_here == who) {
            spots[s].state = 2;
            break;
        }
    }
    
    bool had_goal = power >= power_goal;
    
    switch(tas_type->contribution_method) {
    case GROUP_TASK_CONTRIBUTION_NORMAL: {
        power++;
        break;
    } case GROUP_TASK_CONTRIBUTION_WEIGHT: {
        power += who->pik_type->weight;
        break;
    } case GROUP_TASK_CONTRIBUTION_CARRY_STRENGTH: {
        power += who->pik_type->carry_strength;
        break;
    } case GROUP_TASK_CONTRIBUTION_PUSH_STRENGTH: {
        power += who->pik_type->push_strength;
        break;
    }
    }
    
    if(!had_goal && power >= power_goal) {
        string msg = "goal_reached";
        who->send_message(this, msg);
    }
}


/* ----------------------------------------------------------------------------
 * Code to run when the task is finished.
 */
void group_task::finish_task() {
    for(
        size_t p = 0;
        p < game.states.gameplay->mobs.pikmin_list.size(); ++p
    ) {
        pikmin* p_ptr = game.states.gameplay->mobs.pikmin_list[p];
        if(p_ptr->focused_mob && p_ptr->focused_mob == this) {
            p_ptr->fsm.run_event(MOB_EV_FOCUSED_MOB_UNAVAILABLE);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Frees up a previously-reserved spot.
 * whose:
 *   Who had the reservation?
 */
void group_task::free_up_spot(pikmin* whose) {
    bool was_contributing = false;
    
    for(size_t s = 0; s < spots.size(); ++s) {
        if(spots[s].pikmin_here == whose) {
            if(spots[s].state == 2) {
                was_contributing = true;
            }
            spots[s].state = 0;
            spots[s].pikmin_here = NULL;
            break;
        }
    }
    
    if(was_contributing) {
        bool had_goal = power >= power_goal;
        
        switch(tas_type->contribution_method) {
        case GROUP_TASK_CONTRIBUTION_NORMAL: {
            power--;
            break;
        } case GROUP_TASK_CONTRIBUTION_WEIGHT: {
            power -= whose->pik_type->weight;
            break;
        } case GROUP_TASK_CONTRIBUTION_CARRY_STRENGTH: {
            power -= whose->pik_type->carry_strength;
            break;
        } case GROUP_TASK_CONTRIBUTION_PUSH_STRENGTH: {
            power -= whose->pik_type->push_strength;
            break;
        }
        }
        
        if(had_goal && power < power_goal) {
            string msg = "goal_lost";
            whose->send_message(this, msg);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Returns information on how to show the fraction numbers.
 * Returns true if the fraction numbers should be shown, false if not.
 * This only keeps in mind things specific to this class, so it shouldn't
 * check for things like carrying, which is global to all mobs.
 * fraction_value_nr:
 *   The fraction's value (upper) number gets set here.
 * fraction_req_nr:
 *   The fraction's required (lower) number gets set here.
 * fraction_color:
 *   The fraction's color gets set here.
 */
bool group_task::get_fraction_numbers_info(
    float* fraction_value_nr, float* fraction_req_nr,
    ALLEGRO_COLOR* fraction_color
) const {
    if(get_power() <= 0) return false;
    *fraction_value_nr = get_power();
    *fraction_req_nr = power_goal;
    *fraction_color = game.config.carrying_color_stop;
    return true;
}


/* ----------------------------------------------------------------------------
 * Returns a free spot, closest to the center and to the frontmost row as
 * possible.
 * Returns NULL if there is none.
 */
group_task::group_task_spot* group_task::get_free_spot() {
    size_t spots_taken = 0;
    
    for(size_t s = 0; s < spots.size(); ++s) {
        if(spots[s].state != 0) {
            spots_taken++;
            if(spots_taken == tas_type->max_pikmin) {
                //Max Pikmin reached! The Pikmin can't join,
                //regardless of there being free spots.
                return NULL;
            }
        }
        if(spots[s].state == 0) return &(spots[s]);
    }
    
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Returns the current power put into the task.
 */
float group_task::get_power() const {
    return power;
}


/* ----------------------------------------------------------------------------
 * Returns the current world coordinates of a spot, occupied by a Pikmin.
 * Returns a (0,0) point if that Pikmin doesn't have a spot.
 * whose:
 *   Pikmin whose spot to check.
 */
point group_task::get_spot_pos(const pikmin* whose) const {
    for(size_t s = 0; s < spots.size(); ++s) {
        if(spots[s].pikmin_here == whose) {
            return spots[s].absolute_pos;
        }
    }
    return point();
}


/* ----------------------------------------------------------------------------
 * Reads the provided script variables, if any, and does stuff with them.
 * svr:
 *   Script var reader to use.
 */
void group_task::read_script_vars(const script_var_reader &svr) {
    mob::read_script_vars(svr);
    
    svr.get("power_goal", power_goal);
}


/* ----------------------------------------------------------------------------
 * Reserves a spot for a Pikmin.
 * spot:
 *   Pointer to the spot to reserve.
 * who:
 *   Who will be reserving this spot?
 */
void group_task::reserve_spot(group_task::group_task_spot* spot, pikmin* who) {
    spot->state = 1;
    spot->pikmin_here = who;
}


/* ----------------------------------------------------------------------------
 * Ticks time by one frame of logic.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 */
void group_task::tick_class_specifics(const float delta_t) {
    if(health <= 0 && !ran_task_finished_code) {
        ran_task_finished_code = true;
        finish_task();
    }
    
    if(health > 0) {
        ran_task_finished_code = false;
    }
    
    if(
        chase_info.state == CHASE_STATE_CHASING &&
        power >= power_goal &&
        tas_type->speed_bonus != 0.0f
    ) {
        //Being moved and movements can go through speed bonuses?
        //Let's update the speed.
        chase_info.max_speed =
            type->move_speed +
            (power - power_goal) * tas_type->speed_bonus;
        chase_info.acceleration = MOB::CARRIED_MOB_ACCELERATION;
    }
    
    update_spot_absolute_positions();
}


/* ----------------------------------------------------------------------------
 * Updates the absolute position of all spots, based on where the group task
 * mob currently is and where it is currently facing.
 */
void group_task::update_spot_absolute_positions() {
    ALLEGRO_TRANSFORM t;
    al_identity_transform(&t);
    al_rotate_transform(&t, angle);
    al_translate_transform(&t, pos.x, pos.y);
    
    for(size_t s = 0; s < spots.size(); ++s) {
        point* p = &(spots[s].absolute_pos);
        *p = spots[s].relative_pos;
        al_transform_coordinates(&t, &(p->x), &(p->y));
    }
}


/* ----------------------------------------------------------------------------
 * Creates a new group task spot struct.
 * pos:
 *   Position of the spot, in relative coordinates.
 */
group_task::group_task_spot::group_task_spot(const point &pos) :
    relative_pos(pos), absolute_pos(pos), state(0), pikmin_here(nullptr) {
    
}
