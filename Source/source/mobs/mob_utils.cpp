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
#include <unordered_set>

#include "mob_utils.h"

#include "../functions.h"
#include "../game.h"
#include "../mob_script_action.h"
#include "../utils/string_utils.h"
#include "mob.h"


using std::unordered_set;
using std::size_t;


//Default distance at which the mob considers the chase finished.
const float chase_info_struct::DEF_TARGET_DISTANCE = 3.0f;


/* ----------------------------------------------------------------------------
 * Creates a structure with info about a carrying spot.
 * pos:
 *   The spot's relative coordinates.
 */
carrier_spot_struct::carrier_spot_struct(const point &pos) :
    state(CARRY_SPOT_FREE),
    pos(pos),
    pik_ptr(NULL) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a structure with info about carrying.
 * m:
 *   The mob this info belongs to.
 * destination:
 *   Where to deliver the mob. Use CARRY_DESTINATION_*.
 */
carry_info_struct::carry_info_struct(mob* m, const size_t destination) :
    m(m),
    destination(destination),
    cur_carrying_strength(0),
    cur_n_carriers(0),
    is_moving(false),
    intended_mob(nullptr),
    intended_pik_type(nullptr),
    must_return(false),
    return_dist(0) {
    
    for(size_t c = 0; c < m->type->max_carriers; ++c) {
        float angle = TAU / m->type->max_carriers * c;
        point p(
            cos(angle) * (m->radius + game.config.standard_pikmin_radius),
            sin(angle) * (m->radius + game.config.standard_pikmin_radius)
        );
        spot_info.push_back(carrier_spot_struct(p));
    }
}


/* ----------------------------------------------------------------------------
 * Returns true if the carriers can all fly, and thus, the object can
 * be carried through the air.
 */
bool carry_info_struct::can_fly() const {
    for(size_t c = 0; c < spot_info.size(); ++c) {
        mob* carrier_ptr = spot_info[c].pik_ptr;
        if(!carrier_ptr) continue;
        if(!spot_info[c].pik_ptr->can_move_in_midair) {
            return false;
        }
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Returns a list of hazards to which all carrier Pikmin are invulnerable.
 */
vector<hazard*> carry_info_struct::get_carrier_invulnerabilities() const {
    //First, get all types to save on the amount of hazard checks.
    unordered_set<mob_type*> carrier_types;
    for(size_t c = 0; c < spot_info.size(); ++c) {
        mob* carrier_ptr = spot_info[c].pik_ptr;
        if(!carrier_ptr) continue;
        carrier_types.insert(carrier_ptr->type);
    }
    
    //Now, count how many types are invulnerable to each detected hazard.
    map<hazard*, size_t> inv_instances;
    for(auto t : carrier_types) {
        for(auto h : t->hazard_vulnerabilities) {
            if(h.second.damage_mult == 0.0f) {
                inv_instances[h.first]++;
            }
        }
    }
    
    //Finally, only accept those that ALL types are invulnerable to.
    vector<hazard*> invulnerabilities;
    for(auto i : inv_instances) {
        if(i.second == carrier_types.size()) {
            invulnerabilities.push_back(i.first);
        }
    }
    
    return invulnerabilities;
}


/* ----------------------------------------------------------------------------
 * Returns the speed at which the object should move, given the carrier Pikmin.
 */
float carry_info_struct::get_speed() const {
    if(cur_n_carriers == 0) {
        return 0;
    }
    
    float max_speed = 0;
    
    //Begin by obtaining the average walking speed of the carriers.
    for(size_t s = 0; s < spot_info.size(); ++s) {
        const carrier_spot_struct* s_ptr = &spot_info[s];
        
        if(s_ptr->state != CARRY_SPOT_USED) continue;
        
        pikmin* p_ptr = (pikmin*) s_ptr->pik_ptr;
        max_speed += p_ptr->get_base_speed();
    }
    max_speed /= cur_n_carriers;
    
    //If the object has all carriers, the Pikmin move as fast
    //as possible, which looks bad, since they're not jogging,
    //they're carrying. Let's add a penalty for the weight...
    max_speed *= (1 - game.config.carrying_speed_weight_mult * m->type->weight);
    //...and a global carrying speed penalty.
    max_speed *= game.config.carrying_speed_max_mult;
    
    //The closer the mob is to having full carriers,
    //the closer to the max speed we get.
    //The speed goes from carrying_speed_base_mult (0 carriers)
    //to max_speed (all carriers).
    return
        max_speed * (
            game.config.carrying_speed_base_mult +
            (cur_n_carriers / (float) spot_info.size()) *
            (1 - game.config.carrying_speed_base_mult)
        );
}


/* ----------------------------------------------------------------------------
 * Returns true if no spot is reserved or used. False otherwise.
 */
bool carry_info_struct::is_empty() const {
    for(size_t s = 0; s < spot_info.size(); ++s) {
        if(spot_info[s].state != CARRY_SPOT_FREE) return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Returns true if all spots are reserved. False otherwise.
 */
bool carry_info_struct::is_full() const {
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
 * angle:
 *   Angle to rotate to.
 */
void carry_info_struct::rotate_points(const float angle) {
    for(size_t s = 0; s < spot_info.size(); ++s) {
        float s_angle = angle + (TAU / m->type->max_carriers * s);
        point p(
            cos(s_angle) *
            (m->radius + game.config.standard_pikmin_radius),
            sin(s_angle) *
            (m->radius + game.config.standard_pikmin_radius)
        );
        spot_info[s].pos = p;
    }
}


/* ----------------------------------------------------------------------------
 * Creates an instance of a structure with info about what the mob's chasing.
 */
chase_info_struct::chase_info_struct() :
    state(CHASE_STATE_STOPPED),
    flags(0),
    offset_z(0.0f),
    orig_coords(nullptr),
    orig_z(nullptr),
    target_dist(0.0f),
    cur_speed(0.0f),
    max_speed(-1.0f) {
    
}


/* ----------------------------------------------------------------------------
 * Creates an instance of a structure with info about the mob's circling.
 * m:
 *   Mob this circling info struct belongs to.
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
 * Creates a new delivery information struct.
 */
delivery_info_struct::delivery_info_struct() :
    intended_pik_type(nullptr) {
    
    color = game.config.carrying_color_move;
}


/* ----------------------------------------------------------------------------
 * Creates a new group information struct.
 * leader_ptr:
 *   Mob this group info struct belongs to.
 */
group_info_struct::group_info_struct(mob* leader_ptr) :
    radius(0),
    anchor(leader_ptr->pos),
    transform(game.identity_transform),
    cur_standby_type(nullptr),
    follow_mode(false) {
}


/* ----------------------------------------------------------------------------
 * Changes to a different standby subgroup type in case there are no more
 * Pikmin of the current one. Or to no type.
 */
void group_info_struct::change_standby_type_if_needed() {
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
 * Returns how many members of the given type exist in the group.
 * type:
 *   Type to check.
 */
size_t group_info_struct::get_amount_by_type(mob_type* type) const {
    size_t amount = 0;
    for(size_t m = 0; m < members.size(); ++m) {
        if(members[m]->type == type) {
            amount++;
        }
    }
    return amount;
}


/* ----------------------------------------------------------------------------
 * Returns the average position of the members.
 */
point group_info_struct::get_average_member_pos() const {
    point avg;
    for(size_t m = 0; m < members.size(); ++m) {
        avg += members[m]->pos;
    }
    return avg / members.size();
}


/* ----------------------------------------------------------------------------
 * Returns a point's offset from the anchor,
 * given the current group transformation.
 * spot_index:
 *   Index of the spot to check.
 */
point group_info_struct::get_spot_offset(const size_t spot_index) const {
    point res = spots[spot_index].pos;
    al_transform_coordinates(&transform, &res.x, &res.y);
    return res;
}


/* ----------------------------------------------------------------------------
 * (Re-)Initializes the group spots. This resizes it to the current number
 * of group members. Any old group members are moved to the appropriate
 * new spot.
 * affected_mob_ptr:
 *   If this initialization is because a new mob entered
 *   or left the group, this should point to said mob.
 */
void group_info_struct::init_spots(mob* affected_mob_ptr) {
    const float SPOT_MAX_DEVIATION = GROUP_SPOT_INTERVAL * 0.60f;
    
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
    radius = game.config.standard_pikmin_radius;
    
    //Center spot first.
    alpha_spots.push_back(alpha_spot(point()));
    
    while(alpha_spots.size() < members.size()) {
    
        //First, calculate how far the center
        //of these spots are from the central spot.
        float dist_from_center =
            game.config.standard_pikmin_radius * current_wheel + //Spots.
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
            game.config.standard_pikmin_radius * 2.0 + GROUP_SPOT_INTERVAL;
            
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
                        randomf(-SPOT_MAX_DEVIATION, SPOT_MAX_DEVIATION),
                        dist_from_center * sin(angle * s) +
                        randomf(-SPOT_MAX_DEVIATION, SPOT_MAX_DEVIATION)
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
void group_info_struct::reassign_spots() {
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
 * move_backwards:
 *   If true, go through the list backwards.
 */
bool group_info_struct::set_next_cur_standby_type(const bool move_backwards) {

    if(members.empty()) {
        cur_standby_type = NULL;
        return true;
    }
    
    bool success = false;
    subgroup_type* starting_type = cur_standby_type;
    subgroup_type* final_type = cur_standby_type;
    if(!starting_type) {
        starting_type =
            game.states.gameplay->subgroup_types.get_first_type();
    }
    subgroup_type* scanning_type = starting_type;
    subgroup_type* leader_subgroup_type =
        game.states.gameplay->subgroup_types.get_type(
            SUBGROUP_TYPE_CATEGORY_LEADER
        );
        
    if(move_backwards) {
        scanning_type =
            game.states.gameplay->subgroup_types.get_prev_type(
                scanning_type
            );
    } else {
        scanning_type =
            game.states.gameplay->subgroup_types.get_next_type(
                scanning_type
            );
    }
    while(scanning_type != starting_type && !success) {
        //For each type, let's check if there's any group member that matches.
        if(
            scanning_type == leader_subgroup_type &&
            !game.config.can_throw_leaders
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
            scanning_type =
                game.states.gameplay->subgroup_types.get_prev_type(
                    scanning_type
                );
        } else {
            scanning_type =
                game.states.gameplay->subgroup_types.get_next_type(
                    scanning_type
                );
        }
    }
    
    cur_standby_type = final_type;
    return success;
}


/* ----------------------------------------------------------------------------
 * Sorts the group with the specified type at the front, and the other types
 * (in order) behind.
 * leading_type:
 *   The subgroup type that will be at the front of the group.
 */
void group_info_struct::sort(subgroup_type* leading_type) {

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
            cur_type =
                game.states.gameplay->subgroup_types.get_next_type(cur_type);
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
 * final_z:
 *   The Z coordinate is returned here.
 */
point hold_info_struct::get_final_pos(float* final_z) const {
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
                offset_dist * m->radius
            );
        *final_z = m->z;
    }
    
    return final_pos;
}



/* ----------------------------------------------------------------------------
 * Initializes a parent mob information struct.
 * m:
 *   The parent mob.
 */
parent_info_struct::parent_info_struct(mob* m) :
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
 * m:
 *   Mob this path info struct belongs to.
 * target:
 *   Its target destination.
 * settings:
 *   Settings about how the path should be followed.
 */
path_info_struct::path_info_struct(
    mob* m,
    const point &target,
    const path_follow_settings &settings
) :
    m(m),
    target_point(target),
    target_mob(nullptr),
    cur_path_stop_nr(0),
    go_straight(false),
    is_blocked(false),
    settings(settings) {
    
    path =
        get_path(
            m->pos, target, settings,
            &go_straight, NULL, NULL, NULL
        );
}


/* ----------------------------------------------------------------------------
 * Calculates whether or not the way forward is currently blocked.
 */
bool path_info_struct::check_blockage() {
    if(
        path.size() >= 2 &&
        cur_path_stop_nr > 0 &&
        cur_path_stop_nr < path.size()
    ) {
        path_stop* cur_stop = path[cur_path_stop_nr - 1];
        path_stop* next_stop = path[cur_path_stop_nr];
        
        return
            !can_traverse_path_link(
                cur_stop->get_link(next_stop),
                settings
            );
    }
    return false;
}


//Wait these many seconds before allowing another Pikmin to be called out.
const float pikmin_nest_struct::CALL_INTERVAL = 0.01f;

/* ----------------------------------------------------------------------------
 * Creates an instance of a class with info about a mob that
 * can nest Pikmin inside.
 * m_ptr:
 *   Nest mob responsible.
 * type:
 *   Type of nest.
 */
pikmin_nest_struct::pikmin_nest_struct(
    mob* m_ptr, pikmin_nest_type_struct* type
) :
    m_ptr(m_ptr),
    nest_type(type),
    calling_leader(nullptr),
    next_call_time(0.0f) {
    
    for(size_t t = 0; t < nest_type->pik_types.size(); ++t) {
        pikmin_inside.push_back(vector<size_t>(N_MATURITIES, 0));
        call_queue.push_back(0);
    }
}


/* ----------------------------------------------------------------------------
 * Calls out a Pikmin from inside the nest, if possible.
 * Gives priority to the higher maturities.
 * Returns true if a Pikmin was spawned, false otherwise.
 * m_ptr:
 *   Pointer to the nest mob.
 * type_idx:
 *   Index of the Pikmin type, from the types this nest manages.
 */
bool pikmin_nest_struct::call_pikmin(mob* m_ptr, const size_t type_idx) {
    if(
        game.states.gameplay->mobs.pikmin_list.size() >=
        game.config.max_pikmin_in_field
    ) {
        return false;
    }
    
    for(size_t m = 0; m < N_MATURITIES; ++m) {
        //Let's check the maturities in reverse order.
        size_t cur_m = N_MATURITIES - m - 1;
        
        if(pikmin_inside[type_idx][cur_m] == 0) continue;
        
        //Spawn the Pikmin!
        //Update the Pikmin count.
        pikmin_inside[type_idx][cur_m]--;
        
        //Decide a leg to come out of.
        size_t leg_idx =
            randomi(0, (nest_type->leg_body_parts.size() / 2) - 1);
        size_t leg_hole_bp_idx =
            m_ptr->anim.anim_db->find_body_part(
                nest_type->leg_body_parts[leg_idx * 2]
            );
        size_t leg_foot_bp_idx =
            m_ptr->anim.anim_db->find_body_part(
                nest_type->leg_body_parts[leg_idx * 2 + 1]
            );
        point spawn_coords =
            m_ptr->get_hitbox(leg_hole_bp_idx)->get_cur_pos(
                m_ptr->pos, m_ptr->angle
            );
        float spawn_angle =
            get_angle(m_ptr->pos, spawn_coords);
            
        //Create the Pikmin.
        pikmin* new_pikmin =
            (pikmin*)
            create_mob(
                game.mob_categories.get(MOB_CATEGORY_PIKMIN),
                spawn_coords, nest_type->pik_types[type_idx], spawn_angle,
                "maturity=" + i2s(cur_m)
            );
            
        //Set its data to start sliding.
        new_pikmin->fsm.set_state(PIKMIN_STATE_LEAVING_ONION, (void*) this);
        vector<size_t> checkpoints;
        checkpoints.push_back(leg_hole_bp_idx);
        checkpoints.push_back(leg_foot_bp_idx);
        new_pikmin->track_info =
            new track_info_struct(
            m_ptr, checkpoints, nest_type->pikmin_exit_speed
        );
        new_pikmin->leader_to_return_to = calling_leader;
        
        return true;
    }
    
    return false;
}


/* ----------------------------------------------------------------------------
 * Returns how many Pikmin of the given type exist inside.
 * type:
 *   Type to check.
 */
size_t pikmin_nest_struct::get_amount_by_type(pikmin_type* type) {
    size_t amount = 0;
    for(size_t t = 0; t < nest_type->pik_types.size(); ++t) {
        if(nest_type->pik_types[t] == type) {
            for(size_t m = 0; m < N_MATURITIES; ++m) {
                amount += pikmin_inside[t][m];
            }
            break;
        }
    }
    return amount;
}


/* ----------------------------------------------------------------------------
 * Reads the provided script variables, if any, and does stuff with
 * any that are related to nests.
 * svr:
 *   Script var reader to use.
 */
void pikmin_nest_struct::read_script_vars(const script_var_reader &svr) {
    string pikmin_inside_var;
    
    if(svr.get("pikmin_inside", pikmin_inside_var)) {
        vector<string> pikmin_inside_vars = split(pikmin_inside_var);
        size_t word = 0;
        
        for(size_t t = 0; t < nest_type->pik_types.size(); ++t) {
            for(size_t m = 0; m < N_MATURITIES; ++m) {
                if(word < pikmin_inside_vars.size()) {
                    pikmin_inside[t][m] = s2i(pikmin_inside_vars[word]);
                    word++;
                }
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Requests that Pikmin of the given type get called out.
 * type_idx:
 *   Index of the type of Pikmin to call out, from the nest's types.
 * amount:
 *   How many to call out.
 * l_ptr:
 *   Leader responsible.
 */
void pikmin_nest_struct::request_pikmin(
    const size_t type_idx, const size_t amount, leader* l_ptr
) {
    call_queue[type_idx] += amount;
    next_call_time = CALL_INTERVAL;
    calling_leader = l_ptr;
}


/* ----------------------------------------------------------------------------
 * Stores the given Pikmin inside the nest. This basically deletes the
 * Pikmin and updates the amount inside the nest.
 * p_ptr:
 *   Pikmin to store.
 */
void pikmin_nest_struct::store_pikmin(pikmin* p_ptr) {
    for(size_t t = 0; t < nest_type->pik_types.size(); ++t) {
        if(p_ptr->type == nest_type->pik_types[t]) {
            pikmin_inside[t][p_ptr->maturity]++;
            break;
        }
    }
    
    p_ptr->to_delete = true;
}


/* ----------------------------------------------------------------------------
 * Ticks one frame of logic.
 * delta_t:
 *   Time to tick by.
 */
void pikmin_nest_struct::tick(const float delta_t) {
    if(calling_leader && calling_leader->to_delete) {
        calling_leader = NULL;
    }
    
    //Call out Pikmin, if the timer agrees.
    if(next_call_time > 0.0f) {
        next_call_time -= delta_t;
    }
    
    while(next_call_time < 0.0f) {
        size_t best_type = INVALID;
        size_t best_type_amount = 0;
        
        for(size_t t = 0; t < nest_type->pik_types.size(); ++t) {
            if(call_queue[t] == 0) continue;
            if(call_queue[t] > best_type_amount) {
                best_type = t;
                best_type_amount = call_queue[t];
            }
        }
        
        if(best_type != INVALID) {
            //Try to call a Pikmin.
            if(call_pikmin(m_ptr, best_type)) {
                //Call successful! Update the queue.
                call_queue[best_type]--;
            } else {
                //Call failed. Forget the player's request.
                call_queue[best_type] = 0;
            }
        }
        
        next_call_time += CALL_INTERVAL;
    }
}


/* ----------------------------------------------------------------------------
 * Creates an instance of a class with info about a mob type that
 * can nest Pikmin inside.
 */
pikmin_nest_type_struct::pikmin_nest_type_struct() :
    pikmin_enter_speed(0.7f),
    pikmin_exit_speed(2.0f) {
    
}


/* ----------------------------------------------------------------------------
 * Loads nest-related properties from a data file.
 * file:
 *   File to read from.
 */
void pikmin_nest_type_struct::load_properties(
    data_node* file
) {
    reader_setter rs(file);
    
    string pik_types_str;
    string legs_str;
    data_node* pik_types_node = NULL;
    data_node* legs_node = NULL;
    
    rs.set("leg_body_parts", legs_str, &legs_node);
    rs.set("pikmin_types", pik_types_str, &pik_types_node);
    rs.set("pikmin_enter_speed", pikmin_enter_speed);
    rs.set("pikmin_exit_speed", pikmin_exit_speed);
    
    leg_body_parts = semicolon_list_to_vector(legs_str);
    if(pik_types_node && leg_body_parts.empty()) {
        log_error(
            "A nest-like object type needs a list of leg body parts!",
            file
        );
    } else if(legs_node && leg_body_parts.size() % 2 == 1) {
        log_error(
            "A nest-like object type needs an even number of leg body parts!",
            legs_node
        );
    }
    
    vector<string> pik_types_strs = semicolon_list_to_vector(pik_types_str);
    for(size_t t = 0; t < pik_types_strs.size(); ++t) {
        string &str = pik_types_strs[t];
        if(
            game.mob_types.pikmin.find(str) ==
            game.mob_types.pikmin.end()
        ) {
            log_error(
                "Unknown Pikmin type \"" + str + "\"!",
                pik_types_node
            );
        } else {
            pik_types.push_back(game.mob_types.pikmin[str]);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Creates an instance of a structure with info about the track the mob
 * is riding.
 * m:
 *   Mob this track info struct belongs to.
 * checkpoints:
 *   List of checkpoints (body part indexes) to cross.
 * ride_speed:
 *  Speed to ride at, in ratio per second.
 */
track_info_struct::track_info_struct(
    mob* m, const vector<size_t> checkpoints, const float ride_speed
) :
    m(m),
    checkpoints(checkpoints),
    cur_cp_nr(0),
    cur_cp_progress(0.0f),
    ride_speed(ride_speed) {
    
}


/* ----------------------------------------------------------------------------
 * Calculates the maximum span that a mob can ever reach from its center.
 * radius:
 *   The mob's radius.
 * anim_max_span:
 *   Maximum span of its animation-related data.
 * rectangular_dim:
 *   Rectangular dimensions of the mob, if any.
 */
float calculate_mob_max_span(
    const float radius, const float anim_max_span, const point &rectangular_dim
) {
    float max_span = std::max(radius, anim_max_span);
    
    if(rectangular_dim.x != 0) {
        max_span =
            std::max(
                max_span, dist(point(0, 0), rectangular_dim / 2.0).to_float()
            );
    }
    
    return max_span;
}


/* ----------------------------------------------------------------------------
 * Creates a mob, adding it to the corresponding vectors.
 * Returns the new mob.
 * category:
 *   The category the new mob belongs to.
 * pos:
 *   Initial position.
 * type:
 *   Type of the new mob.
 * angle:
 *   Initial facing angle.
 * vars:
 *   Script variables.
 * code_after_creation:
 *   Code to run right after the mob is created, if any.
 *   This is run before any scripting takes place.
 * first_state_override:
 *   If this is INVALID, use the first state number defined in the mob's
 *   FSM struct, or the standard first state number. Otherwise, use this.
 */
mob* create_mob(
    mob_category* category, const point &pos, mob_type* type,
    const float angle, const string &vars,
    std::function<void(mob*)> code_after_creation,
    const size_t first_state_override
) {
    mob* m_ptr = category->create_mob(pos, type, angle);
    
    if(code_after_creation) {
        code_after_creation(m_ptr);
    }
    
    for(size_t a = 0; a < type->init_actions.size(); ++a) {
        type->init_actions[a]->run(m_ptr, NULL, NULL);
    }
    
    if(!vars.empty()) {
        map<string, string> vars_map = get_var_map(vars);
        script_var_reader svr(vars_map);
        
        m_ptr->read_script_vars(svr);
        
        for(auto &v : vars_map) {
            m_ptr->vars[v.first] = v.second;
        }
    }
    
    if(
        !m_ptr->fsm.set_state(
            first_state_override != INVALID ?
            first_state_override :
            m_ptr->fsm.first_state_override != INVALID ?
            m_ptr->fsm.first_state_override :
            type->first_state_nr
        )
    ) {
        //If something went wrong, give it some dummy state.
        m_ptr->fsm.cur_state = game.dummy_mob_state;
    };
    
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
                "does not exist in the list of spawn data!"
            );
            continue;
        }
        
        mob* new_mob = m_ptr->spawn(spawn_info);
        
        if(!new_mob) continue;
        
        parent_info_struct* p_info = new parent_info_struct(m_ptr);
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
    
    game.states.gameplay->mobs.all.push_back(m_ptr);
    return m_ptr;
}


/* ----------------------------------------------------------------------------
 * Deletes a mob from the relevant vectors.
 * It's always removed from the vector of mobs, but it's
 * also removed from the vector of Pikmin if it's a Pikmin,
 * leaders if it's a leader, etc.
 * m_ptr:
 *   The mob to delete.
 * complete_destruction:
 *   If true, don't bother removing it from groups and such,
 *   since everything is going to be destroyed.
 */
void delete_mob(mob* m_ptr, const bool complete_destruction) {
    if(game.maker_tools.info_lock == m_ptr) game.maker_tools.info_lock = NULL;
    
    if(!complete_destruction) {
        m_ptr->leave_group();
        
        for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); ++m) {
            mob* m2_ptr = game.states.gameplay->mobs.all[m];
            if(m2_ptr->focused_mob == m_ptr) {
                m2_ptr->fsm.run_event(MOB_EV_FOCUSED_MOB_UNAVAILABLE);
                m2_ptr->fsm.run_event(MOB_EV_FOCUS_OFF_REACH);
                m2_ptr->fsm.run_event(MOB_EV_FOCUS_DIED);
                m2_ptr->focused_mob = NULL;
            }
            if(m2_ptr->parent && m2_ptr->parent->m == m_ptr) {
                delete m2_ptr->parent;
                m2_ptr->parent = NULL;
                m2_ptr->to_delete = true;
            }
            for(size_t f = 0; f < m2_ptr->focused_mob_memory.size(); ++f) {
                if(m2_ptr->focused_mob_memory[f] == m_ptr) {
                    m2_ptr->focused_mob_memory[f] = NULL;
                }
            }
            if(m2_ptr->stored_inside_another == m_ptr) {
                m_ptr->release(m2_ptr);
                m2_ptr->stored_inside_another = NULL;
            }
        }
        
        while(!m_ptr->holding.empty()) {
            m_ptr->release(m_ptr->holding[0]);
        }
        
        m_ptr->set_can_block_paths(false);
        
        m_ptr->fsm.set_state(INVALID);
    }
    
    m_ptr->type->category->erase_mob(m_ptr);
    game.states.gameplay->mobs.all.erase(
        find(
            game.states.gameplay->mobs.all.begin(),
            game.states.gameplay->mobs.all.end(),
            m_ptr
        )
    );
    
    delete m_ptr;
}


/* ----------------------------------------------------------------------------
 * Returns a string that describes the given mob. Used in error messages
 * where you have to indicate a specific mob in the area.
 * m:
 *   The mob.
 */
string get_error_message_mob_info(mob* m) {
    return
        "type \"" + m->type->name + "\", coordinates " +
        p2s(m->pos) + ", area \"" + game.cur_area_data.name + "\"";
}


/* ----------------------------------------------------------------------------
 * Converts a string to the numeric representation of a mob target type.
 * Returns INVALID if the string is not valid.
 * type_str:
 *   Text representation of the target type.
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
 * team_str:
 *   Text representation of the team.
 */
size_t string_to_team_nr(const string &team_str) {
    for(size_t t = 0; t < N_MOB_TEAMS; ++t) {
        if(team_str == game.team_internal_names[t]) {
            return t;
        }
    }
    return INVALID;
}
