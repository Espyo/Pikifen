/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Mob class and mob-related functions.
 */

#include <algorithm>

#include "mob.h"

#include "../const.h"
#include "../drawing.h"
#include "../functions.h"
#include "../game.h"
#include "../mob_script_action.h"
#include "../utils/geometry_utils.h"
#include "../utils/string_utils.h"
#include "pikmin.h"
#include "ship.h"
#include "tool.h"
#include "track.h"


size_t next_mob_id = 0;

/* ----------------------------------------------------------------------------
 * Creates a mob of no particular type.
 * pos:
 *   Starting coordinates.
 * type:
 *   Mob type this mob belongs to.
 * angle:
 *   Starting angle.
 */
mob::mob(const point &pos, mob_type* type, const float angle) :
    type(type),
    to_delete(false),
    anim(&type->anims),
    fsm(this),
    script_timer(0),
    focused_mob(nullptr),
    itch_damage(0),
    itch_time(0),
    far_reach(INVALID),
    near_reach(INVALID),
    pos(pos),
    z(0),
    speed_z(0),
    angle(angle),
    intended_turn_angle(angle),
    intended_turn_pos(nullptr),
    height(type->height),
    z_cap(FLT_MAX),
    home(pos),
    ground_sector(nullptr),
    center_sector(nullptr),
    standing_on_mob(nullptr),
    gravity_mult(1.0f),
    push_amount(0),
    push_angle(0),
    unpushable(false),
    tangible(true),
    was_thrown(false),
    chase_info(),
    path_info(nullptr),
    circling_info(nullptr),
    following_group(nullptr),
    subgroup_type_ptr(nullptr),
    group(nullptr),
    group_spot_index(INVALID),
    carry_info(nullptr),
    delivery_info(nullptr),
    health_wheel_smoothed_ratio(1.0f),
    health_wheel_alpha(1.0f),
    id(next_mob_id),
    health(type->max_health),
    invuln_period(0),
    team(MOB_TEAM_NONE),
    hide(false),
    has_invisibility_status(false),
    is_huntable(true),
    height_effect_pivot(LARGE_FLOAT),
    on_hazard(nullptr),
    chomp_max(0),
    disabled_state_flags(0),
    parent(nullptr),
    time_alive(0.0f) {
    
    next_mob_id++;
    
    sector* sec = get_sector(pos, nullptr, true);
    if(sec) {
        z = sec->z;
    } else {
        to_delete = true;
    }
    ground_sector = sec;
    center_sector = sec;
    
    team = type->starting_team;
}


/* ----------------------------------------------------------------------------
 * Destroys an instance of a mob.
 */
mob::~mob() {
    if(carry_info) delete carry_info;
    if(group) delete group;
    if(parent) delete parent;
}



/* ----------------------------------------------------------------------------
 * Adds a mob to this mob's group.
 * new_member:
 *   The new member to add.
 */
void mob::add_to_group(mob* new_member) {
    //If it's already following, never mind.
    if(new_member->following_group == this) return;
    
    new_member->following_group = this;
    group->members.push_back(new_member);
    
    //Find a spot.
    group->init_spots(new_member);
    new_member->group_spot_index = group->spots.size() - 1;
    
    if(!group->cur_standby_type) {
        if(
            new_member->type->category->id != MOB_CATEGORY_LEADERS ||
            game.config.can_throw_leaders
        ) {
            group->cur_standby_type =
                new_member->subgroup_type_ptr;
        }
    }
    
    if(group->members.size() == 1) {
        //If this is the first member, update the anchor position.
        group->anchor = pos;
    }
}


/* ----------------------------------------------------------------------------
 * Applies the damage caused by an attack from another mob to this one.
 * attacker:
 *   The mob that caused the attack.
 * attack_h:
 *   Hitbox used for the attack.
 * victim_h:
 *   Victim's hitbox that got hit.
 * damage:
 *   Total damage the attack caused.
 */
void mob::apply_attack_damage(
    mob* attacker, hitbox* attack_h, hitbox* victim_h, float damage
) {
    //Register this hit, so the next frame doesn't hit it too.
    attacker->hit_opponents.push_back(
        std::make_pair(OPPONENT_HIT_REGISTER_TIMEOUT, this)
    );
    
    //Will the parent mob be handling the damage?
    if(parent && parent->relay_damage) {
        parent->m->apply_attack_damage(attacker, attack_h, victim_h, damage);
        if(!parent->handle_damage) {
            return;
        }
    }
    
    //Perform the damage and script-related events.
    if(damage > 0) {
        set_health(true, false, -damage);
        
        hitbox_interaction ev_info(this, victim_h, attack_h);
        fsm.run_event(MOB_EV_DAMAGE, (void*) &ev_info);
        
        attacker->cause_spike_damage(this, false);
    }
    
    //Final setup.
    itch_damage += damage;
}


/* ----------------------------------------------------------------------------
 * Applies the knockback values to a mob, caused by an attack.
 * knockback:
 *   Total knockback value.
 * knockback_angle:
 *   Angle to knockback towards.
 */
void mob::apply_knockback(const float knockback, const float knockback_angle) {
    if(knockback != 0) {
        stop_chasing();
        speed.x = cos(knockback_angle) * knockback * MOB_KNOCKBACK_H_POWER;
        speed.y = sin(knockback_angle) * knockback * MOB_KNOCKBACK_H_POWER;
        speed_z = MOB_KNOCKBACK_V_POWER;
        face(get_angle(point(), point(speed)) + TAU / 2, NULL);
        start_height_effect();
    }
}


/* ----------------------------------------------------------------------------
 * Applies a status effect's effects.
 * s:
 *   Status effect to use.
 * refill:
 *   If true, then the time left before the status wears off is reset, if the
 *   mob is already under this status effect.
 * given_by_parent:
 *   If true, this status effect was given to the mob by its parent mob.
 */
void mob::apply_status_effect(
    status_type* s, const bool refill, const bool given_by_parent
) {
    if(parent && parent->relay_statuses && !given_by_parent) {
        parent->m->apply_status_effect(s, refill, false);
        if(!parent->handle_statuses) return;
    }
    
    if(!given_by_parent && !can_receive_status(s)) {
        return;
    }
    
    //Let's start by sending the status to the child mobs.
    for(size_t m = 0; m < game.states.gameplay->mobs.all.size(); ++m) {
        mob* m2_ptr = game.states.gameplay->mobs.all[m];
        if(m2_ptr->parent && m2_ptr->parent->m == this) {
            m2_ptr->apply_status_effect(s, refill, true);
        }
    }
    
    //Check if the mob is already under this status.
    for(size_t ms = 0; ms < this->statuses.size(); ++ms) {
        if(this->statuses[ms].type == s) {
            //Already exists. Can we refill its duration?
            
            if(refill && s->auto_remove_time > 0.0f) {
                this->statuses[ms].time_left = s->auto_remove_time;
            }
            
            return;
        }
    }
    
    //This status is not already inflicted. Let's do so.
    this->statuses.push_back(status(s));
    handle_status_effect(s);
    
    if(s->turns_invisible) {
        has_invisibility_status = true;
    }
    
    if(s->generates_particles) {
        particle_generator pg = *s->particle_gen;
        pg.follow_mob = this;
        pg.follow_pos_offset = s->particle_offset_pos;
        pg.follow_z_offset = s->particle_offset_z;
        pg.follow_angle = &this->angle;
        pg.reset();
        particle_generators.push_back(pg);
    }
}


/* ----------------------------------------------------------------------------
 * Does the logic that arachnorb feet need to move to their next spot, based
 * on variables set by the parent mob (the arachnorb head).
 */
void mob::arachnorb_foot_move_logic() {
    if(!parent) {
        return;
    }
    if(parent->limb_parent_body_part == INVALID) {
        return;
    }
    
    float feet_normal_distance = s2f(parent->m->vars["feet_normal_distance"]);
    if(feet_normal_distance == 0) {
        feet_normal_distance = 175;
    }
    
    float default_angle =
        get_angle(
            point(),
            parent->m->get_hitbox(
                parent->limb_parent_body_part
            )->pos
        );
        
    point final_pos = s2p(parent->m->vars["_destination_pos"]);
    float final_angle = s2f(parent->m->vars["_destination_angle"]);
    
    point offset = point(feet_normal_distance, 0);
    offset = rotate_point(offset, default_angle);
    offset = rotate_point(offset, final_angle);
    
    final_pos += offset;
    
    chase(final_pos, NULL, false);
}


/* ----------------------------------------------------------------------------
 * Does the logic that arachnorb heads need to turn, based on their
 * feet's positions.
 */
void mob::arachnorb_head_turn_logic() {
    if(links.empty()) return;
    
    float angle_deviation_avg = 0;
    size_t n_feet = 0;
    
    for(size_t l = 0; l < links.size(); ++l) {
        if(!links[l]->parent) {
            continue;
        }
        if(links[l]->parent->m != this) {
            continue;
        }
        if(links[l]->parent->limb_parent_body_part == INVALID) {
            continue;
        }
        
        n_feet++;
        
        float default_angle =
            get_angle(
                point(),
                get_hitbox(
                    links[l]->parent->limb_parent_body_part
                )->pos
            );
        float cur_angle =
            get_angle(pos, links[l]->pos) - angle;
        float angle_deviation =
            get_angle_cw_dif(default_angle, cur_angle);
        if(angle_deviation > M_PI) {
            angle_deviation -= TAU;
        }
        angle_deviation_avg += angle_deviation;
    }
    
    face(angle + (angle_deviation_avg / n_feet), NULL);
}


/* ----------------------------------------------------------------------------
 * Does the logic that arachnorb heads need to plan out how to move their feet
 * for the next set of steps.
 * goal:
 *   Use MOB_ACTION_ARACHNORB_PLAN_LOGIC_*.
 */
void mob::arachnorb_plan_logic(const unsigned char goal) {
    float max_step_distance = s2f(vars["max_step_distance"]);
    float max_turn_angle = deg_to_rad(s2f(vars["max_turn_angle"]));
    float min_turn_angle = deg_to_rad(s2f(vars["min_turn_angle"]));
    if(max_step_distance == 0) {
        max_step_distance = 100;
    }
    if(max_turn_angle == 0) {
        max_turn_angle = TAU * 0.2;
    }
    
    float amount_to_move = 0;
    float amount_to_turn = 0;
    
    switch(goal) {
    case MOB_ACTION_ARACHNORB_PLAN_LOGIC_HOME: {
        amount_to_turn = get_angle_cw_dif(angle, get_angle(pos, home));
        if(amount_to_turn > TAU / 2)  amount_to_turn -= TAU;
        if(amount_to_turn < -TAU / 2) amount_to_turn += TAU;
        
        if(fabs(amount_to_turn) < TAU * 0.05) {
            //We can also start moving towards home now.
            amount_to_move = dist(pos, home).to_float();
        }
        break;
        
    } case MOB_ACTION_ARACHNORB_PLAN_LOGIC_FORWARD: {
        amount_to_move = max_step_distance;
        break;
        
    } case MOB_ACTION_ARACHNORB_PLAN_LOGIC_CW_TURN: {
        amount_to_turn = randomf(min_turn_angle, TAU * 0.25);
        break;
        
    } case MOB_ACTION_ARACHNORB_PLAN_LOGIC_CCW_TURN: {
        amount_to_turn = randomf(-TAU * 0.25, -min_turn_angle);
        break;
        
    }
    }
    
    amount_to_move = std::min(amount_to_move, max_step_distance);
    amount_to_turn =
        sign(amount_to_turn) *
        std::min((double) fabs(amount_to_turn), (double) max_turn_angle);
        
    point destination_pos = pos;
    float destination_angle = angle + amount_to_turn;
    normalize_angle(destination_angle);
    
    point offset = point(amount_to_move, 0);
    offset = rotate_point(offset, destination_angle);
    
    destination_pos += offset;
    
    vars["_destination_pos"] = p2s(destination_pos);
    vars["_destination_angle"] = f2s(destination_angle);
}


/* ----------------------------------------------------------------------------
 * Sets up data for a mob to become carriable.
 * destination:
 *   Where to carry it. Use CARRY_DESTINATION_*.
 */
void mob::become_carriable(const size_t destination) {
    carry_info = new carry_info_struct(this, destination);
}


/* ----------------------------------------------------------------------------
 * Sets up data for a mob to stop being carriable.
 */
void mob::become_uncarriable() {
    if(!carry_info) return;
    
    for(size_t p = 0; p < carry_info->spot_info.size(); ++p) {
        if(carry_info->spot_info[p].state != CARRY_SPOT_FREE) {
            carry_info->spot_info[p].pik_ptr->fsm.run_event(
                MOB_EV_FOCUSED_MOB_UNAVAILABLE
            );
        }
    }
    
    stop_chasing();
    
    delete carry_info;
    carry_info = NULL;
}


/* ----------------------------------------------------------------------------
 * Calculates the final carrying target, and the final carrying position,
 * given the sort of carry destination, what Pikmin are holding on, and what
 * Pikmin got added or removed.
 * Returns true on success, false if there are no available targets or if
 *   something went wrong.
 * added:
 *   The Pikmin that got added, if any.
 * removed:
 *   The Pikmin that got removed, if any.
 * target_type:
 *   Return the target Pikmin type (if any) here.
 * target_mob:
 *   Return the target mob (if any) here.
 * target_point:
 *   Return the target point here.
 */
bool mob::calculate_carrying_destination(
    mob* added, mob* removed,
    pikmin_type** target_type, mob** target_mob, point* target_point
) const {
    if(!carry_info) return false;
    
    //For starters, check if this is to be carried to the ship.
    //Get that out of the way if so.
    if(carry_info->destination == CARRY_DESTINATION_SHIP) {
    
        ship* closest_ship = NULL;
        dist closest_ship_dist;
        
        for(size_t s = 0; s < game.states.gameplay->mobs.ships.size(); ++s) {
            ship* s_ptr = game.states.gameplay->mobs.ships[s];
            dist d(pos, s_ptr->beam_final_pos);
            
            if(!closest_ship || d < closest_ship_dist) {
                closest_ship = s_ptr;
                closest_ship_dist = d;
            }
        }
        
        if(closest_ship) {
            *target_mob = closest_ship;
            *target_point = closest_ship->beam_final_pos;
            return true;
            
        } else {
            *target_mob = NULL;
            return false;
        }
    }
    
    //Now, if it's towards a linked mob, just go there.
    if(carry_info->destination == CARRY_DESTINATION_LINKED_MOB) {
        if(!links.empty()) {
            *target_mob = links[0];
            *target_point = (*target_mob)->pos;
            return true;
            
        } else {
            *target_mob = NULL;
            return false;
        }
    }
    
    //If it's meant for an Onion, we need to decide which Onion, based on
    //the Pikmin. Buckle up, because it's not as easy as it might seem.
    
    //How many of each Pikmin type are carrying.
    map<pikmin_type*, unsigned> type_quantity;
    //The Pikmin type with the most carriers.
    vector<pikmin_type*> majority_types;
    unordered_set<pikmin_type*> available_types;
    
    //First, check which Onion Pikmin types are even available.
    for(size_t o = 0; o < game.states.gameplay->mobs.onions.size(); o++) {
        onion* o_ptr = game.states.gameplay->mobs.onions[o];
        if(o_ptr->activated) {
            for(
                size_t t = 0;
                t < o_ptr->oni_type->nest->pik_types.size();
                ++t
            ) {
                available_types.insert(
                    o_ptr->oni_type->nest->pik_types[t]
                );
            }
        }
    }
    
    if(available_types.empty()) {
        //No available types?! Well...make the Pikmin stuck.
        *target_mob = NULL;
        return false;
    }
    
    //Count how many of each type there are carrying.
    for(size_t p = 0; p < type->max_carriers; ++p) {
        pikmin* pik_ptr = NULL;
        
        if(carry_info->spot_info[p].state != CARRY_SPOT_USED) continue;
        
        pik_ptr = (pikmin*) carry_info->spot_info[p].pik_ptr;
        
        //If it doesn't have an Onion to carry to, it won't even count.
        if(available_types.find(pik_ptr->pik_type) == available_types.end()) {
            continue;
        }
        
        type_quantity[pik_ptr->pik_type]++;
    }
    
    //Then figure out what are the majority types.
    unsigned most = 0;
    for(auto &t : type_quantity) {
        if(t.second > most) {
            most = t.second;
            majority_types.clear();
        }
        if(t.second == most) majority_types.push_back(t.first);
    }
    
    //If we ended up with no candidates, pick a type at random,
    //out of all possible types.
    if(majority_types.empty()) {
        for(
            auto t = available_types.begin();
            t != available_types.end(); ++t
        ) {
            majority_types.push_back(*t);
        }
    }
    
    pikmin_type* decided_type = NULL;
    
    //Now let's pick an Pikmin type from the candidates.
    if(majority_types.size() == 1) {
        //If there's only one possible type to pick, pick it.
        decided_type = *majority_types.begin();
        
    } else {
        //If there's a tie, let's take a careful look.
        bool new_tie = false;
        
        //Is the Pikmin that just joined part of the majority types?
        //If so, that means this Pikmin just created a NEW tie!
        //So let's pick a random Onion again.
        if(added) {
            for(size_t mt = 0; mt < majority_types.size(); ++mt) {
                if(added->type == majority_types[mt]) {
                    new_tie = true;
                    break;
                }
            }
        }
        
        //If a Pikmin left, check if it is related to the majority types.
        //If not, then a new tie wasn't made, no worries.
        //If it was related, a new tie was created.
        if(removed) {
            new_tie = false;
            for(size_t mt = 0; mt < majority_types.size(); ++mt) {
                if(removed->type == majority_types[mt]) {
                    new_tie = true;
                    break;
                }
            }
        }
        
        //Check if the previously decided type belongs to one of the majorities.
        //If so, it can be chosen again, but if not, it cannot.
        bool can_continue = false;
        for(size_t mt = 0; mt < majority_types.size(); ++mt) {
            if(majority_types[mt] == decided_type) {
                can_continue = true;
                break;
            }
        }
        if(!can_continue) decided_type = NULL;
        
        //If the Pikmin that just joined is not a part of the majorities,
        //then it had no impact on the existing ties.
        //Go with the Onion that had been decided before.
        if(new_tie || !decided_type) {
            decided_type =
                majority_types[randomi(0, majority_types.size() - 1)];
        }
    }
    
    
    //Figure out where that type's Onion is.
    size_t closest_onion_nr = INVALID;
    dist closest_onion_dist;
    for(size_t o = 0; o < game.states.gameplay->mobs.onions.size(); ++o) {
        onion* o_ptr = game.states.gameplay->mobs.onions[o];
        if(!o_ptr->activated) continue;
        bool has_type = false;
        for(
            size_t t = 0;
            t < o_ptr->oni_type->nest->pik_types.size();
            ++t
        ) {
            if(o_ptr->oni_type->nest->pik_types[t] == decided_type) {
                has_type = true;
                break;
            }
        }
        if(!has_type) continue;
        
        dist d(pos, o_ptr->pos);
        if(closest_onion_nr == INVALID || d < closest_onion_dist) {
            closest_onion_dist = d;
            closest_onion_nr = o;
        }
    }
    
    //Finally, set the destination data.
    *target_type = decided_type;
    *target_mob = game.states.gameplay->mobs.onions[closest_onion_nr];
    *target_point = (*target_mob)->pos;
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Calculates how much damage an attack will cause.
 * Returns true if the attack will hit (even if it will end up causing zero
 * damage), false if it cannot hit (e.g. the victim hitbox is not valid).
 * victim:
 *   The mob that'll take the damage.
 * attack_h:
 *   Hitbox used for the attack.
 * victim_h:
 *   Victim's hitbox that got hit.
 * damage:
 *   Return the calculated damage here.
 */
bool mob::calculate_damage(
    mob* victim, hitbox* attack_h, hitbox* victim_h, float* damage
) const {
    float attacker_offense = 0;
    float defense_multiplier = 1;
    
    //First, check if this hitbox cannot be damaged.
    if(victim_h->type != HITBOX_TYPE_NORMAL) {
        //This hitbox can't be damaged! Abort!
        return false;
    }
    
    //Calculate the damage.
    if(attack_h) {
        attacker_offense = attack_h->value;
        
        if(!attack_h->hazards.empty()) {
            float max_vulnerability = 0.0f;
            for(size_t h = 0; h < attack_h->hazards.size(); ++h) {
                max_vulnerability =
                    std::max(
                        victim->get_hazard_vulnerability(attack_h->hazards[h]),
                        max_vulnerability
                    );
            }
            
            if(max_vulnerability == 0.0f) {
                //The victim is immune to this hazard!
                *damage = 0;
                return true;
            } else {
                defense_multiplier = 1.0f / max_vulnerability;
            }
            
        } else {
        
            if(victim->type->default_vulnerability == 0.0f) {
                //The victim is invulnerable to everything about this attack!
                *damage = 0;
                return true;
            } else {
                defense_multiplier = 1.0f / victim->type->default_vulnerability;
            }
        }
        
    } else {
        attacker_offense = 1;
    }
    
    if(victim_h->value == 0.0f) {
        //Hah, this hitbox is invulnerable!
        *damage = 0;
        return true;
    }
    
    defense_multiplier *= victim_h->value;
    
    for(size_t s = 0; s < statuses.size(); ++s) {
        attacker_offense *= statuses[s].type->attack_multiplier;
    }
    for(size_t s = 0; s < victim->statuses.size(); ++s) {
        defense_multiplier *= victim->statuses[s].type->defense_multiplier;
    }
    
    *damage = attacker_offense * (1.0f / defense_multiplier);
    return true;
}


/* ----------------------------------------------------------------------------
 * Calculates how much knockback an attack will cause.
 * victim:
 *   The mob that'll take the damage.
 * attack_h:
 *   The hitbox of the attacker mob, if any.
 * victim_h:
 *   The hitbox of the victim mob, if any.
 * kb_strength:
 *   The variable to return the knockback amount to.
 * kb_angle:
 *   The variable to return the angle of the knockback to.
 */
void mob::calculate_knockback(
    mob* victim, hitbox* attack_h,
    hitbox* victim_h, float* kb_strength, float* kb_angle
) const {
    if(attack_h) {
        *kb_strength = attack_h->knockback;
        if(attack_h->knockback_outward) {
            *kb_angle = get_angle(pos, victim->pos);
        } else {
            *kb_angle = angle + attack_h->knockback_angle;
        }
    } else {
        *kb_strength = 0;
        *kb_angle = 0;
    }
}


/* ----------------------------------------------------------------------------
 * Calculates the requires horizontal and vertical speed in order to
 * throw this mob to the specified coordinates, such that it reaches a
 * specific peak height.
 * If the calculation is impossible (like if the peak height is lower than the
 * starting height), the speed variables will all be set to 0.
 * target_xy:
 *   Target destination's X and Y coordinates.
 * target_z:
 *   Target destination's Z coordinate.
 * max_h:
 *   Maximum height, using the starting Z as the reference.
 * req_speed_xy:
 *   The required X and Y speed is returned here.
 * req_speed_z:
 *   The required Z speed is returned here.
 * final_h_angle:
 *   The final horizontal angle is returned here (if not NULL).
 */
void mob::calculate_throw(
    const point &target_xy, const float target_z, const float max_h,
    point* req_speed_xy, float* req_speed_z, float* final_h_angle
) const {

    if(target_z > max_h) {
        //If the target is above the maximum height it can be thrown...
        //Then this is an impossible throw.
        *req_speed_xy = point();
        *req_speed_z = 0;
        return;
    }
    
    //Code from https://physics.stackexchange.com/questions/515688
    //First, we calculate stuff in 2D, with horizontal and vertical components
    //only.
    
    //We start with the vertical speed. This will be constant regardless
    //of how far the mob is thrown. In order to reach the required max height,
    //the vertical speed needs to be set thusly:
    *req_speed_z = sqrt(2.0 * (-GRAVITY_ADDER) * max_h);
    
    //Now that we know the vertical speed, we can figure out how long it takes
    //for the mob to land at the target vertical coordinate. The formula for
    //this can be found on Wikipedia, for instance.
    float height_delta = z - target_z;
    //Because of floating point precision problems, the result of the sqrt
    //could end up negative. Let's cap it to zero.
    float sqrt_part =
        std::max(
            0.0,
            sqrt(
                (*req_speed_z) * (*req_speed_z) +
                2.0 * (-GRAVITY_ADDER) * (height_delta)
            )
        );
    float flight_time = ((*req_speed_z) + sqrt_part) / (-GRAVITY_ADDER);
    
    //Once we know the total flight time, we can divide the horizontal reach
    //by the total time to get the horizontal speed.
    float h_angle, h_reach;
    coordinates_to_angle(target_xy - pos, &h_angle, &h_reach);
    
    float h_speed = h_reach / flight_time;
    
    //Now that we know the vertical and horizontal speed, just split the
    //horizontal speed into X and Y 3D world components.
    *req_speed_xy = angle_to_coordinates(h_angle, h_speed);
    
    //Return the final horizontal angle, if needed.
    if(final_h_angle) *final_h_angle = h_angle;
}


/* ----------------------------------------------------------------------------
 * Does this mob want to attack mob v? Teams and other factors are used to
 * decide this.
 * v:
 *   The victim to check.
 */
bool mob::can_hunt(mob* v) const {
    //Teammates cannot hunt each other down.
    if(team == v->team && team != MOB_TEAM_NONE) return false;
    
    //Mobs that do not participate in combat whatsoever cannot be hunted down.
    if(v->type->target_type == MOB_TARGET_TYPE_NONE) return false;
    
    //Invisible mobs cannot be seen, so they can't be hunted down.
    if(v->has_invisibility_status) return false;
    
    //Mobs that don't want to be hunted right now cannot be hunted down.
    if(!v->is_huntable) return false;
    
    //Return whether or not this mob wants to hunt v.
    return (type->huntable_targets & v->type->target_type);
}


/* ----------------------------------------------------------------------------
 * Can this mob damage v? Teams and other factors are used to decide this.
 * v:
 *   The victim to check.
 */
bool mob::can_hurt(mob* v) const {
    //Teammates cannot hurt each other.
    if(team == v->team && team != MOB_TEAM_NONE) return false;
    
    //Mobs that do not participate in combat whatsoever cannot be hurt.
    if(v->type->target_type == MOB_TARGET_TYPE_NONE) return false;
    
    //Mobs that are invulnerable cannot be hurt.
    if(v->invuln_period.time_left > 0) return false;
    
    //Check if this mob has already hit v recently.
    for(size_t h = 0; h < hit_opponents.size(); ++h) {
        if(hit_opponents[h].second == v) {
            //v was hit by this mob recently, so don't let it attack again.
            //This stops the same attack from hitting every single frame.
            return false;
        }
    }
    
    //Return whether or not this mob can damage v.
    return (type->hurtable_targets & v->type->target_type);
}


/* ----------------------------------------------------------------------------
 * Returns whether or not a mob can receive a given status effect.
 * s:
 *   Status type to check.
 */
bool mob::can_receive_status(status_type* s) const {
    return s->affects & STATUS_AFFECTS_OTHERS;
}


/* ----------------------------------------------------------------------------
 * Makes the mob cause spike damage to another mob.
 * victim:
 *   The mob that will be damaged.
 * is_ingestion:
 *   If true, the attacker just got eaten. If false, it merely got hurt.
 */
void mob::cause_spike_damage(mob* victim, const bool is_ingestion) {
    if(!type->spike_damage) return;
    
    if(type->spike_damage->ingestion_only != is_ingestion) return;
    
    float damage;
    if(type->spike_damage->is_damage_ratio) {
        damage = victim->type->max_health * type->spike_damage->damage;
    } else {
        damage = type->spike_damage->damage;
    }
    
    auto v =
        victim->type->spike_damage_vulnerabilities.find(type->spike_damage);
    if(v != victim->type->spike_damage_vulnerabilities.end()) {
        damage *= v->second;
    }
    
    victim->set_health(true, false, -damage);
    
    if(type->spike_damage->particle_gen) {
        particle_generator pg = *(type->spike_damage->particle_gen);
        pg.base_particle.pos =
            victim->pos + type->spike_damage->particle_offset_pos;
        pg.base_particle.z =
            victim->z + type->spike_damage->particle_offset_z;
        pg.emit(game.states.gameplay->particles);
    }
}


/* ----------------------------------------------------------------------------
 * Sets a target for the mob to follow.
 * offset:
 *   Coordinates of the target, relative to either the
 *   world origin, or another point, specified in the next parameters.
 * orig_coords:
 *   Pointer to changing coordinates. If NULL, it is
 *   the world origin. Use this to make the mob follow another mob
 *   wherever they go, for instance.
 * teleport:
 *   If true, the mob teleports to that spot, instead of walking to it.
 * teleport_z:
 *   Teleports to this Z coordinate, too.
 * free_move:
 *   If true, the mob can go to a direction they're not facing.
 * target_distance:
 *   Distance from the target in which the mob is
 *   considered as being there.
 * speed:
 *   Speed at which to go to the target. -1 uses the mob's speed.
 */
void mob::chase(
    const point &offset, point* orig_coords,
    const bool teleport, float* teleport_z,
    const bool free_move, const float target_distance, const float speed
) {

    this->chase_info.offset = offset;
    this->chase_info.orig_coords = orig_coords;
    this->chase_info.teleport = teleport;
    this->chase_info.teleport_z = teleport_z;
    this->chase_info.free_move = free_move || type->can_free_move;
    this->chase_info.target_dist = target_distance;
    this->chase_info.speed = (speed == -1 ? get_base_speed() : speed);
    
    chase_info.is_chasing = true;
    chase_info.reached_destination = false;
}


/* ----------------------------------------------------------------------------
 * Makes a mob chomp another mob. Mostly applicable for enemies chomping
 * on Pikmin.
 * m:
 *   The mob to be chomped.
 * hitbox_info:
 *   Information about the hitbox that caused the chomp.
 */
void mob::chomp(mob* m, hitbox* hitbox_info) {
    if(m->type->category->id == MOB_CATEGORY_TOOLS) {
        if(!(((tool*) m)->holdability_flags & HOLDABLE_BY_ENEMIES)) {
            //Enemies can't chomp this tool right now.
            return;
        }
    }
    
    float h_offset_dist;
    float h_offset_angle;
    get_hitbox_hold_point(
        m, hitbox_info, &h_offset_dist, &h_offset_angle
    );
    hold(
        m, hitbox_info->body_part_index, h_offset_dist, h_offset_angle,
        true, false
    );
    
    m->focus_on_mob(this);
    chomping_mobs.push_back(m);
}


/* ----------------------------------------------------------------------------
 * Makes the mob start circling around a point or another mob.
 * m:
 *   The mob to circle around.
 *   If NULL, circle around a point instead.
 * p:
 *   The point to circle around, if any.
 * radius:
 *   Circle these many units around the target.
 * clockwise:
 *   Circle clockwise or counter-clockwise?
 * speed:
 *   Speed at which to move.
 * can_free_move:
 *   Can the mob move freely, or only forward?
 */
void mob::circle_around(
    mob* m, const point &p, const float radius, const bool clockwise,
    const float speed, const bool can_free_move
) {
    if(!circling_info) {
        circling_info = new circling_info_struct(this);
    }
    circling_info->circling_mob = m;
    circling_info->circling_point = p;
    circling_info->radius = radius;
    circling_info->clockwise = clockwise;
    circling_info->speed = speed;
    circling_info->can_free_move = can_free_move;
    circling_info->cur_angle =
        get_angle((m ? m->pos : p), pos);
}


/* ----------------------------------------------------------------------------
 * Deletes all status effects asking to be deleted.
 */
void mob::delete_old_status_effects() {
    for(size_t s = 0; s < statuses.size(); ) {
        if(statuses[s].to_delete) {
            if(statuses[s].type->causes_panic) {
                handle_panic_loss();
            }
            if(statuses[s].type->generates_particles) {
                remove_particle_generator(statuses[s].type->particle_gen->id);
            }
            statuses.erase(statuses.begin() + s);
        } else {
            ++s;
        }
    }
    
    //Update some flags.
    has_invisibility_status = false;
    for(size_t s = 0; s < statuses.size(); ++s) {
        if(statuses[s].type->turns_invisible) {
            has_invisibility_status = true;
            break;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Starts the particle effect and sound for an attack, which could either be
 * a meaty whack, or a harmless ding.
 * attacker:
 *   Mob that caused the attack.
 * attack_h:
 *   Hitbox that caused the attack.
 * victim_h:
 *   Hitbox that suffered the attack.
 * damage:
 *   Total damage caused.
 * knockback:
 *   Total knockback strength.
 */
void mob::do_attack_effects(
    mob* attacker, hitbox* attack_h, hitbox* victim_h,
    const float damage, const float knockback
) {
    if(attack_h->value == 0.0f) {
        //Attack hitboxes that cause 0 damage don't need to smack or ding.
        //This way, objects can "attack" other objects at 0 damage for the
        //purposes of triggering events (like hazard touching), without
        //having to constantly display the dings.
        //The ding effect should only be used when an attack that really WANTED
        //to cause damage failed to do so, thus highlighting the uselessness.
        return;
    }
    
    //Calculate the particle's final position.
    point attack_h_pos = attack_h->get_cur_pos(attacker->pos, attacker->angle);
    point victim_h_pos = victim_h->get_cur_pos(pos, angle);
    
    float edges_d;
    float a_to_v_angle;
    coordinates_to_angle(
        victim_h_pos - attack_h_pos,
        &a_to_v_angle, &edges_d
    );
    
    edges_d -= attack_h->radius;
    edges_d -= victim_h->radius;
    float offset = attack_h->radius + edges_d / 2.0;
    
    point particle_pos =
        attack_h_pos +
        point(cos(a_to_v_angle) * offset, sin(a_to_v_angle) * offset);
        
    bool useless = (damage <= 0 && knockback == 0.0f);
    
    //Create the particle.
    if(!useless) {
        particle smack_p(
            PARTICLE_TYPE_SMACK, particle_pos,
            std::max(z + height + 1, attacker->z + attacker->height + 1),
            64, SMACK_PARTICLE_DUR, PARTICLE_PRIORITY_MEDIUM
        );
        smack_p.bitmap = game.sys_assets.bmp_smack;
        smack_p.color = al_map_rgb(255, 160, 128);
        game.states.gameplay->particles.add(smack_p);
        
    } else {
        particle ding_p(
            PARTICLE_TYPE_DING, particle_pos,
            std::max(z + height + 1, attacker->z + attacker->height + 1),
            24, SMACK_PARTICLE_DUR * 2, PARTICLE_PRIORITY_MEDIUM
        );
        ding_p.bitmap = game.sys_assets.bmp_wave_ring;
        ding_p.color = al_map_rgb(192, 208, 224);
        game.states.gameplay->particles.add(ding_p);
        
    }
    
    //Play the sound.
    if(!useless) {
        game.sys_assets.sfx_attack.play(0.06, false, 0.6f);
    }
}


/* ----------------------------------------------------------------------------
 * Draws the limb that connects this mob to its parent.
 */
void mob::draw_limb() {
    if(!parent) return;
    if(!parent->limb_anim.anim_db) return;
    sprite* sprite_to_use = parent->limb_anim.get_cur_sprite();
    if(!sprite_to_use) return;
    
    bitmap_effect_info eff;
    get_sprite_bitmap_effects(sprite_to_use, &eff, true, true);
    
    point parent_end;
    if(parent->limb_parent_body_part == INVALID) {
        parent_end = parent->m->pos;
    } else {
        parent_end =
            parent->m->get_hitbox(
                parent->limb_parent_body_part
            )->get_cur_pos(
                parent->m->pos, parent->m->angle_cos, parent->m->angle_sin
            );
    }
    
    point child_end;
    if(parent->limb_child_body_part == INVALID) {
        child_end = pos;
    } else {
        child_end =
            get_hitbox(
                parent->limb_child_body_part
            )->get_cur_pos(pos, angle_cos, angle_sin);
    }
    
    float p2c_angle = get_angle(parent_end, child_end);
    
    if(parent->limb_parent_offset) {
        parent_end +=
            rotate_point(
                point(parent->limb_parent_offset, 0), p2c_angle
            );
    }
    if(parent->limb_child_offset) {
        child_end -=
            rotate_point(
                point(parent->limb_child_offset, 0), p2c_angle
            );
    }
    
    float length = dist(parent_end, child_end).to_float();
    
    eff.translation = (parent_end + child_end) / 2.0;
    eff.scale.x =
        length / al_get_bitmap_width(sprite_to_use->bitmap);
    eff.scale.y =
        parent->limb_thickness / al_get_bitmap_height(sprite_to_use->bitmap);
    eff.rotation = p2c_angle;
    
    draw_bitmap_with_effects(sprite_to_use->bitmap, eff);
}


/* ----------------------------------------------------------------------------
 * Draws just the mob. This is a generic function, and can be overwritten
 * by child classes.
 */
void mob::draw_mob() {
    sprite* s_ptr = anim.get_cur_sprite();
    
    if(!s_ptr) return;
    
    bitmap_effect_info eff;
    get_sprite_bitmap_effects(s_ptr, &eff, true, true);
    
    draw_bitmap_with_effects(s_ptr->bitmap, eff);
}


/* ----------------------------------------------------------------------------
 * Makes a mob intend to face a new angle.
 * new_angle:
 *   Face this angle.
 * new_pos:
 *   If this is not NULL, turn towards this point every frame, instead.
 */
void mob::face(const float new_angle, point* new_pos) {
    if(carry_info) return; //If it's being carried, it shouldn't rotate.
    intended_turn_angle = new_angle;
    intended_turn_pos = new_pos;
}


/* ----------------------------------------------------------------------------
 * Sets up stuff for the end of the mob's dying process.
 */
void mob::finish_dying() {
    release_chomped_pikmin();
    
    finish_dying_class_specifics();
}


/* ----------------------------------------------------------------------------
 * Sets up stuff for the end of the mob's dying process.
 * This function is meant to be overridden by child classes.
 */
void mob::finish_dying_class_specifics() {
}


/* ----------------------------------------------------------------------------
 * Makes the mob focus on m2.
 * m2:
 *   The mob to focus on.
 */
void mob::focus_on_mob(mob* m2) {
    unfocus_from_mob();
    focused_mob = m2;
}


/* ----------------------------------------------------------------------------
 * Makes the mob start following a path. This populates the path_info
 * class member, and calculates a path to take.
 * Returns whether or not there is a path available.
 * target:
 *   Target point to reach.
 * can_continue:
 *   If true, it is possible for the new path to continue
 *   from where the old one left off, if there was an old one.
 * speed:
 *   Speed at which to travel. -1 uses the mob's speed.
 * final_target_distance:
 *   For the final chase, from the last path stop to
 *   the destination, use this for the target distance parameter.
 */
bool mob::follow_path(
    const point &target, const bool can_continue,
    const float speed, const float final_target_distance
) {
    bool was_blocked = false;
    path_stop* old_next_stop = NULL;
    
    if(can_continue && path_info) {
        was_blocked = path_info->is_blocked;
        if(path_info->cur_path_stop_nr < path_info->path.size()) {
            old_next_stop = path_info->path[path_info->cur_path_stop_nr];
        }
    }
    
    if(path_info) {
        delete path_info;
    }
    
    path_info = new path_info_struct(this, target);
    path_info->final_target_distance = final_target_distance;
    
    if(
        can_continue &&
        old_next_stop &&
        !was_blocked &&
        path_info->path.size() >= 2
    ) {
        for(size_t s = 1; s < path_info->path.size(); ++s) {
            if(path_info->path[s] == old_next_stop) {
                //If before, the mob was already heading towards this stop,
                //then just continue the new journey from there.
                path_info->cur_path_stop_nr = s;
                break;
            }
        }
    }
    
    if(path_info->path.size() >= 2 && path_info->cur_path_stop_nr > 0) {
        if(path_info->check_blockage()) {
            path_info->is_blocked = true;
            fsm.run_event(MOB_EV_PATH_BLOCKED);
        }
    }
    
    if(path_info->go_straight) {
        chase(
            target, NULL, false, NULL, true,
            path_info->final_target_distance, speed
        );
    } else if(!path_info->path.empty()) {
        chase(
            path_info->path[path_info->cur_path_stop_nr]->pos,
            NULL, false, NULL, true, 3.0f, speed
        );
    } else {
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Returns the base speed for this mob.
 * This is overwritten by some child classes.
 */
float mob::get_base_speed() const {
    return this->type->move_speed;
}


/* ----------------------------------------------------------------------------
 * Returns the actual location of the movement target.
 */
point mob::get_chase_target() const {
    point p = chase_info.offset;
    if(chase_info.orig_coords) p += (*chase_info.orig_coords);
    return p;
}


/* ----------------------------------------------------------------------------
 * Returns the closest hitbox to a point, belonging to a mob's current frame
 * of animation and position.
 * p:
 *   The point.
 * h_type:
 *   Type of hitbox. INVALID means any.
 * d:
 *   Return the distance here, optionally.
 */
hitbox* mob::get_closest_hitbox(
    const point &p, const size_t h_type, dist* d
) const {
    sprite* s = anim.get_cur_sprite();
    if(!s) return NULL;
    hitbox* closest_hitbox = NULL;
    float closest_hitbox_dist = 0;
    
    for(size_t h = 0; h < s->hitboxes.size(); ++h) {
        hitbox* h_ptr = &s->hitboxes[h];
        if(h_type != INVALID && h_ptr->type != h_type) continue;
        
        float d =
            dist(
                h_ptr->get_cur_pos(pos, angle_cos, angle_sin), p
            ).to_float() - h_ptr->radius;
        if(closest_hitbox == NULL || d < closest_hitbox_dist) {
            closest_hitbox_dist = d;
            closest_hitbox = h_ptr;
        }
    }
    
    if(d) *d = closest_hitbox_dist;
    
    return closest_hitbox;
}


/* ----------------------------------------------------------------------------
 * Returns how vulnerable the mob is to that specific hazard,
 * or the mob type's default if there is no vulnerability data for that hazard.
 * h_ptr:
 *   The hazard to check.
 */
float mob::get_hazard_vulnerability(hazard* h_ptr) const {
    float vulnerability_value = type->default_vulnerability;
    auto vul = type->hazard_vulnerabilities.find(h_ptr);
    if(vul != type->hazard_vulnerabilities.end()) {
        vulnerability_value = vul->second;
    }
    
    return vulnerability_value;
}


/* ----------------------------------------------------------------------------
 * Returns the hitbox in the current animation with the specified number.
 * nr:
 *   The hitbox's number.
 */
hitbox* mob::get_hitbox(const size_t nr) const {
    sprite* s = anim.get_cur_sprite();
    if(!s) return NULL;
    if(s->hitboxes.empty()) return NULL;
    return &s->hitboxes[nr];
}


/* ----------------------------------------------------------------------------
 * When a mob is meant to be held by a hitbox, this function returns where
 * in the hitbox the mob currently is.
 * mob_to_hold:
 *   The mob that will be held.
 * h_ptr:
 *   Pointer to the hitbox to check.
 * offset_dist:
 *   The distance from the center of the hitbox is returned here.
 *   1 means the full radius.
 * offset_angle:
 *   The angle the mob to hold makes with the hitbox's center is returned here.
 */
void mob::get_hitbox_hold_point(
    mob* mob_to_hold, hitbox* h_ptr, float* offset_dist, float* offset_angle
) const {
    point actual_h_pos = h_ptr->get_cur_pos(pos, angle_cos, angle_sin);
    
    point pos_dif = mob_to_hold->pos - actual_h_pos;
    coordinates_to_angle(pos_dif, offset_angle, offset_dist);
    
    //Relative to 0 degrees.
    *offset_angle -= angle;
    //Distance in units to distance in percentage.
    *offset_dist /= h_ptr->radius;
}


/* ----------------------------------------------------------------------------
 * Returns how many Pikmin are currently latched on to this mob.
 */
size_t mob::get_latched_pikmin_amount() const {
    size_t total = 0;
    for(
        size_t p = 0;
        p < game.states.gameplay->mobs.pikmin_list.size(); ++p
    ) {
        pikmin* p_ptr = game.states.gameplay->mobs.pikmin_list[p];
        if(p_ptr->focused_mob != this) continue;
        if(p_ptr->holder.m != this) continue;
        if(!p_ptr->latched) continue;
        total++;
    }
    return total;
}


/* ----------------------------------------------------------------------------
 * Returns the total weight of the Pikmin that are currently
 * latched on to this mob.
 */
float mob::get_latched_pikmin_weight() const {
    float total = 0;
    for(
        size_t p = 0;
        p < game.states.gameplay->mobs.pikmin_list.size(); ++p
    ) {
        pikmin* p_ptr = game.states.gameplay->mobs.pikmin_list[p];
        if(p_ptr->focused_mob != this) continue;
        if(p_ptr->holder.m != this) continue;
        if(!p_ptr->latched) continue;
        total += p_ptr->type->weight;
    }
    return total;
}


/* ----------------------------------------------------------------------------
 * Returns what the given sprite's center, rotation, tint, etc. should be
 * at the present moment, for normal mob drawing routines.
 * s_ptr:
 *   Sprite to get info about.
 * info:
 *   Struct to fill the info with.
 * add_status:
 *   If true, add status effect coloring to the result.
 * add_sector_brightness:
 *   If true, add sector brightness coloring to the result.
 * delivery_time_ratio_left:
 *   If not LARGE_FLOAT, this indicates how much time
 *   is left in the delivery, as a ratio, and the delivery's shrinking
 *   and glowing effects will be added to the result.
 * delivery_color:
 *   If applying a delivery effect, this is the color to make it glow in.
 */
void mob::get_sprite_bitmap_effects(
    sprite* s_ptr, bitmap_effect_info* info,
    const bool add_status, const bool add_sector_brightness,
    const float delivery_time_ratio_left, const ALLEGRO_COLOR &delivery_color
) const {
    info->translation =
        point(
            pos.x + angle_cos * s_ptr->offset.x - angle_sin * s_ptr->offset.y,
            pos.y + angle_sin * s_ptr->offset.x + angle_cos * s_ptr->offset.y
        );
    info->rotation = angle + s_ptr->angle;
    get_sprite_dimensions(s_ptr, &(info->scale));
    
    if(add_status) {
        size_t n_glow_colors = 0;
        ALLEGRO_COLOR glow_color_sum = al_map_rgba(0, 0, 0, 0);
        
        for(size_t s = 0; s < statuses.size(); ++s) {
            status_type* t = this->statuses[s].type;
            if(
                t->tint.r == 1.0f &&
                t->tint.g == 1.0f &&
                t->tint.b == 1.0f &&
                t->tint.a == 1.0f &&
                t->glow.a == 0.0f
            ) {
                continue;
            }
            
            info->tint_color.r *= t->tint.r;
            info->tint_color.g *= t->tint.g;
            info->tint_color.b *= t->tint.b;
            info->tint_color.a *= t->tint.a;
            
            if(t->glow.a > 0) {
                glow_color_sum.r += t->glow.r;
                glow_color_sum.g += t->glow.g;
                glow_color_sum.b += t->glow.b;
                glow_color_sum.a += t->glow.a;
                n_glow_colors++;
            }
            
            if(n_glow_colors > 0) {
                t->glow.r = glow_color_sum.r / n_glow_colors;
                t->glow.g = glow_color_sum.g / n_glow_colors;
                t->glow.b = glow_color_sum.b / n_glow_colors;
                t->glow.a = glow_color_sum.a / n_glow_colors;
            }
        }
    }
    
    if(add_sector_brightness) {
        info->tint_color.r *= (center_sector->brightness / 255.0);
        info->tint_color.g *= (center_sector->brightness / 255.0);
        info->tint_color.b *= (center_sector->brightness / 255.0);
    }
    
    if(delivery_time_ratio_left != LARGE_FLOAT) {
        ALLEGRO_COLOR new_glow;
        float new_scale;
        
        if(delivery_time_ratio_left > 0.5) {
            new_glow =
                interpolate_color(
                    delivery_time_ratio_left, 0.5, 1.0,
                    delivery_color, map_gray(0)
                );
            new_scale = 1.0f;
        } else {
            new_glow = delivery_color;
            new_scale =
                interpolate_number(
                    delivery_time_ratio_left, 0.0, 0.5,
                    0.0f, 1.0f
                );
        }
        
        info->glow_color.r = clamp(info->glow_color.r + new_glow.r, 0.0f, 1.0f);
        info->glow_color.g = clamp(info->glow_color.g + new_glow.g, 0.0f, 1.0f);
        info->glow_color.b = clamp(info->glow_color.b + new_glow.b, 0.0f, 1.0f);
        info->glow_color.a = clamp(info->glow_color.a + new_glow.a, 0.0f, 1.0f);
        
        info->scale *= new_scale;
    }
}


/* ----------------------------------------------------------------------------
 * Returns where a sprite's center should be, for normal mob drawing routines.
 * s:
 *   Sprite to check.
 */
point mob::get_sprite_center(sprite* s) const {
    point p;
    p.x = pos.x + angle_cos * s->offset.x - angle_sin * s->offset.y;
    p.y = pos.y + angle_sin * s->offset.x + angle_cos * s->offset.y;
    return p;
}


/* ----------------------------------------------------------------------------
 * Returns what a sprite's dimensions should be,
 * for normal mob drawing routines.
 * s:
 *   The sprite.
 * scale:
 *   Variable to return the scale used to. Optional.
 */
point mob::get_sprite_dimensions(sprite* s, point* scale) const {
    point dim;
    dim.x = s->file_size.x;
    dim.y = s->file_size.y;
    
    float sucking_mult = 1.0;
    float height_mult = 1.0;
    
    if(height_effect_pivot != LARGE_FLOAT) {
        height_mult +=
            (z - height_effect_pivot) * MOB_HEIGHT_EFFECT_FACTOR;
    }
    height_mult = std::max(height_mult, 1.0f);
    if(ground_sector->is_bottomless_pit && height_mult == 1.0f) {
        height_mult =
            (z - ground_sector->z) /
            (height_effect_pivot - ground_sector->z);
    }
    
    point final_scale = s->scale * sucking_mult * height_mult;
    if(scale) *scale = final_scale;
    
    dim.x *= final_scale.x;
    dim.y *= final_scale.y;
    return dim;
}


/* ----------------------------------------------------------------------------
 * Returns the current sprite of one of the status effects
 * that the mob is under.
 * bmp_scale:
 *   Returns the mob size's scale to apply to the image.
 */
ALLEGRO_BITMAP* mob::get_status_bitmap(float* bmp_scale) const {
    *bmp_scale = 0.0f;
    for(size_t st = 0; st < this->statuses.size(); ++st) {
        status_type* t = this->statuses[st].type;
        if(t->animation_name.empty()) continue;
        sprite* sp = t->anim_instance.get_cur_sprite();
        if(!sp) return NULL;
        *bmp_scale = t->animation_mob_scale;
        return sp->bitmap;
    }
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Handler for when there is no longer any status effect-induced panic.
 */
void mob::handle_panic_loss() {
}


/* ----------------------------------------------------------------------------
 * Handles a status effect being applied.
 * s:
 *   Status type to check.
 */
void mob::handle_status_effect(status_type* s) {
}


/* ----------------------------------------------------------------------------
 * Starts holding the specified mob.
 * m:
 *   Mob to start holding.
 * hitbox_nr:
 *   Number of the hitbox to hold on. INVALID for mob center.
 * offset_dist:
 *   Distance from the hitbox/body center. 1 is full radius.
 * offset_angle:
 *   Hitbox/body angle from which the mob will be held.
 * above_holder:
 *   Is the mob meant to appear above the holder?
 * rotation_method:
 *   How should the held mob rotate? Use HOLD_ROTATION_METHOD_*.
 */
void mob::hold(
    mob* m, const size_t hitbox_nr,
    const float offset_dist, const float offset_angle,
    const bool above_holder, const unsigned char rotation_method
) {
    holding.push_back(m);
    m->holder.m = this;
    m->holder.hitbox_nr = hitbox_nr;
    m->holder.offset_dist = offset_dist;
    m->holder.offset_angle = offset_angle;
    m->holder.above_holder = above_holder;
    m->holder.rotation_method = rotation_method;
    m->fsm.run_event(MOB_EV_HELD, (void*) this);
    
    if(standing_on_mob) {
        if(m->type->weight > 0) {
            //Better inform the mob below that extra weight has been added.
            standing_on_mob->fsm.run_event(MOB_EV_WEIGHT_ADDED, (void*) m);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Checks if a mob is completely off-camera.
 */
bool mob::is_off_camera() const {
    if(parent) return false;
    
    float m_radius;
    if(type->rectangular_dim.x == 0) {
        m_radius = type->radius;
    } else {
        m_radius =
            std::max(
                type->rectangular_dim.x / 2.0,
                type->rectangular_dim.y / 2.0
            );
    }
    
    return !bbox_check(game.cam.box[0], game.cam.box[1], pos, m_radius);
}


/* ----------------------------------------------------------------------------
 * Checks if a mob is resistant to all of the hazards inside a given list.
 * hazards:
 *   List of hazards to check.
 */
bool mob::is_resistant_to_hazards(vector<hazard*> &hazards) const {
    for(size_t h = 0; h < hazards.size(); ++h) {
        if(get_hazard_vulnerability(hazards[h]) != 0.0f) return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Removes a mob from its leader's group.
 */
void mob::leave_group() {
    if(!following_group) return;
    
    mob* group_leader = following_group;
    
    group_leader->group->members.erase(
        find(
            group_leader->group->members.begin(),
            group_leader->group->members.end(),
            this
        )
    );
    
    group_leader->group->init_spots(this);
    
    group_leader->group->change_standby_type_if_needed();
    
    following_group = NULL;
}


/* ----------------------------------------------------------------------------
 * Returns a string containing the FSM state history for this mob.
 * This is used for debugging crashes.
 */
string mob::print_state_history() const {
    string str = "State history: ";
    
    if(fsm.cur_state) {
        str += fsm.cur_state->name;
    } else {
        str += "No current state!";
        return str;
    }
    
    for(size_t s = 0; s < STATE_HISTORY_SIZE; ++s) {
        str += ", " + fsm.prev_state_names[s];
    }
    str += ".";
    
    return str;
}


/* ----------------------------------------------------------------------------
 * Reads the provided script variables, if any, and does stuff with them.
 * svr:
 *   Script var reader to use.
 */
void mob::read_script_vars(const script_var_reader &svr) {
    string team_var;
    
    if(svr.get("team", team_var)) {
        size_t team_nr = string_to_team_nr(team_var);
        if(team_nr == INVALID) {
            log_error(
                "Unknown team name \"" + team_var + "\", when trying to "
                "create a mob of type " + type->name + ", at coordinates " +
                p2s(pos) + "!", NULL
            );
        } else {
            team = team_nr;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Stop holding a mob.
 * m:
 *   Mob to release.
 */
void mob::release(mob* m) {
    for(size_t h = 0; h < holding.size(); ++h) {
        if(holding[h] == m) {
            m->fsm.run_event(MOB_EV_RELEASED, (void*) this);
            holding.erase(holding.begin() + h);
            break;
        }
    }
    
    m->holder.clear();
    
    if(standing_on_mob) {
        if(m->type->weight > 0) {
            //Better inform the mob below that weight has been removed.
            standing_on_mob->fsm.run_event(MOB_EV_WEIGHT_REMOVED, (void*) m);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Safely releases all chomped Pikmin.
 */
void mob::release_chomped_pikmin() {
    for(size_t p = 0; p < chomping_mobs.size(); ++p) {
        release(chomping_mobs[p]);
    }
    chomping_mobs.clear();
}


/* ----------------------------------------------------------------------------
 * Removes all particle generators with the given ID.
 * id:
 *   ID of particle generators to remove.
 */
void mob::remove_particle_generator(const size_t id) {
    for(size_t g = 0; g < particle_generators.size();) {
        if(particle_generators[g].id == id) {
            particle_generators.erase(particle_generators.begin() + g);
        } else {
            ++g;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Respawns an object back to its home.
 */
void mob::respawn() {
    pos = home;
    center_sector = get_sector(pos, NULL, true);
    ground_sector = center_sector;
    z = center_sector->z + 100;
}


/* ----------------------------------------------------------------------------
 * Sends a message to another mob. This calls the mob's "message received"
 * event, with the message as data.
 * receiver:
 *   Mob that will receive the message.
 * msg:
 *   The message.
 */
void mob::send_message(mob* receiver, string &msg) const {
    mob_event* ev = q_get_event(receiver, MOB_EV_RECEIVE_MESSAGE);
    if(!ev) return;
    ev->run(receiver, (void*) &msg, (void*) this);
}


/* ----------------------------------------------------------------------------
 * Sets the mob's animation.
 * nr:
 *   Animation number. It's the animation instance number from the database.
 * pre_named:
 *   If true, the animation has already been named in-engine.
 * auto_start:
 *   After the change, start the new animation from time 0.
 */
void mob::set_animation(
    const size_t nr, const bool pre_named, const bool auto_start
) {
    if(nr >= type->anims.animations.size()) return;
    
    size_t final_nr;
    if(pre_named) {
        if(anim.anim_db->pre_named_conversions.size() <= nr) return;
        final_nr = anim.anim_db->pre_named_conversions[nr];
    } else {
        final_nr = nr;
    }
    
    if(final_nr == INVALID) {
        log_error(
            "Mob " + this->type->name + " tried to switch from " +
            (
                anim.cur_anim ? "animation \"" + anim.cur_anim->name + "\"" :
                "no animation"
            ) +
            " to a non-existent one (with the internal"
            " number of " + i2s(nr) + ")!"
        );
        return;
    }
    
    animation* new_anim = anim.anim_db->animations[final_nr];
    anim.cur_anim = new_anim;
    
    if(new_anim->frames.empty()) {
        anim.cur_frame_index = INVALID;
    } else {
        if(auto_start || anim.cur_frame_index >= anim.cur_anim->frames.size()) {
            anim.start();
        }
    }
}


/* ----------------------------------------------------------------------------
 * Changes a mob's health, relatively or absolutely.
 * add:
 *   If true, change is relative to the current value
 *   (i.e. add or subtract from current health).
 *   If false, simply set to that number.
 * ratio:
 *   If true, the specified value represents the max health ratio.
 *   If false, it's the number in HP.
 * amount:
 *   Health amount.
 */
void mob::set_health(const bool add, const bool ratio, const float amount) {
    float change = amount;
    if(ratio) change = type->max_health * amount;
    float base_nr = 0;
    if(add) base_nr = health;
    
    health = clamp(base_nr + change, 0.0f, type->max_health);
}


/* ----------------------------------------------------------------------------
 * Changes the timer's time and interval.
 * time:
 *   New time.
 */
void mob::set_timer(const float time) {
    script_timer.duration = time;
    script_timer.start();
}


/* ----------------------------------------------------------------------------
 * Sets a script variable's value.
 * name:
 *   The variable's name
 * value:
 *   The variable's new value.
 */
void mob::set_var(const string &name, const string &value) {
    vars[name] = value;
}


/* ----------------------------------------------------------------------------
 * Makes the current mob spawn a new mob, given some spawn information.
 * info:
 *   Structure with information about how to spawn it.
 * type_ptr:
 *   If NULL, the pointer to the mob type is obtained given its
 *   name in the information structure. If not NULL, uses this instead.
 */
mob* mob::spawn(mob_type::spawn_struct* info, mob_type* type_ptr) {
    //First, find the mob.
    if(!type_ptr) {
        type_ptr = game.mob_categories.find_mob_type(info->mob_type_name);
    }
    
    if(!type_ptr) return NULL;
    if(
        type_ptr->category->id == MOB_CATEGORY_PIKMIN &&
        game.states.gameplay->mobs.pikmin_list.size() >=
        game.config.max_pikmin_in_field
    ) {
        return NULL;
    }
    
    point new_xy;
    float new_z = 0;
    float new_angle = 0;
    
    if(info->relative) {
        new_xy = pos + rotate_point(info->coords_xy, angle);
        new_z = z + info->coords_z;
        new_angle = angle + info->angle;
    } else {
        new_xy = info->coords_xy;
        new_z = info->coords_z;
        new_angle = info->angle;
    }
    
    if(!get_sector(new_xy, NULL, true)) {
        //Spawn out of bounds? No way!
        return NULL;
    }
    
    mob* new_mob =
        create_mob(
            type_ptr->category,
            new_xy,
            type_ptr,
            new_angle,
            info->vars
        );
        
    new_mob->z = new_z;
    
    if(type_ptr->category->id == MOB_CATEGORY_TREASURES) {
        //This way, treasures that fall into the abyss respawn at the
        //spawner mob's original spot.
        new_mob->home = home;
    } else {
        new_mob->home = new_xy;
    }
    
    if(info->link_object_to_spawn) {
        links.push_back(new_mob);
    }
    if(info->link_spawn_to_object) {
        new_mob->links.push_back(this);
    }
    if(info->momentum != 0) {
        float a = randomf(0, TAU);
        new_mob->speed.x = cos(a) * info->momentum;
        new_mob->speed.y = sin(a) * info->momentum;
        new_mob->speed_z = info->momentum * 7;
    }
    
    return new_mob;
}


/* ----------------------------------------------------------------------------
 * Sets up stuff for the beginning of the mob's death process.
 */
void mob::start_dying() {
    set_health(false, false, 0.0f);
    
    stop_chasing();
    stop_turning();
    gravity_mult = 1.0;
    
    particle p(
        PARTICLE_TYPE_BITMAP, pos, z + height + 1,
        64, 1.5, PARTICLE_PRIORITY_LOW
    );
    p.bitmap = game.sys_assets.bmp_sparkle;
    p.color = al_map_rgb(255, 192, 192);
    particle_generator pg(0, p, 25);
    pg.number_deviation = 5;
    pg.angle = 0;
    pg.angle_deviation = TAU / 2;
    pg.total_speed = 100;
    pg.total_speed_deviation = 40;
    pg.duration_deviation = 0.5;
    pg.emit(game.states.gameplay->particles);
    
    start_dying_class_specifics();
}


/* ----------------------------------------------------------------------------
 * Sets up stuff for the beginning of the mob's death process.
 * This function is meant to be overridden by child classes.
 */
void mob::start_dying_class_specifics() {
}


/* ----------------------------------------------------------------------------
 * From here on out, the mob's Z changes will be reflected in the height
 * effect.
 */
void mob::start_height_effect() {
    height_effect_pivot = z;
}


/* ----------------------------------------------------------------------------
 * Makes a mob not follow any target any more.
 */
void mob::stop_chasing() {
    chase_info.is_chasing = false;
    chase_info.reached_destination = false;
    chase_info.teleport_z = NULL;
    
    speed.x = speed.y = 0;
}


/* ----------------------------------------------------------------------------
 * Makes the mob stop circling around a point or another mob.
 */
void mob::stop_circling() {
    if(circling_info) {
        delete circling_info;
        circling_info = NULL;
        stop_chasing();
    }
}


/* ----------------------------------------------------------------------------
 * Makes the mob stop following a path graph.
 */
void mob::stop_following_path() {
    if(!path_info) return;
    
    stop_chasing();
    
    delete path_info;
    path_info = NULL;
}


/* ----------------------------------------------------------------------------
 * From here on out, stop using the height effect.
 */
void mob::stop_height_effect() {
    height_effect_pivot = LARGE_FLOAT;
}


/* ----------------------------------------------------------------------------
 * Makes a mob stop riding on a track mob.
 */
void mob::stop_track_ride() {
    if(!track_info) return;
    
    delete track_info;
    track_info = NULL;
    stop_chasing();
    speed_z = 0;
    stop_height_effect();
}


/* ----------------------------------------------------------------------------
 * Makes a mob stop wanting to turn towards some direciton.
 */
void mob::stop_turning() {
    intended_turn_angle = angle;
    intended_turn_pos = NULL;
}


/* ----------------------------------------------------------------------------
 * Makes the mob swallow some of the opponents it has chomped on.
 * nr:
 *   Number of captured opponents to swallow.
 */
void mob::swallow_chomped_pikmin(const size_t nr) {

    size_t total = std::min(nr, chomping_mobs.size());
    
    for(size_t p = 0; p < total; ++p) {
        chomping_mobs[p]->set_health(false, false, 0.0f);
        chomping_mobs[p]->cause_spike_damage(this, true);
        release(chomping_mobs[p]);
    }
    chomping_mobs.clear();
}


/* ----------------------------------------------------------------------------
 * Makes the mob follow a game tick.
 * This basically calls sub-tickers.
 * Think of it this way: when you want to go somewhere,
 * you first think about rotating your body to face that
 * point, and then think about moving your legs.
 * Then, the actual physics go into place, your nerves
 * send signals to the muscles, and gravity, intertia, etc.
 * take over the rest, to make you move.
 * delta_t:
 *   How many seconds to tick by.
 */
void mob::tick(const float delta_t) {
    //Since the mob could be marked for deletion after any little
    //interaction with the world, and since doing logic on a mob that already
    //forgot some things due to deletion is dangerous... Let's constantly
    //check if the mob is scheduled for deletion, and bail if so.
    
    if(to_delete) return;
    
    //Brain.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Object -- Brain");
    }
    tick_brain(delta_t);
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    if(to_delete) return;
    
    //Physics.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Object -- Physics");
    }
    tick_physics(delta_t);
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    if(to_delete) return;
    
    //Misc. logic.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Object -- Misc. logic");
    }
    tick_misc_logic(delta_t);
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    if(to_delete) return;
    
    //Animation.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Object -- Animation");
    }
    tick_animation(delta_t);
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    if(to_delete) return;
    
    //Script.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Object -- Script");
    }
    tick_script(delta_t);
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
    if(to_delete) return;
    
    //Class specifics.
    if(game.perf_mon) {
        game.perf_mon->start_measurement("Object -- Misc. specifics");
    }
    tick_class_specifics(delta_t);
    if(game.perf_mon) {
        game.perf_mon->finish_measurement();
    }
}


/* ----------------------------------------------------------------------------
 * Ticks one game frame into the mob's animations.
 * delta_t:
 *   How many seconds to tick by.
 */
void mob::tick_animation(const float delta_t) {
    float mult = 1.0f;
    for(size_t s = 0; s < this->statuses.size(); ++s) {
        mult *= this->statuses[s].type->anim_speed_multiplier;
    }
    
    vector<size_t> frame_signals;
    bool finished_anim = anim.tick(delta_t* mult, &frame_signals);
    
    if(finished_anim) {
        fsm.run_event(MOB_EV_ANIMATION_END);
    }
    for(size_t s = 0; s < frame_signals.size(); ++s) {
        fsm.run_event(MOB_EV_FRAME_SIGNAL, &frame_signals[s]);
    }
    
    for(size_t h = 0; h < hit_opponents.size();) {
        hit_opponents[h].first -= delta_t;
        if(hit_opponents[h].first <= 0.0f) {
            hit_opponents.erase(hit_opponents.begin() + h);
        } else {
            ++h;
        }
    }
    
    if(parent && parent->limb_anim.anim_db) {
        parent->limb_anim.tick(delta_t* mult);
    }
}


/* ----------------------------------------------------------------------------
 * Ticks the mob's brain for the next frame.
 * This has nothing to do with the mob's individual script.
 * This is related to mob-global things, like
 * thinking about where to move next and such.
 * delta_t:
 *   How many seconds to tick by.
 */
void mob::tick_brain(const float delta_t) {
    //Circling around something.
    if(circling_info) {
        point center =
            circling_info->circling_mob ?
            circling_info->circling_mob->pos :
            circling_info->circling_point;
            
        circling_info->cur_angle +=
            linear_dist_to_angular(
                circling_info->speed * delta_t, circling_info->radius
            ) *
            (circling_info->clockwise ? 1 : -1);
            
        chase(
            center + angle_to_coordinates(
                circling_info->cur_angle, circling_info->radius
            ),
            NULL, false, NULL,
            circling_info->can_free_move,
            3.0f,
            circling_info->speed
        );
    }
    
    //Chasing a target.
    if(chase_info.is_chasing && !chase_info.teleport && speed_z == 0) {
    
        //Calculate where the target is.
        point final_target_pos = get_chase_target();
        
        if(
            dist(pos, final_target_pos) > chase_info.target_dist
        ) {
            //If it still hasn't reached its target
            //(or close enough to the target),
            //time to make it think about how to get there.
            
            //Let the mob think about facing the actual target.
            if(!type->can_free_move) {
                face(get_angle(pos, final_target_pos), NULL);
            }
            
        } else {
            //Reached the chase location.
            
            if(path_info && !path_info->go_straight) {
            
                if(!path_info->is_blocked) {
                    path_info->cur_path_stop_nr++;
                }
                
                if(path_info->cur_path_stop_nr < path_info->path.size()) {
                    //Reached a regular stop while traversing the path.
                    //Think about going to the next, if possible.
                    if(path_info->check_blockage()) {
                        //Oop, there's an obstacle!
                        path_info->is_blocked = true;
                        fsm.run_event(MOB_EV_PATH_BLOCKED);
                    } else {
                        //All good. Head to the next stop.
                        chase(
                            path_info->path[path_info->cur_path_stop_nr]->pos,
                            NULL, false, NULL, true, 3.0f, chase_info.speed
                        );
                    }
                    
                } else if(
                    path_info->cur_path_stop_nr == path_info->path.size()
                ) {
                    //Reached the final stop of the path, but not the goal.
                    //Let's head there.
                    chase(
                        path_info->target_point,
                        NULL, false, NULL, true,
                        path_info->final_target_distance, chase_info.speed
                    );
                    
                } else if(
                    path_info->cur_path_stop_nr == path_info->path.size() + 1
                ) {
                    //Reached the path's goal.
                    chase_info.reached_destination = true;
                    
                }
                
            } else {
                chase_info.reached_destination = true;
            }
            
            if(chase_info.reached_destination) {
                //Reached the final destination. Think about stopping.
                chase_info.speed = 0;
                fsm.run_event(MOB_EV_REACHED_DESTINATION);
            }
        }
        
    }
}


/* ----------------------------------------------------------------------------
 * Code specific for each class. Meant to be overwritten by the child classes.
 * delta_t:
 *   How many seconds to tick by.
 */
void mob::tick_class_specifics(const float delta_t) {
}


/* ----------------------------------------------------------------------------
 * Performs some logic code for this game frame.
 * delta_t:
 *   How many seconds to tick by.
 */
void mob::tick_misc_logic(const float delta_t) {
    time_alive += delta_t;
    
    invuln_period.tick(delta_t);
    
    for(size_t s = 0; s < this->statuses.size(); ++s) {
        statuses[s].tick(delta_t);
        set_health(
            true, true,
            statuses[s].type->health_change_ratio * delta_t
        );
    }
    delete_old_status_effects();
    
    for(size_t g = 0; g < particle_generators.size();) {
        particle_generators[g].tick(
            delta_t, game.states.gameplay->particles
        );
        if(particle_generators[g].emission_interval == 0) {
            particle_generators.erase(particle_generators.begin() + g);
        } else {
            ++g;
        }
    }
    
    if(ground_sector->is_bottomless_pit) {
        if(height_effect_pivot == LARGE_FLOAT) {
            height_effect_pivot = z;
        }
    }
    
    if(type->blocks_carrier_pikmin && health <= 0) {
        game.states.gameplay->path_mgr.handle_obstacle_clear(this);
    }
    
    float ratio = health / type->max_health;
    float ratio_difference = ratio - health_wheel_smoothed_ratio;
    float ratio_max_change_amount = 1 * delta_t;
    
    if(ratio_difference < 0) {
        ratio_difference *= -1;
    }
    
    if(health <= 0) {
        ratio_max_change_amount *= 4.0f;
        if(health_wheel_smoothed_ratio <= 0) {
            health_wheel_alpha -= 3.0f * delta_t;
        }
    }
    
    if(ratio_difference < ratio_max_change_amount) {
        health_wheel_smoothed_ratio = ratio;
    } else {
        if(ratio > health_wheel_smoothed_ratio) {
            health_wheel_smoothed_ratio += ratio_max_change_amount;
        } else {
            health_wheel_smoothed_ratio -= ratio_max_change_amount;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Checks general events in the mob's script for this frame.
 * delta_t:
 *   How many seconds to tick by.
 */
void mob::tick_script(const float delta_t) {
    if(!fsm.cur_state) return;
    
    //Timer events.
    mob_event* timer_ev = q_get_event(this, MOB_EV_TIMER);
    if(script_timer.duration > 0) {
        if(script_timer.time_left > 0) {
            script_timer.tick(delta_t);
            if(script_timer.time_left == 0.0f && timer_ev) {
                timer_ev->run(this);
            }
        }
    }
    
    //Has it reached its home?
    mob_event* reach_dest_ev = q_get_event(this, MOB_EV_REACHED_DESTINATION);
    if(reach_dest_ev && chase_info.reached_destination) {
        reach_dest_ev->run(this);
    }
    
    //Is it dead?
    if(health <= 0 && type->max_health != 0) {
        fsm.run_event(MOB_EV_DEATH, this);
    }
    
    //Check the focused mob.
    if(focused_mob) {
    
        if(focused_mob->health <= 0) {
            fsm.run_event(MOB_EV_FOCUS_DIED);
            fsm.run_event(MOB_EV_FOCUS_OFF_REACH);
        }
        
        //We have to recheck if the focused mob is not NULL, because
        //sending MOB_EV_FOCUS_DIED could've set this to NULL.
        if(focused_mob) {
        
            mob* focus = focused_mob;
            
            mob_event* for_ev =
                q_get_event(this, MOB_EV_FOCUS_OFF_REACH);
                
            if(far_reach != INVALID && for_ev) {
                dist d(pos, focus->pos);
                float face_diff =
                    get_angle_smallest_dif(
                        angle,
                        get_angle(pos, focus->pos)
                    );
                    
                mob_type::reach_struct* r_ptr =
                    &type->reaches[far_reach];
                if(
                    (
                        d > r_ptr->radius_1 +
                        (type->radius + focus->type->radius) ||
                        face_diff > r_ptr->angle_1 / 2.0f
                    ) && (
                        d > r_ptr->radius_2 +
                        (type->radius + focus->type->radius) ||
                        face_diff > r_ptr->angle_2 / 2.0f
                    )
                    
                ) {
                    for_ev->run(this);
                }
                
            }
        }
        
    }
    
    //Itch event.
    if(type->itch_damage > 0 || type->itch_time > 0) {
        itch_time += delta_t;
        mob_event* itch_ev = q_get_event(this, MOB_EV_ITCH);
        if(
            itch_ev &&
            itch_damage > type->itch_damage && itch_time > type->itch_time
        ) {
            itch_ev->run(this);
            itch_damage = 0;
            itch_time = 0;
        }
    }
    
    //Health regeneration.
    if(health > 0) {
        set_health(true, false, type->health_regen * delta_t);
    }
    
    //Check if it got whistled.
    mob_event* whistled_ev = q_get_event(this, MOB_EV_WHISTLED);
    if(game.states.gameplay->whistle.whistling && whistled_ev) {
        if(
            dist(pos, game.states.gameplay->leader_cursor_w) <=
            game.states.gameplay->whistle.radius
        ) {
            whistled_ev->run(this);
        }
    }
    
    //Following a leader.
    if(following_group) {
        mob_event* spot_near_ev = q_get_event(this, MOB_EV_SPOT_IS_NEAR);
        mob_event* spot_far_ev =  q_get_event(this, MOB_EV_SPOT_IS_FAR);
        
        if(spot_near_ev || spot_far_ev) {
            point final_pos =
                following_group->group->anchor +
                following_group->group->get_spot_offset(
                    group_spot_index
                );
            dist d(pos, final_pos);
            if(spot_far_ev && d >= 5) {
                spot_far_ev->run(this, (void*) &final_pos);
            } else if(spot_near_ev && d < 5) {
                spot_near_ev->run(this);
            }
        }
    }
    
    //Far away from home.
    mob_event* far_from_home_ev = q_get_event(this, MOB_EV_FAR_FROM_HOME);
    if(far_from_home_ev) {
        dist d(pos, home);
        if(d >= type->territory_radius) {
            far_from_home_ev->run(this);
        }
    }
    
    //Tick event.
    fsm.run_event(MOB_EV_ON_TICK);
}


/* ----------------------------------------------------------------------------
 * Ticks one frame's worth of time while the mob is riding on a track mob.
 * This updates the mob's position and riding progress.
 * Returns true if the ride is over, false if not.
 */
bool mob::tick_track_ride() {
    track_info->cur_cp_progress +=
        track_info->ride_speed * game.delta_t;
        
    if(track_info->cur_cp_progress >= 1.0f) {
        //Next checkpoint.
        track_info->cur_cp_nr++;
        track_info->cur_cp_progress -= 1.0f;
        
        if(
            track_info->cur_cp_nr ==
            track_info->checkpoints.size() - 1
        ) {
            stop_track_ride();
            return true;
        }
    }
    
    //Teleport to the right spot.
    hitbox* cur_cp =
        track_info->m->get_hitbox(
            track_info->checkpoints[track_info->cur_cp_nr]
        );
    hitbox* next_cp =
        track_info->m->get_hitbox(
            track_info->checkpoints[track_info->cur_cp_nr + 1]
        );
    point cur_cp_pos =
        cur_cp->get_cur_pos(track_info->m->pos, track_info->m->angle);
    point next_cp_pos =
        next_cp->get_cur_pos(track_info->m->pos, track_info->m->angle);
        
    point dest_xy(
        interpolate_number(
            track_info->cur_cp_progress, 0.0f, 1.0f,
            cur_cp_pos.x, next_cp_pos.x
        ),
        interpolate_number(
            track_info->cur_cp_progress, 0.0f, 1.0f,
            cur_cp_pos.y, next_cp_pos.y
        )
    );
    
    float dest_z =
        interpolate_number(
            track_info->cur_cp_progress, 0.0f, 1.0f,
            track_info->m->z + cur_cp->z,
            track_info->m->z + next_cp->z
        );
        
    float dest_angle = get_angle(cur_cp_pos, next_cp_pos);
    
    chase(dest_xy, NULL, true);
    z = dest_z;
    face(dest_angle, NULL);
    
    return false;
}


/* ----------------------------------------------------------------------------
 * Makes the mob lose focus on its currently focused mob.
 */
void mob::unfocus_from_mob() {
    focused_mob = nullptr;
}


/* ----------------------------------------------------------------------------
 * Initializes the members of a mob with anim groups.
 */
mob_with_anim_groups::mob_with_anim_groups() :
    cur_base_anim_nr(INVALID) {
    
}


/* ----------------------------------------------------------------------------
 * Returns the number of an animation, given a base animation number and
 * group number.
 * base_anim_nr:
 *   Base animation number.
 * group_nr:
 *   Group it belongs to.
 * base_anim_total:
 *   Total number of base animations.
 */
size_t mob_with_anim_groups::get_animation_nr_from_base_and_group(
    const size_t base_anim_nr, const size_t group_nr,
    const size_t base_anim_total
) const {
    return group_nr * base_anim_total + base_anim_nr;
}
