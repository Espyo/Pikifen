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

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"
#include "../other/mob_script_action.h"
#include "mob.h"


using std::unordered_set;
using std::size_t;


/**
 * @brief Constructs a new carrier spot struct object.
 *
 * @param pos The spot's relative coordinates.
 */
carrier_spot_t::carrier_spot_t(const point &pos) :
    pos(pos) {
    
}


/**
 * @brief Constructs a new carry info struct object.
 *
 * @param m The mob this info belongs to.
 * @param destination Where to deliver the mob.
 */
carry_t::carry_t(
    mob* m, const CARRY_DESTINATION destination
) :
    m(m),
    destination(destination) {
    
    for(size_t c = 0; c < m->type->max_carriers; c++) {
        point p;
        if(m->type->custom_carry_spots.empty()) {
            float angle = TAU / m->type->max_carriers * c;
            p =
                point(
                    cos(angle) *
                    (m->radius + game.config.standard_pikmin_radius),
                    sin(angle) *
                    (m->radius + game.config.standard_pikmin_radius)
                );
        } else {
            p = m->type->custom_carry_spots[c];
        }
        spot_info.push_back(carrier_spot_t(p));
    }
}


/**
 * @brief Returns true if the carriers can all fly, and thus, the object can
 * be carried through the air.
 *
 * @return Whether it can fly.
 */
bool carry_t::can_fly() const {
    for(size_t c = 0; c < spot_info.size(); c++) {
        mob* carrier_ptr = spot_info[c].pik_ptr;
        if(!carrier_ptr) continue;
        if(!has_flag(spot_info[c].pik_ptr->flags, MOB_FLAG_CAN_MOVE_MIDAIR)) {
            return false;
        }
    }
    return true;
}


/**
 * @brief Returns a list of hazards to which all carrier Pikmin
 * are invulnerable.
 *
 * @return The invulnerabilities.
 */
vector<hazard*> carry_t::get_carrier_invulnerabilities() const {
    //Get all types to save on the amount of hazard checks.
    unordered_set<mob_type*> carrier_types;
    for(size_t c = 0; c < spot_info.size(); c++) {
        mob* carrier_ptr = spot_info[c].pik_ptr;
        if(!carrier_ptr) continue;
        carrier_types.insert(carrier_ptr->type);
    }
    
    return get_mob_type_list_invulnerabilities(carrier_types);
}


/**
 * @brief Returns the speed at which the object should move,
 * given the carrier Pikmin.
 *
 * @return The speed.
 */
float carry_t::get_speed() const {
    if(cur_n_carriers == 0) {
        return 0;
    }
    
    float max_speed = 0;
    
    //Begin by obtaining the average walking speed of the carriers.
    for(size_t s = 0; s < spot_info.size(); s++) {
        const carrier_spot_t* s_ptr = &spot_info[s];
        
        if(s_ptr->state != CARRY_SPOT_STATE_USED) continue;
        
        pikmin* p_ptr = (pikmin*) s_ptr->pik_ptr;
        max_speed += p_ptr->get_base_speed() * p_ptr->get_speed_multiplier();
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


/**
 * @brief Returns true if no spot is reserved or used. False otherwise.
 *
 * @return Whether it is empty.
 */
bool carry_t::is_empty() const {
    for(size_t s = 0; s < spot_info.size(); s++) {
        if(spot_info[s].state != CARRY_SPOT_STATE_FREE) return false;
    }
    return true;
}


/**
 * @brief Returns true if all spots are reserved. False otherwise.
 *
 * @return Whether it is full.
 */
bool carry_t::is_full() const {
    for(size_t s = 0; s < spot_info.size(); s++) {
        if(spot_info[s].state == CARRY_SPOT_STATE_FREE) return false;
    }
    return true;
}


/**
 * @brief Rotates all points in the struct, making it so spot 0 faces
 * the specified angle away from the mob.
 * This is useful when the first Pikmin is coming, to make the first carry
 * spot be closer to that Pikmin.
 *
 * @param angle Angle to rotate to.
 */
void carry_t::rotate_points(float angle) {
    for(size_t s = 0; s < spot_info.size(); s++) {
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


/**
 * @brief Constructs a new circling info struct object.
 *
 * @param m Mob this circling info struct belongs to.
 */
circling_t::circling_t(mob* m) :
    m(m) {
    
}


/**
 * @brief Constructs a new delivery info struct object.
 */
delivery_t::delivery_t() :
    color(game.config.carrying_color_move) {
}


/**
 * @brief Constructs a new group info struct object.
 *
 * @param leader_ptr Mob this group info struct belongs to.
 */
group_t::group_t(mob* leader_ptr) :
    anchor(leader_ptr->pos),
    transform(game.identity_transform) {
}


/**
 * @brief Sets the standby group member type to the next available one,
 * or nullptr if none.
 *
 * @param move_backwards If true, go through the list backwards.
 * @return Whether it succeeded.
 */
bool group_t::change_standby_type(bool move_backwards) {
    return get_next_standby_type(move_backwards, &cur_standby_type);
}


/**
 * @brief Changes to a different standby subgroup type in case there are no more
 * Pikmin of the current one. Or to no type.
 */
void group_t::change_standby_type_if_needed() {
    for(size_t m = 0; m < members.size(); m++) {
        if(members[m]->subgroup_type_ptr == cur_standby_type) {
            //Never mind, there is a member of this subgroup type.
            return;
        }
    }
    //No members of the current type? Switch to the next.
    change_standby_type(false);
}


/**
 * @brief Returns how many members of the given type exist in the group.
 *
 * @param type Type to check.
 * @return The amount.
 */
size_t group_t::get_amount_by_type(const mob_type* type) const {
    size_t amount = 0;
    for(size_t m = 0; m < members.size(); m++) {
        if(members[m]->type == type) {
            amount++;
        }
    }
    return amount;
}


/**
 * @brief Returns the average position of the members.
 *
 * @return The average position.
 */
point group_t::get_average_member_pos() const {
    point avg;
    for(size_t m = 0; m < members.size(); m++) {
        avg += members[m]->pos;
    }
    return avg / members.size();
}


/**
 * @brief Returns a list of hazards to which all of a leader's group mobs
 * are invulnerable.
 *
 * @param include_leader If not nullptr, include the group leader mob.
 * @return The list of invulnerabilities.
 */
vector<hazard*> group_t::get_group_invulnerabilities(
    mob* include_leader
) const {
    //Get all types to save on the amount of hazard checks.
    unordered_set<mob_type*> member_types;
    for(size_t m = 0; m < members.size(); m++) {
        mob* member_ptr = members[m];
        if(!member_ptr) continue;
        member_types.insert(member_ptr->type);
    }
    
    if(include_leader) member_types.insert(include_leader->type);
    
    return get_mob_type_list_invulnerabilities(member_types);
}


/**
 * @brief Returns the next available standby group member type, or nullptr if none.
 *
 * @param move_backwards If true, go through the list backwards.
 * @param new_type The new type is returned here.
 * @return Whether it succeeded.
 */
bool group_t::get_next_standby_type(
    bool move_backwards, subgroup_type** new_type
) {

    if(members.empty()) {
        *new_type = nullptr;
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
            for(size_t m = 0; m < members.size(); m++) {
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
    
    *new_type = final_type;
    return success;
}


/**
 * @brief Returns a point's offset from the anchor,
 * given the current group transformation.
 *
 * @param spot_idx Index of the spot to check.
 * @return The offset.
 */
point group_t::get_spot_offset(size_t spot_idx) const {
    point res = spots[spot_idx].pos;
    al_transform_coordinates(&transform, &res.x, &res.y);
    return res;
}


/**
 * @brief (Re-)Initializes the group spots. This resizes it to the current
 * number of group members. Any old group members are moved to the appropriate
 * new spot.
 *
 * @param affected_mob_ptr If this initialization is because a new mob entered
 * or left the group, this should point to said mob.
 */
void group_t::init_spots(mob* affected_mob_ptr) {
    if(members.empty()) {
        spots.clear();
        radius = 0;
        return;
    }
    
    //First, backup the old mob indexes.
    vector<mob*> old_mobs;
    old_mobs.resize(spots.size());
    for(size_t m = 0; m < spots.size(); m++) {
        old_mobs[m] = spots[m].mob_ptr;
    }
    
    //Now, rebuild the spots. Let's draw wheels from the center, for now.
    
    /**
     * @brief Initial spot.
     */
    struct alpha_spot {
    
        //--- Members ---
        
        //Position of the spot.
        point pos;
        
        //How far away it is from the rightmost spot.
        dist distance_to_rightmost;
        
        
        //--- Function definitions ---
        
        /**
         * @brief Constructs a new alpha spot object.
         *
         * @param p The position.
         */
        explicit alpha_spot(const point &p) :
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
            MOB::GROUP_SPOT_INTERVAL * current_wheel; //Interval between spots.
            
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
            game.config.standard_pikmin_radius * 2.0 + MOB::GROUP_SPOT_INTERVAL;
            
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
        
        for(unsigned s = 0; s < n_spots_on_wheel; s++) {
            alpha_spots.push_back(
                alpha_spot(
                    point(
                        dist_from_center * cos(angle * s) +
                        randomf(
                            -MOB::GROUP_SPOT_MAX_DEVIATION,
                            MOB::GROUP_SPOT_MAX_DEVIATION
                        ),
                        dist_from_center * sin(angle * s) +
                        randomf(
                            -MOB::GROUP_SPOT_MAX_DEVIATION,
                            MOB::GROUP_SPOT_MAX_DEVIATION
                        )
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
    for(size_t a = 0; a < alpha_spots.size(); a++) {
        alpha_spots[a].distance_to_rightmost =
            dist(
                alpha_spots[a].pos,
                point(radius, 0)
            );
    }
    
    std::sort(
        alpha_spots.begin(), alpha_spots.end(),
    [] (const alpha_spot & a1, const alpha_spot & a2) -> bool {
        return a1.distance_to_rightmost < a2.distance_to_rightmost;
    }
    );
    
    //Finally, create the group spots.
    spots.clear();
    spots.resize(members.size(), group_spot());
    for(size_t s = 0; s < members.size(); s++) {
        spots[s] =
            group_spot(
                point(
                    alpha_spots[s].pos.x - radius,
                    alpha_spots[s].pos.y
                ),
                nullptr
            );
    }
    
    //Pass the old mobs over.
    if(old_mobs.size() < spots.size()) {
        for(size_t m = 0; m < old_mobs.size(); m++) {
            spots[m].mob_ptr = old_mobs[m];
            spots[m].mob_ptr->group_spot_idx = m;
        }
        spots[old_mobs.size()].mob_ptr = affected_mob_ptr;
        affected_mob_ptr->group_spot_idx = old_mobs.size();
        
    } else if(old_mobs.size() > spots.size()) {
        for(size_t m = 0, s = 0; m < old_mobs.size(); m++) {
            if(old_mobs[m] == affected_mob_ptr) {
                old_mobs[m]->group_spot_idx = INVALID;
                continue;
            }
            spots[s].mob_ptr = old_mobs[m];
            spots[s].mob_ptr->group_spot_idx = s;
            s++;
        }
        
    } else {
        for(size_t m = 0; m < old_mobs.size(); m++) {
            spots[m].mob_ptr = old_mobs[m];
            spots[m].mob_ptr->group_spot_idx = m;
        }
    }
}


/**
 * @brief Assigns each mob a new spot, given how close each one of them is to
 * each spot.
 */
void group_t::reassign_spots() {
    for(size_t m = 0; m < members.size(); m++) {
        members[m]->group_spot_idx = INVALID;
    }
    
    for(size_t s = 0; s < spots.size(); s++) {
        point spot_pos = anchor + get_spot_offset(s);
        mob* closest_mob = nullptr;
        dist closest_dist;
        
        for(size_t m = 0; m < members.size(); m++) {
            mob* m_ptr = members[m];
            if(m_ptr->group_spot_idx != INVALID) continue;
            
            dist d(m_ptr->pos, spot_pos);
            
            if(!closest_mob || d < closest_dist) {
                closest_mob = m_ptr;
                closest_dist = d;
            }
        }
        
        if(closest_mob) closest_mob->group_spot_idx = s;
    }
}


/**
 * @brief Sorts the group with the specified type at the front, and the
 * other types (in order) behind.
 *
 * @param leading_type The subgroup type that will be at the front of
 * the group.
 */
void group_t::sort(subgroup_type* leading_type) {

    for(size_t m = 0; m < members.size(); m++) {
        members[m]->group_spot_idx = INVALID;
    }
    
    subgroup_type* cur_type = leading_type;
    size_t cur_spot = 0;
    
    while(cur_spot != spots.size()) {
        point spot_pos = anchor + get_spot_offset(cur_spot);
        
        //Find the member closest to this spot.
        mob* closest_member = nullptr;
        dist closest_dist;
        for(size_t m = 0; m < members.size(); m++) {
            mob* m_ptr = members[m];
            if(m_ptr->subgroup_type_ptr != cur_type) continue;
            if(m_ptr->group_spot_idx != INVALID) continue;
            
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
            closest_member->group_spot_idx = cur_spot;
            cur_spot++;
        }
        
    }
    
}


/**
 * @brief Clears the information.
 */
void hold_t::clear() {
    m = nullptr;
    hitbox_idx = INVALID;
    offset_dist = 0;
    offset_angle = 0;
    vertical_dist = 0;
}


/**
 * @brief Returns the final coordinates this mob should be at.
 *
 * @param out_z The Z coordinate is returned here.
 * @return The (X and Y) coordinates.
 */
point hold_t::get_final_pos(float* out_z) const {
    if(!m) return point();
    
    hitbox* h_ptr = nullptr;
    if(hitbox_idx != INVALID) {
        h_ptr = m->get_hitbox(hitbox_idx);
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
        *out_z = m->z + h_ptr->z + (h_ptr->height * vertical_dist);
    } else {
        //Body center.
        final_pos = m->pos;
        
        final_pos +=
            angle_to_coordinates(
                offset_angle + m->angle,
                offset_dist * m->radius
            );
        *out_z = m->z + (m->height * vertical_dist);
    }
    
    return final_pos;
}


/**
 * @brief Constructs a new parent info struct object.
 *
 * @param m The parent mob.
 */
parent_t::parent_t(mob* m) :
    m(m) {
    
}


/**
 * @brief Constructs a new path info struct object.
 *
 * @param m Mob this path info struct belongs to.
 * @param settings Settings about how the path should be followed.
 */
path_t::path_t(
    mob* m,
    const path_follow_settings &settings
) :
    m(m),
    settings(settings) {
    
    result =
        get_path(
            m->pos, settings.target_point, settings,
            path, nullptr, nullptr, nullptr
        );
}


/**
 * @brief Calculates whether or not the way forward is currently blocked.
 *
 * @param out_reason If not nullptr, the reason is returned here.
 * @return Whether there is a blockage.
 */
bool path_t::check_blockage(PATH_BLOCK_REASON* out_reason) {
    if(
        path.size() >= 2 &&
        cur_path_stop_idx > 0 &&
        cur_path_stop_idx < path.size()
    ) {
        path_stop* cur_stop = path[cur_path_stop_idx - 1];
        path_stop* next_stop = path[cur_path_stop_idx];
        
        return
            !can_traverse_path_link(
                cur_stop->get_link(next_stop),
                settings,
                out_reason
            );
    }
    
    if(out_reason) *out_reason = PATH_BLOCK_REASON_NONE;
    return false;
}


/**
 * @brief Constructs a new Pikmin nest struct object.
 *
 * @param m_ptr Nest mob responsible.
 * @param type Type of nest.
 */
pikmin_nest_t::pikmin_nest_t(
    mob* m_ptr, pikmin_nest_type_t* type
) :
    m_ptr(m_ptr),
    nest_type(type) {
    
    for(size_t t = 0; t < nest_type->pik_types.size(); t++) {
        pikmin_inside.push_back(vector<size_t>(N_MATURITIES, 0));
        call_queue.push_back(0);
    }
}


/**
 * @brief Calls out a Pikmin from inside the nest, if possible.
 * Gives priority to the higher maturities.
 *
 * @param m_ptr Pointer to the nest mob.
 * @param type_idx Index of the Pikmin type, from the types this nest manages.
 * @return Whether a Pikmin spawned.
 */
bool pikmin_nest_t::call_pikmin(mob* m_ptr, size_t type_idx) {
    if(
        game.states.gameplay->mobs.pikmin_list.size() >=
        game.config.max_pikmin_in_field
    ) {
        return false;
    }
    
    for(size_t m = 0; m < N_MATURITIES; m++) {
        //Let's check the maturities in reverse order.
        size_t cur_m = N_MATURITIES - m - 1;
        
        if(pikmin_inside[type_idx][cur_m] == 0) continue;
        
        //Spawn the Pikmin!
        //Update the Pikmin count.
        pikmin_inside[type_idx][cur_m]--;
        
        //Decide a leg to come out of.
        size_t leg_idx =
            randomi(0, (int) (nest_type->leg_body_parts.size() / 2) - 1);
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
            new track_t(
            m_ptr, checkpoints, nest_type->pikmin_exit_speed
        );
        new_pikmin->leader_to_return_to = calling_leader;
        
        return true;
    }
    
    return false;
}


/**
 * @brief Returns how many Pikmin of the given type exist inside.
 *
 * @param type Type to check.
 * @return The amount.
 */
size_t pikmin_nest_t::get_amount_by_type(const pikmin_type* type) {
    size_t amount = 0;
    for(size_t t = 0; t < nest_type->pik_types.size(); t++) {
        if(nest_type->pik_types[t] == type) {
            for(size_t m = 0; m < N_MATURITIES; m++) {
                amount += pikmin_inside[t][m];
            }
            break;
        }
    }
    return amount;
}


/**
 * @brief Reads the provided script variables, if any, and does stuff with
 * any that are related to nests.
 *
 * @param svr Script var reader to use.
 */
void pikmin_nest_t::read_script_vars(const script_var_reader &svr) {
    string pikmin_inside_var;
    
    if(svr.get("pikmin_inside", pikmin_inside_var)) {
        vector<string> pikmin_inside_vars = split(pikmin_inside_var);
        size_t word = 0;
        
        for(size_t t = 0; t < nest_type->pik_types.size(); t++) {
            for(size_t m = 0; m < N_MATURITIES; m++) {
                if(word < pikmin_inside_vars.size()) {
                    pikmin_inside[t][m] = s2i(pikmin_inside_vars[word]);
                    word++;
                }
            }
        }
    }
}


/**
 * @brief Requests that Pikmin of the given type get called out.
 *
 * @param type_idx Index of the type of Pikmin to call out, from the
 * nest's types.
 * @param amount How many to call out.
 * @param l_ptr Leader responsible.
 */
void pikmin_nest_t::request_pikmin(
    size_t type_idx, size_t amount, leader* l_ptr
) {
    call_queue[type_idx] += amount;
    next_call_time = MOB::PIKMIN_NEST_CALL_INTERVAL;
    calling_leader = l_ptr;
}


/**
 * @brief Stores the given Pikmin inside the nest. This basically deletes the
 * Pikmin and updates the amount inside the nest.
 *
 * @param p_ptr Pikmin to store.
 */
void pikmin_nest_t::store_pikmin(pikmin* p_ptr) {
    for(size_t t = 0; t < nest_type->pik_types.size(); t++) {
        if(p_ptr->type == nest_type->pik_types[t]) {
            pikmin_inside[t][p_ptr->maturity]++;
            break;
        }
    }
    
    p_ptr->to_delete = true;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void pikmin_nest_t::tick(float delta_t) {
    if(calling_leader && calling_leader->to_delete) {
        calling_leader = nullptr;
    }
    
    //Call out Pikmin, if the timer agrees.
    if(next_call_time > 0.0f) {
        next_call_time -= delta_t;
    }
    
    while(next_call_time < 0.0f) {
        size_t best_type = INVALID;
        size_t best_type_amount = 0;
        
        for(size_t t = 0; t < nest_type->pik_types.size(); t++) {
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
        
        next_call_time += MOB::PIKMIN_NEST_CALL_INTERVAL;
    }
}


/**
 * @brief Loads nest-related properties from a data file.
 *
 * @param file File to read from.
 */
void pikmin_nest_type_t::load_properties(
    data_node* file
) {
    reader_setter rs(file);
    
    string pik_types_str;
    string legs_str;
    data_node* pik_types_node = nullptr;
    data_node* legs_node = nullptr;
    
    rs.set("leg_body_parts", legs_str, &legs_node);
    rs.set("pikmin_types", pik_types_str, &pik_types_node);
    rs.set("pikmin_enter_speed", pikmin_enter_speed);
    rs.set("pikmin_exit_speed", pikmin_exit_speed);
    
    leg_body_parts = semicolon_list_to_vector(legs_str);
    if(pik_types_node && leg_body_parts.empty()) {
        game.errors.report(
            "A nest-like object type needs a list of leg body parts!",
            file
        );
    } else if(legs_node && leg_body_parts.size() % 2 == 1) {
        game.errors.report(
            "A nest-like object type needs an even number of leg body parts!",
            legs_node
        );
    }
    
    vector<string> pik_types_strs = semicolon_list_to_vector(pik_types_str);
    for(size_t t = 0; t < pik_types_strs.size(); t++) {
        string &str = pik_types_strs[t];
        if(
            game.content.mob_types.list.pikmin.find(str) ==
            game.content.mob_types.list.pikmin.end()
        ) {
            game.errors.report(
                "Unknown Pikmin type \"" + str + "\"!",
                pik_types_node
            );
        } else {
            pik_types.push_back(game.content.mob_types.list.pikmin[str]);
        }
    }
}


/**
 * @brief Constructs a new track info struct object.
 *
 * @param m Mob this track info struct belongs to.
 * @param checkpoints List of checkpoints (body part indexes) to cross.
 * @param ride_speed Speed to ride at, in ratio per second.
 */
track_t::track_t(
    mob* m, const vector<size_t> &checkpoints, float ride_speed
) :
    m(m),
    checkpoints(checkpoints),
    ride_speed(ride_speed) {
    
}


/**
 * @brief Calculates the maximum physical span that a mob can ever reach
 * from its center.
 *
 * @param radius The mob's radius.
 * @param anim_hitbox_span Maximum span of its hitboxes data.
 * @param rectangular_dim Rectangular dimensions of the mob, if any.
 * @return The span.
 */
float calculate_mob_physical_span(
    float radius, float anim_hitbox_span,
    const point &rectangular_dim
) {
    float final_span = std::max(radius, anim_hitbox_span);
    
    if(rectangular_dim.x != 0) {
        final_span =
            std::max(
                final_span, dist(point(0.0f), rectangular_dim / 2.0).to_float()
            );
    }
    
    return final_span;
}


/**
 * @brief Creates a mob, adding it to the corresponding vectors.
 *
 * @param category The category the new mob belongs to.
 * @param pos Initial position.
 * @param type Type of the new mob.
 * @param angle Initial facing angle.
 * @param vars Script variables.
 * @param code_after_creation Code to run right after the mob is created,
 * if any. This is run before any scripting takes place.
 * @param first_state_override If this is INVALID, use the first state
 * index defined in the mob's FSM struct, or the standard first state index.
 * Otherwise, use this.
 * @return The new mob.
 */
mob* create_mob(
    mob_category* category, const point &pos, mob_type* type,
    float angle, const string &vars,
    std::function<void(mob*)> code_after_creation,
    size_t first_state_override
) {
    mob* m_ptr = category->create_mob(pos, type, angle);
    
    if(m_ptr->type->walkable) {
        game.states.gameplay->mobs.walkables.push_back(m_ptr);
    }
    
    if(code_after_creation) {
        code_after_creation(m_ptr);
    }
    
    for(size_t a = 0; a < type->init_actions.size(); a++) {
        type->init_actions[a]->run(m_ptr, nullptr, nullptr);
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
            type->first_state_idx
        )
    ) {
        //If something went wrong, give it some dummy state.
        m_ptr->fsm.cur_state = game.dummy_mob_state;
    };
    
    for(size_t c = 0; c < type->children.size(); c++) {
        mob_type::child_t* child_info =
            &type->children[c];
        mob_type::spawn_t* spawn_info =
            get_spawn_info_from_child_info(m_ptr->type, &type->children[c]);
            
        if(!spawn_info) {
            game.errors.report(
                "Object \"" + type->name + "\" tried to spawn a child with the "
                "spawn name \"" + child_info->spawn_name + "\", but that name "
                "does not exist in the list of spawn data!"
            );
            continue;
        }
        
        mob* new_mob = m_ptr->spawn(spawn_info);
        
        if(!new_mob) continue;
        
        parent_t* p_info = new parent_t(m_ptr);
        new_mob->parent = p_info;
        p_info->handle_damage = child_info->handle_damage;
        p_info->relay_damage = child_info->relay_damage;
        p_info->handle_events = child_info->handle_events;
        p_info->relay_events = child_info->relay_events;
        p_info->handle_statuses = child_info->handle_statuses;
        p_info->relay_statuses = child_info->relay_statuses;
        if(!child_info->limb_anim_name.empty()) {
            p_info->limb_anim.anim_db = m_ptr->anim.anim_db;
            animation* anim_to_use = nullptr;
            for(size_t a = 0; a < m_ptr->anim.anim_db->animations.size(); a++) {
                if(
                    m_ptr->anim.anim_db->animations[a]->name ==
                    child_info->limb_anim_name
                ) {
                    anim_to_use = m_ptr->anim.anim_db->animations[a];
                }
            }
            
            if(anim_to_use) {
                p_info->limb_anim.cur_anim = anim_to_use;
                p_info->limb_anim.to_start();
            } else {
                game.errors.report(
                    "Object \"" + new_mob->type->name + "\", child object of "
                    "object \"" + type->name + "\", tried to use animation \"" +
                    child_info->limb_anim_name + "\" for a limb, but that "
                    "animation doesn't exist in the parent object's animations!"
                );
            }
        }
        p_info->limb_thickness = child_info->limb_thickness;
        p_info->limb_parent_body_part =
            type->anim_db->find_body_part(child_info->limb_parent_body_part);
        p_info->limb_parent_offset = child_info->limb_parent_offset;
        p_info->limb_child_body_part =
            new_mob->type->anim_db->find_body_part(
                child_info->limb_child_body_part
            );
        p_info->limb_child_offset = child_info->limb_child_offset;
        p_info->limb_draw_method = child_info->limb_draw_method;
        
        if(child_info->parent_holds) {
            m_ptr->hold(
                new_mob,
                type->anim_db->find_body_part(child_info->hold_body_part),
                child_info->hold_offset_dist,
                child_info->hold_offset_angle,
                child_info->hold_offset_vert_dist,
                false,
                child_info->hold_rotation_method
            );
        }
    }
    
    game.states.gameplay->mobs.all.push_back(m_ptr);
    return m_ptr;
}


/**
 * @brief Deletes a mob from the relevant vectors.
 *
 * It's always removed from the vector of mobs, but it's
 * also removed from the vector of Pikmin if it's a Pikmin,
 * leaders if it's a leader, etc.
 *
 * @param m_ptr The mob to delete.
 * @param complete_destruction If true, don't bother removing it from groups
 * and such, since everything is going to be destroyed.
 */
void delete_mob(mob* m_ptr, bool complete_destruction) {
    if(game.maker_tools.info_lock == m_ptr) game.maker_tools.info_lock = nullptr;
    
    if(!complete_destruction) {
        m_ptr->leave_group();
        
        for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); m++) {
            mob* m2_ptr = game.states.gameplay->mobs.all[m];
            if(m2_ptr->focused_mob == m_ptr) {
                m2_ptr->fsm.run_event(MOB_EV_FOCUSED_MOB_UNAVAILABLE);
                m2_ptr->fsm.run_event(MOB_EV_FOCUS_OFF_REACH);
                m2_ptr->fsm.run_event(MOB_EV_FOCUS_DIED);
                m2_ptr->focused_mob = nullptr;
            }
            if(m2_ptr->parent && m2_ptr->parent->m == m_ptr) {
                delete m2_ptr->parent;
                m2_ptr->parent = nullptr;
                m2_ptr->to_delete = true;
            }
            for(size_t f = 0; f < m2_ptr->focused_mob_memory.size(); f++) {
                if(m2_ptr->focused_mob_memory[f] == m_ptr) {
                    m2_ptr->focused_mob_memory[f] = nullptr;
                }
            }
            for(size_t c = 0; c < m2_ptr->chomping_mobs.size(); c++) {
                if(m2_ptr->chomping_mobs[c] == m_ptr) {
                    m2_ptr->chomping_mobs[c] = nullptr;
                }
            }
            for(size_t l = 0; l < m2_ptr->links.size(); l++) {
                if(m2_ptr->links[l] == m_ptr) {
                    m2_ptr->links[l] = nullptr;
                }
            }
            if(m2_ptr->stored_inside_another == m_ptr) {
                m_ptr->release(m2_ptr);
                m2_ptr->stored_inside_another = nullptr;
            }
            if(m2_ptr->carry_info) {
                for(
                    size_t c = 0; c < m2_ptr->carry_info->spot_info.size(); c++
                ) {
                    if(m2_ptr->carry_info->spot_info[c].pik_ptr == m_ptr) {
                        m2_ptr->carry_info->spot_info[c].pik_ptr =
                            nullptr;
                        m2_ptr->carry_info->spot_info[c].state =
                            CARRY_SPOT_STATE_FREE;
                    }
                }
            }
        }
        
        if(m_ptr->holder.m) {
            m_ptr->holder.m->release(m_ptr);
        }
        
        while(!m_ptr->holding.empty()) {
            m_ptr->release(m_ptr->holding[0]);
        }
        
        m_ptr->set_can_block_paths(false);
        
        m_ptr->fsm.set_state(INVALID);
    }
    
    game.audio.handle_mob_deletion(m_ptr);
    
    m_ptr->type->category->erase_mob(m_ptr);
    game.states.gameplay->mobs.all.erase(
        find(
            game.states.gameplay->mobs.all.begin(),
            game.states.gameplay->mobs.all.end(),
            m_ptr
        )
    );
    if(m_ptr->type->walkable) {
        game.states.gameplay->mobs.walkables.erase(
            find(
                game.states.gameplay->mobs.walkables.begin(),
                game.states.gameplay->mobs.walkables.end(),
                m_ptr
            )
        );
    }
    
    delete m_ptr;
}


/**
 * @brief Returns a string that describes the given mob. Used in error messages
 * where you have to indicate a specific mob in the area.
 *
 * @param m The mob.
 * @return The string.
 */
string get_error_message_mob_info(mob* m) {
    return
        "type \"" + m->type->name + "\", coordinates " +
        p2s(m->pos) + ", area \"" + game.cur_area_data->name + "\"";
}


/**
 * @brief Returns a list of hazards to which all mob types given
 * are invulnerable.
 *
 * @param types Mob types to check.
 * @return The invulnerabilities.
 */
vector<hazard*> get_mob_type_list_invulnerabilities(
    const unordered_set<mob_type*> &types
) {
    //Count how many types are invulnerable to each detected hazard.
    map<hazard*, size_t> inv_instances;
    for(auto &t : types) {
        for(auto &h : t->hazard_vulnerabilities) {
            if(h.second.effect_mult == 0.0f) {
                inv_instances[h.first]++;
            }
        }
    }
    
    //Only accept those that ALL types are invulnerable to.
    vector<hazard*> invulnerabilities;
    for(auto &i : inv_instances) {
        if(i.second == types.size()) {
            invulnerabilities.push_back(i.first);
        }
    }
    
    return invulnerabilities;
}


/**
 * @brief Given a child info block, returns the spawn info block that matches.
 *
 * @param type Mob type that owns the children and spawn blocks.
 * @param child_info Child info to check.
 * @return The spawn info, or nullptr if not found.
 */
mob_type::spawn_t* get_spawn_info_from_child_info(
    mob_type* type, const mob_type::child_t* child_info
) {
    for(size_t s = 0; s < type->spawns.size(); s++) {
        if(type->spawns[s].name == child_info->spawn_name) {
            return &type->spawns[s];
        }
    }
    return nullptr;
}


/**
 * @brief Returns whether a given mob is in reach or out of reach of another,
 * given the positional and reach data.
 *
 * @param reach_t_ptr Pointer to the reach information.
 * @param dist_between Distance between the two mobs.
 * @param angle_diff Angle difference between the two mobs.
 * @return Whether it's in reach.
 */
bool is_mob_in_reach(
    mob_type::reach_t* reach_t_ptr, const dist &dist_between, float angle_diff
) {
    bool in_reach =
        (
            dist_between <= reach_t_ptr->radius_1 &&
            angle_diff <= reach_t_ptr->angle_1 / 2.0
        );
    if(in_reach) return true;
    in_reach =
        (
            dist_between <= reach_t_ptr->radius_2 &&
            angle_diff <= reach_t_ptr->angle_2 / 2.0
        );
    return in_reach;
}


/**
 * @brief Converts a string to the numeric representation of a mob target type.
 *
 * @param type_str Text representation of the target type.
 * @return The type, or INVALID if invalid.
 */
MOB_TARGET_FLAG string_to_mob_target_type(const string &type_str) {
    if(type_str == "none") {
        return MOB_TARGET_FLAG_NONE;
    } else if(type_str == "player") {
        return MOB_TARGET_FLAG_PLAYER;
    } else if(type_str == "enemy") {
        return MOB_TARGET_FLAG_ENEMY;
    } else if(type_str == "weak_plain_obstacle") {
        return MOB_TARGET_FLAG_WEAK_PLAIN_OBSTACLE;
    } else if(type_str == "strong_plain_obstacle") {
        return MOB_TARGET_FLAG_STRONG_PLAIN_OBSTACLE;
    } else if(type_str == "pikmin_obstacle") {
        return MOB_TARGET_FLAG_PIKMIN_OBSTACLE;
    } else if(type_str == "explodable") {
        return MOB_TARGET_FLAG_EXPLODABLE;
    } else if(type_str == "explodable_pikmin_obstacle") {
        return MOB_TARGET_FLAG_EXPLODABLE_PIKMIN_OBSTACLE;
    } else if(type_str == "fragile") {
        return MOB_TARGET_FLAG_FRAGILE;
    }
    return (MOB_TARGET_FLAG) INVALID;
}


/**
 * @brief Converts a string to the numeric representation of a team.
 *
 * @param team_str Text representation of the team.
 * @return The team, or INVALID if invalid.
 */
MOB_TEAM string_to_team_nr(const string &team_str) {
    for(size_t t = 0; t < N_MOB_TEAMS; t++) {
        if(team_str == game.team_internal_names[t]) {
            return (MOB_TEAM) t;
        }
    }
    return (MOB_TEAM) INVALID;
}
