/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Mob utility classes and functions.
 */

#include <algorithm>

#include "mob_utils.h"

#include "../functions.h"
#include "../mob_script_action.h"
#include "../vars.h"
#include "mob.h"

using namespace std;


/* ----------------------------------------------------------------------------
 * Creates a structure with info about carrying.
 * m:                 The mob this info belongs to.
 * max_carriers:      The maximum number of carrier Pikmin.
 * carry_destination: Where to deliver the mob. Use CARRY_DESTINATION_*.
 */
carry_info_struct::carry_info_struct(mob* m, const size_t destination) :
    m(m),
    destination(destination),
    cur_carrying_strength(0),
    cur_n_carriers(0),
    is_stuck(false),
    is_moving(false),
    intended_mob(nullptr),
    must_return(false),
    return_dist(0) {
    
    for(size_t c = 0; c < m->type->max_carriers; ++c) {
        float angle = TAU / m->type->max_carriers * c;
        point p(
            cos(angle) * (m->type->radius + standard_pikmin_radius),
            sin(angle) * (m->type->radius + standard_pikmin_radius)
        );
        spot_info.push_back(carrier_spot_struct(p));
    }
}


/* ----------------------------------------------------------------------------
 * Deletes a carrier info structure.
 */
carry_info_struct::~carry_info_struct() {
    //TODO
}


/* ----------------------------------------------------------------------------
 * Returns the speed at which the object should move, given the carrier Pikmin.
 */
float carry_info_struct::get_speed() {
    float max_speed = 0;
    
    //Begin by obtaining the average walking speed of the carriers.
    for(size_t s = 0; s < spot_info.size(); ++s) {
        carrier_spot_struct* s_ptr = &spot_info[s];
        
        if(s_ptr->state != CARRY_SPOT_USED) continue;
        
        pikmin* p_ptr = (pikmin*) s_ptr->pik_ptr;
        max_speed += p_ptr->get_base_speed();
    }
    max_speed /= cur_n_carriers;
    
    //If the object has all carriers, the Pikmin move as fast
    //as possible, which looks bad, since they're not jogging,
    //they're carrying. Let's add a penalty for the weight...
    max_speed *= (1 - carrying_speed_weight_mult * m->type->weight);
    //...and a global carrying speed penalty.
    max_speed *= carrying_speed_max_mult;
    
    //The closer the mob is to having full carriers,
    //the closer to the max speed we get.
    //The speed goes from carrying_speed_base_mult (0 carriers)
    //to max_speed (all carriers).
    return
        max_speed * (
            carrying_speed_base_mult +
            (cur_n_carriers / (float) spot_info.size()) *
            (1 - carrying_speed_base_mult)
        );
}


/* ----------------------------------------------------------------------------
 * Returns true if no spot is reserved or used. False otherwise.
 */
bool carry_info_struct::is_empty() {
    for(size_t s = 0; s < spot_info.size(); ++s) {
        if(spot_info[s].state != CARRY_SPOT_FREE) return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Returns true if all spots are reserved. False otherwise.
 */
bool carry_info_struct::is_full() {
    for(size_t s = 0; s < spot_info.size(); ++s) {
        if(spot_info[s].state == CARRY_SPOT_FREE) return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Rotates all points in the struct, making it so spot 0 faces the specified
 * angle away from the mob.
 * This is useful when the first Pikmin is coming, to make the first carry
 * spot be closer to that Pikmin.
 */
void carry_info_struct::rotate_points(const float angle) {
    for(size_t s = 0; s < spot_info.size(); ++s) {
        float s_angle = angle + (TAU / m->type->max_carriers * s);
        point p(
            cos(s_angle) * (m->type->radius + standard_pikmin_radius),
            sin(s_angle) * (m->type->radius + standard_pikmin_radius)
        );
        spot_info[s].pos = p;
    }
}


/* ----------------------------------------------------------------------------
 * Creates a structure with info about a carrying spot.
 */
carrier_spot_struct::carrier_spot_struct(const point &pos) :
    state(CARRY_SPOT_FREE),
    pos(pos),
    pik_ptr(NULL) {
    
}


/* ----------------------------------------------------------------------------
 * Creates an instance of a structure with info about the mob's circling.
 */
circling_info_struct::circling_info_struct(mob* m) :
    m(m),
    circling_mob(nullptr),
    radius(0),
    clockwise(true),
    speed(0),
    can_free_move(false),
    cur_angle(0) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a new group information struct.
 */
group_info::group_info(mob* leader_ptr) :
    radius(0),
    anchor(leader_ptr->pos),
    transform(identity_transform),
    cur_standby_type(nullptr),
    follow_mode(false) {
}


/* ----------------------------------------------------------------------------
 * Changes to a different standby subgroup type in case there are no more
 * Pikmin of the current one. Or to no type.
 */
void group_info::change_standby_type_if_needed() {
    for(size_t m = 0; m < members.size(); ++m) {
        if(members[m]->subgroup_type_ptr == cur_standby_type) {
            //Never mind, there is a member of this subgroup type.
            return;
        }
    }
    //No members of the current type? Switch to the next.
    set_next_cur_standby_type(false);
}


/* ----------------------------------------------------------------------------
 * Returns the average position of the members.
 */
point group_info::get_average_member_pos() {
    point avg;
    for(size_t m = 0; m < members.size(); ++m) {
        avg += members[m]->pos;
    }
    return avg / members.size();
}


/* ----------------------------------------------------------------------------
 * Returns a point's offset from the anchor,
 * given the current group transformation.
 */
point group_info::get_spot_offset(const size_t spot_index) {
    point res = spots[spot_index].pos;
    al_transform_coordinates(&transform, &res.x, &res.y);
    return res;
}


/* ----------------------------------------------------------------------------
 * (Re-)Initializes the group spots. This resizes it to the current number
 * of group members. Any old group members are moved to the appropriate
 * new spot.
 * affected_mob_ptr: If this initialization is because a new mob entered
 *   or left the group, this should point to said mob.
 */
void group_info::init_spots(mob* affected_mob_ptr) {
    if(members.empty()) {
        spots.clear();
        radius = 0;
        return;
    }
    
    //First, backup the old mob indexes.
    vector<mob*> old_mobs;
    old_mobs.resize(spots.size());
    for(size_t m = 0; m < spots.size(); ++m) {
        old_mobs[m] = spots[m].mob_ptr;
    }
    
    //Now, rebuild the spots. Let's draw wheels from the center, for now.
    struct alpha_spot {
        point pos;
        dist distance_to_rightmost;
        alpha_spot(const point &p) :
            pos(p) { }
    };
    
    vector<alpha_spot> alpha_spots;
    size_t current_wheel = 1;
    radius = standard_pikmin_radius;
    
    //Center spot first.
    alpha_spots.push_back(alpha_spot(point()));
    
    while(alpha_spots.size() < members.size()) {
    
        //First, calculate how far the center
        //of these spots are from the central spot.
        float dist_from_center =
            standard_pikmin_radius * current_wheel + //Spots.
            GROUP_SPOT_INTERVAL * current_wheel; //Interval between spots.
            
        /* Now we need to figure out what's the angular distance
         * between each spot. For that, we need the actual diameter
         * (distance from one point to the other),
         * and the central distance, which is distance between the center
         * and the middle of two spots.
         *
         * We can get the middle distance because we know the actual diameter,
         * which should be the size of a Pikmin and one interval unit,
         * and we know the distance from one spot to the center.
         */
        float actual_diameter =
            standard_pikmin_radius * 2.0 + GROUP_SPOT_INTERVAL;
            
        //Just calculate the remaining side of the triangle, now that we know
        //the hypotenuse and the actual diameter (one side of the triangle).
        float middle_distance =
            sqrt(
                (dist_from_center * dist_from_center) -
                (actual_diameter * 0.5 * actual_diameter * 0.5)
            );
            
        //Now, get the angular distance.
        float angular_dist =
            atan2(actual_diameter, middle_distance * 2.0f) * 2.0;
            
        //Finally, we can calculate where the other spots are.
        size_t n_spots_on_wheel = floor(TAU / angular_dist);
        //Get a better angle. One that can evenly distribute the spots.
        float angle = TAU / n_spots_on_wheel;
        
        for(unsigned s = 0; s < n_spots_on_wheel; ++s) {
            alpha_spots.push_back(
                alpha_spot(
                    point(
                        dist_from_center * cos(angle * s) +
                        randomf(-GROUP_SPOT_INTERVAL, GROUP_SPOT_INTERVAL),
                        dist_from_center * sin(angle * s) +
                        randomf(-GROUP_SPOT_INTERVAL, GROUP_SPOT_INTERVAL)
                    )
                )
            );
        }
        
        current_wheel++;
        radius = dist_from_center;
    }
    
    //Now, given all of these points, create our final spot vector,
    //with the rightmost points coming first.
    
    //Start by sorting the points.
    for(size_t a = 0; a < alpha_spots.size(); ++a) {
        alpha_spots[a].distance_to_rightmost =
            dist(
                alpha_spots[a].pos,
                point(radius, 0)
            );
    }
    
    std::sort(
        alpha_spots.begin(), alpha_spots.end(),
    [] (alpha_spot a1, alpha_spot a2) -> bool {
        return a1.distance_to_rightmost < a2.distance_to_rightmost;
    }
    );
    
    //Finally, create the group spots.
    spots.clear();
    spots.resize(members.size(), group_spot());
    for(size_t s = 0; s < members.size(); ++s) {
        spots[s] =
            group_spot(
                point(
                    alpha_spots[s].pos.x - radius,
                    alpha_spots[s].pos.y
                ),
                NULL
            );
    }
    
    //Pass the old mobs over.
    if(old_mobs.size() < spots.size()) {
        for(size_t m = 0; m < old_mobs.size(); ++m) {
            spots[m].mob_ptr = old_mobs[m];
            spots[m].mob_ptr->group_spot_index = m;
        }
        spots[old_mobs.size()].mob_ptr = affected_mob_ptr;
        affected_mob_ptr->group_spot_index = old_mobs.size();
        
    } else if(old_mobs.size() > spots.size()) {
        for(size_t m = 0, s = 0; m < old_mobs.size(); ++m) {
            if(old_mobs[m] == affected_mob_ptr) {
                old_mobs[m]->group_spot_index = INVALID;
                continue;
            }
            spots[s].mob_ptr = old_mobs[m];
            spots[s].mob_ptr->group_spot_index = s;
            ++s;
        }
        
    } else {
        for(size_t m = 0; m < old_mobs.size(); ++m) {
            spots[m].mob_ptr = old_mobs[m];
            spots[m].mob_ptr->group_spot_index = m;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Assigns each mob a new spot, given how close each one of them is to
 * each spot.
 */
void group_info::reassign_spots() {
    for(size_t m = 0; m < members.size(); ++m) {
        members[m]->group_spot_index = INVALID;
    }
    
    for(size_t s = 0; s < spots.size(); ++s) {
        point spot_pos = anchor + get_spot_offset(s);
        mob* closest_mob = NULL;
        dist closest_dist;
        
        for(size_t m = 0; m < members.size(); ++m) {
            mob* m_ptr = members[m];
            if(m_ptr->group_spot_index != INVALID) continue;
            
            dist d(m_ptr->pos, spot_pos);
            
            if(!closest_mob || d < closest_dist) {
                closest_mob = m_ptr;
                closest_dist = d;
            }
        }
        
        closest_mob->group_spot_index = s;
    }
}


/* ----------------------------------------------------------------------------
 * Sets the standby group member type to the next available one,
 * or NULL if none.
 * Returns true on success, false on failure.
 * move_backwards: If true, go through the list backwards.
 */
bool group_info::set_next_cur_standby_type(const bool move_backwards) {

    if(members.empty()) {
        cur_standby_type = NULL;
        return true;
    }
    
    bool success = false;
    subgroup_type* starting_type = cur_standby_type;
    subgroup_type* final_type = cur_standby_type;
    if(!starting_type) starting_type = subgroup_types.get_first_type();
    subgroup_type* scanning_type = starting_type;
    subgroup_type* leader_subgroup_type =
        subgroup_types.get_type(SUBGROUP_TYPE_CATEGORY_LEADER);
        
    if(move_backwards) {
        scanning_type = subgroup_types.get_prev_type(scanning_type);
    } else {
        scanning_type = subgroup_types.get_next_type(scanning_type);
    }
    while(scanning_type != starting_type && !success) {
        //For each type, let's check if there's any group member that matches.
        if(
            scanning_type == leader_subgroup_type &&
            !can_throw_leaders
        ) {
            //If this is a leader, and leaders cannot be thrown, skip.
        } else {
            for(size_t m = 0; m < members.size(); ++m) {
                if(members[m]->subgroup_type_ptr == scanning_type) {
                    final_type = scanning_type;
                    success = true;
                    break;
                }
            }
        }
        
        if(move_backwards) {
            scanning_type = subgroup_types.get_prev_type(scanning_type);
        } else {
            scanning_type = subgroup_types.get_next_type(scanning_type);
        }
    }
    
    cur_standby_type = final_type;
    return success;
}


/* ----------------------------------------------------------------------------
 * Sorts the group with the specified type at the front, and the other types
 * (in order) behind.
 */
void group_info::sort(subgroup_type* leading_type) {

    for(size_t m = 0; m < members.size(); ++m) {
        members[m]->group_spot_index = INVALID;
    }
    
    subgroup_type* cur_type = leading_type;
    size_t cur_spot = 0;
    
    while(cur_spot != spots.size()) {
        point spot_pos = anchor + get_spot_offset(cur_spot);
        
        //Find the member closest to this spot.
        mob* closest_member = NULL;
        dist closest_dist;
        for(size_t m = 0; m < members.size(); ++m) {
            mob* m_ptr = members[m];
            if(m_ptr->subgroup_type_ptr != cur_type) continue;
            if(m_ptr->group_spot_index != INVALID) continue;
            
            dist d(m_ptr->pos, spot_pos);
            
            if(!closest_member || d < closest_dist) {
                closest_member = m_ptr;
                closest_dist = d;
            }
            
        }
        
        if(!closest_member) {
            //There are no more members of the current type left!
            //Next type.
            cur_type = subgroup_types.get_next_type(cur_type);
        } else {
            spots[cur_spot].mob_ptr = closest_member;
            closest_member->group_spot_index = cur_spot;
            cur_spot++;
        }
        
    }
    
}


/* ----------------------------------------------------------------------------
 * Creates a mob-holding information struct.
 */
hold_info_struct::hold_info_struct() :
    m(nullptr),
    hitbox_nr(INVALID),
    offset_dist(0),
    offset_angle(0),
    above_holder(false) {
    
}


/* ----------------------------------------------------------------------------
 * Clears the information.
 */
void hold_info_struct::clear() {
    m = NULL;
    hitbox_nr = INVALID;
    offset_dist = 0;
    offset_angle = 0;
}


/* ----------------------------------------------------------------------------
 * Returns the final coordinates this mob should be at.
 */
point hold_info_struct::get_final_pos(float* final_z) {
    if(!m) return point();
    
    hitbox* h_ptr = NULL;
    if(hitbox_nr != INVALID) {
        h_ptr = m->get_hitbox(hitbox_nr);
    }
    
    point final_pos;
    
    if(h_ptr) {
        //Hitbox.
        final_pos = rotate_point(h_ptr->pos, m->angle);
        final_pos += m->pos;
        
        final_pos +=
            angle_to_coordinates(
                offset_angle + m->angle,
                offset_dist * h_ptr->radius
            );
        *final_z = m->z + h_ptr->z;
    } else {
        //Body center.
        final_pos = m->pos;
        
        final_pos +=
            angle_to_coordinates(
                offset_angle + m->angle,
                offset_dist * m->type->radius
            );
        *final_z = m->z;
    }
    
    return final_pos;
}



/* ----------------------------------------------------------------------------
 * Initializes a parent mob information struct.
 */
parent_mob_info::parent_mob_info(mob* m) :
    m(m),
    handle_damage(false),
    relay_damage(false),
    handle_statuses(false),
    relay_statuses(false),
    handle_events(false),
    relay_events(false),
    limb_thickness(32.0),
    limb_parent_body_part(INVALID),
    limb_parent_offset(0),
    limb_child_body_part(INVALID),
    limb_child_offset(0),
    limb_draw_method(LIMB_DRAW_ABOVE_CHILD) {
    
}


/* ----------------------------------------------------------------------------
 * Creates an instance of a structure with info about the mob's path-following.
 */
path_info_struct::path_info_struct(mob* m, const point &target) :
    m(m),
    target_point(target),
    cur_path_stop_nr(0),
    go_straight(false) {
    
    path = get_path(m->pos, target, &obstacle_ptrs, &go_straight, NULL);
}


/* ----------------------------------------------------------------------------
 * Creates an instance of a structure with info about the track the mob
 * is riding.
 */
track_info_struct::track_info_struct(mob* m) :
    m(m),
    cur_cp_nr(0),
    cur_cp_progress(0.0f) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a mob, adding it to the corresponding vectors.
 * Returns the new mob.
 * category:            The category the new mob belongs to.
 * pos:                 Initial position.
 * type:                Type of the new mob.
 * angle:               Initial facing angle.
 * vars:                Script variables.
 * code_after_creation: Code to run right after the mob is created, if any.
 *   This is run before any scripting takes place.
 */
mob* create_mob(
    mob_category* category, const point &pos, mob_type* type,
    const float angle, const string &vars,
    function<void(mob*)> code_after_creation
) {
    mob* m_ptr = category->create_mob(pos, type, angle);
    
    if(code_after_creation) {
        code_after_creation(m_ptr);
    }
    
    for(size_t a = 0; a < type->init_actions.size(); ++a) {
        type->init_actions[a]->run(m_ptr, NULL, NULL);
    }
    
    m_ptr->read_script_vars(vars);
    if(!vars.empty()) {
        vector<string> var_name_strings;
        vector<string> var_value_strings;
        get_var_vectors(vars, var_name_strings, var_value_strings);
        for(size_t v = 0; v < var_name_strings.size(); ++v) {
            m_ptr->vars[var_name_strings[v]] = var_value_strings[v];
        }
    }
    
    m_ptr->fsm.set_state(
        m_ptr->fsm.first_state_override != INVALID ?
        m_ptr->fsm.first_state_override :
        type->first_state_nr
    );
    
    for(size_t c = 0; c < type->children.size(); ++c) {
        mob_type::child_struct* child_info = &type->children[c];
        
        mob_type::spawn_struct* spawn_info = NULL;
        for(size_t s = 0; s < type->spawns.size(); ++s) {
            if(type->spawns[s].name == child_info->spawn_name) {
                spawn_info = &type->spawns[s];
                break;
            }
        }
        
        if(!spawn_info) {
            log_error(
                "Object \"" + type->name + "\" tried to spawn a child with the "
                "spawn name \"" + child_info->spawn_name + "\", but that name "
                "does not exist!"
            );
            continue;
        }
        
        mob* new_mob = m_ptr->spawn(spawn_info);
        if(!new_mob) continue;
        
        parent_mob_info* p_info = new parent_mob_info(m_ptr);
        new_mob->parent = p_info;
        p_info->handle_damage = child_info->handle_damage;
        p_info->relay_damage = child_info->relay_damage;
        p_info->handle_events = child_info->handle_events;
        p_info->relay_events = child_info->relay_events;
        p_info->handle_statuses = child_info->handle_statuses;
        p_info->relay_statuses = child_info->relay_statuses;
        if(!child_info->limb_anim_name.empty()) {
            p_info->limb_anim.anim_db = m_ptr->anim.anim_db;
            animation* anim_to_use = NULL;
            for(size_t a = 0; a < m_ptr->anim.anim_db->animations.size(); ++a) {
                if(
                    m_ptr->anim.anim_db->animations[a]->name ==
                    child_info->limb_anim_name
                ) {
                    anim_to_use = m_ptr->anim.anim_db->animations[a];
                }
            }
            
            if(anim_to_use) {
                p_info->limb_anim.cur_anim = anim_to_use;
                p_info->limb_anim.start();
            } else {
                log_error(
                    "Object \"" + new_mob->type->name + "\", child object of "
                    "object \"" + type->name + "\", tried to use animation \"" +
                    child_info->limb_anim_name + "\" for a limb, but that "
                    "animation doesn't exist in the parent object's animations!"
                );
            }
        }
        p_info->limb_thickness = child_info->limb_thickness;
        p_info->limb_parent_body_part =
            type->anims.find_body_part(child_info->limb_parent_body_part);
        p_info->limb_parent_offset = child_info->limb_parent_offset;
        p_info->limb_child_body_part =
            new_mob->type->anims.find_body_part(
                child_info->limb_child_body_part
            );
        p_info->limb_child_offset = child_info->limb_child_offset;
        p_info->limb_draw_method = child_info->limb_draw_method;
        
        if(child_info->parent_holds) {
            m_ptr->hold(
                new_mob,
                type->anims.find_body_part(child_info->hold_body_part),
                child_info->hold_offset_dist,
                child_info->hold_offset_angle, false,
                child_info->hold_rotation_method
            );
        }
    }
    
    mobs.push_back(m_ptr);
    return m_ptr;
}


/* ----------------------------------------------------------------------------
 * Deletes a mob from the relevant vectors.
 * It's always removed from the vector of mobs, but it's
 * also removed from the vector of Pikmin if it's a Pikmin,
 * leaders if it's a leader, etc.
 * m_ptr:                The mob to delete.
 * complete_destruction: If true, don't bother removing it from groups and such,
 *   since everything is going to be destroyed.
 */
void delete_mob(mob* m_ptr, const bool complete_destruction) {
    if(creator_tool_info_lock == m_ptr) creator_tool_info_lock = NULL;
    
    if(!complete_destruction) {
        m_ptr->leave_group();
        
        for(size_t m = 0; m < mobs.size(); ++m) {
            if(mobs[m]->focused_mob == m_ptr) {
                mobs[m]->fsm.run_event(MOB_EVENT_FOCUSED_MOB_UNAVAILABLE);
                mobs[m]->fsm.run_event(MOB_EVENT_FOCUS_OFF_REACH);
                mobs[m]->fsm.run_event(MOB_EVENT_FOCUS_DIED);
                mobs[m]->focused_mob = NULL;
            }
            if(mobs[m]->parent && mobs[m]->parent->m == m_ptr) {
                delete mobs[m]->parent;
                mobs[m]->parent = NULL;
                mobs[m]->to_delete = true;
            }
        }
        
        while(!m_ptr->holding.empty()) {
            m_ptr->release(m_ptr->holding[0]);
        }
        
        m_ptr->fsm.set_state(INVALID);
    }
    
    m_ptr->type->category->erase_mob(m_ptr);
    mobs.erase(find(mobs.begin(), mobs.end(), m_ptr));
    
    delete m_ptr;
}


/* ----------------------------------------------------------------------------
 * Returns a string that describes the given mob. Used in error messages
 * where you have to indicate a specific mob in the area.
 */
string get_error_message_mob_info(mob* m) {
    return
        "type \"" + m->type->name + "\", coordinates " +
        p2s(m->pos) + ", area \"" + cur_area_data.name + "\"";
}


/* ----------------------------------------------------------------------------
 * Converts a string to the numeric representation of a mob target type.
 * Returns INVALID if the string is not valid.
 */
size_t string_to_mob_target_type(const string &type_str) {
    if(type_str == "none") {
        return MOB_TARGET_TYPE_NONE;
    } else if(type_str == "player") {
        return MOB_TARGET_TYPE_PLAYER;
    } else if(type_str == "enemy") {
        return MOB_TARGET_TYPE_ENEMY;
    } else if(type_str == "weak_plain_obstacle") {
        return MOB_TARGET_TYPE_WEAK_PLAIN_OBSTACLE;
    } else if(type_str == "strong_plain_obstacle") {
        return MOB_TARGET_TYPE_STRONG_PLAIN_OBSTACLE;
    } else if(type_str == "pikmin_obstacle") {
        return MOB_TARGET_TYPE_PIKMIN_OBSTACLE;
    } else if(type_str == "explodable") {
        return MOB_TARGET_TYPE_EXPLODABLE;
    } else if(type_str == "explodable_pikmin_obstacle") {
        return MOB_TARGET_TYPE_EXPLODABLE_PIKMIN_OBSTACLE;
    } else if(type_str == "fragile") {
        return MOB_TARGET_TYPE_FRAGILE;
    }
    return INVALID;
}


/* ----------------------------------------------------------------------------
 * Converts a string to the numeric representation of a team.
 * Returns INVALID if the string is not valid.
 */
size_t string_to_team_nr(const string &team_str) {
    if(team_str == "none") {
        return MOB_TEAM_NONE;
    } else if(team_str == "player_1") {
        return MOB_TEAM_PLAYER_1;
    } else if(team_str == "player_2") {
        return MOB_TEAM_PLAYER_2;
    } else if(team_str == "player_3") {
        return MOB_TEAM_PLAYER_3;
    } else if(team_str == "player_4") {
        return MOB_TEAM_PLAYER_4;
    } else if(team_str == "enemy_1") {
        return MOB_TEAM_ENEMY_1;
    } else if(team_str == "enemy_2") {
        return MOB_TEAM_ENEMY_2;
    } else if(team_str == "enemy_3") {
        return MOB_TEAM_ENEMY_3;
    } else if(team_str == "enemy_4") {
        return MOB_TEAM_ENEMY_4;
    } else if(team_str == "obstacle") {
        return MOB_TEAM_OBSTACLE;
    } else if(team_str == "other") {
        return MOB_TEAM_OTHER;
    }
    return INVALID;
}
