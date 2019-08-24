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
#include "../utils/geometry_utils.h"
#include "../utils/string_utils.h"
#include "../vars.h"
#include "pikmin.h"
#include "ship.h"


size_t next_mob_id = 0;

const float MOB_PUSH_THROTTLE_TIMEOUT = 5.0f;


/* ----------------------------------------------------------------------------
 * Creates a mob of no particular type.
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
    chasing(false),
    chase_offset(pos),
    chase_orig_coords(nullptr),
    chase_teleport_z(nullptr),
    chase_teleport(false),
    chase_free_move(false),
    chase_target_dist(0),
    chase_speed(-1),
    reached_destination(false),
    path_info(nullptr),
    circling_info(nullptr),
    following_group(nullptr),
    subgroup_type_ptr(nullptr),
    group(nullptr),
    group_spot_index(INVALID),
    carry_info(nullptr),
    id(next_mob_id),
    health(type->max_health),
    invuln_period(0),
    team(MOB_TEAM_PROP),
    hide(false),
    height_effect_pivot(LARGE_FLOAT),
    on_hazard(nullptr),
    dead(false),
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
    
    if(type->is_obstacle) team = MOB_TEAM_OBSTACLE;
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
 * Adds a bitmap effect to the manager, responsible for color
 * and scaling the mob when it is being delivered to an Onion.
 */
void mob::add_delivery_bitmap_effect(
    bitmap_effect_manager* manager, const float delivery_time_ratio_left,
    const ALLEGRO_COLOR &onion_color
) {

    bitmap_effect se;
    bitmap_effect_props props_half;
    bitmap_effect_props props_end;
    
    se.add_keyframe(0, bitmap_effect_props());
    
    props_half.glow_color = onion_color;
    se.add_keyframe(0.5, props_half);
    
    props_end.glow_color = onion_color;
    props_end.scale = point(0, 0);
    se.add_keyframe(1.0, props_end);
    
    se.set_cur_time(1.0f - delivery_time_ratio_left);
    manager->add_effect(se);
}


/* ----------------------------------------------------------------------------
 * Adds a bitmap effect to the manager, responsible for shading the
 * mob when it is in a shaded sector.
 */
void mob::add_sector_brightness_bitmap_effect(bitmap_effect_manager* manager) {
    if(center_sector->brightness == 255) return;
    
    bitmap_effect se;
    bitmap_effect_props props;
    
    props.tint_color = map_gray(center_sector->brightness);
    
    se.add_keyframe(0, props);
    manager->add_effect(se);
}


/* ----------------------------------------------------------------------------
 * Adds the bitmap effects caused by the status effects to the manager.
 */
void mob::add_status_bitmap_effects(bitmap_effect_manager* manager) {
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
        
        bitmap_effect se;
        bitmap_effect_props props;
        props.tint_color = t->tint;
        props.glow_color = t->glow;
        
        se.add_keyframe(0, props);
        manager->add_effect(se);
    }
}


/* ----------------------------------------------------------------------------
 * Adds a mob to this mob's group.
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
            can_throw_leaders
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
 * Applies the knockback values to a mob.
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
 */
void mob::apply_status_effect( status_type* s, const bool refill, const bool given_by_parent) {
    if(parent && parent->relay_statuses && !given_by_parent) {
        parent->m->apply_status_effect(s, refill, false);
        if(!parent->handle_statuses) return;
    }
    
    if(!given_by_parent && !can_receive_status(s)) {
        return;
    }
    
    //Let's start by sending the status to the child mobs.
    for(size_t m = 0; m < mobs.size(); ++m) {
        if(mobs[m]->parent && mobs[m]->parent->m == this) {
            mobs[m]->apply_status_effect(s, refill, true);
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
 * goal: Use MOB_ACTION_ARACHNORB_PLAN_LOGIC_*.
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
    
    if(goal == MOB_ACTION_ARACHNORB_PLAN_LOGIC_HOME) {
        amount_to_turn = get_angle_cw_dif(angle, get_angle(pos, home));
        if(amount_to_turn > TAU / 2)  amount_to_turn -= TAU;
        if(amount_to_turn < -TAU / 2) amount_to_turn += TAU;
        
        if(fabs(amount_to_turn) < TAU * 0.05) {
            //We can also start moving towards home now.
            amount_to_move = dist(pos, home).to_float();
        }
        
    } else if(goal == MOB_ACTION_ARACHNORB_PLAN_LOGIC_FORWARD) {
        amount_to_move = max_step_distance;
        
    } else if(goal == MOB_ACTION_ARACHNORB_PLAN_LOGIC_CW_TURN) {
        amount_to_turn = randomf(min_turn_angle, TAU * 0.25);
        
    } else if(goal == MOB_ACTION_ARACHNORB_PLAN_LOGIC_CCW_TURN) {
        amount_to_turn = randomf(-TAU * 0.25, -min_turn_angle);
        
    }
    
    amount_to_move = min(amount_to_move, max_step_distance);
    amount_to_turn =
        sign(amount_to_turn) * min((double)fabs(amount_to_turn),(double) max_turn_angle);
        
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
 * Makes the mob attack another mob.
 * Returns true if the attack was successful.
 * victim:   The mob to be attacked.
 * attack_h: Hitbox used for the attack.
 * victim_h: Victim's hitbox that got hit.
 * damage:   If not NULL, total damage caused is returned here.
 */
bool mob::attack(mob* victim, hitbox* attack_h, hitbox* victim_h, float* damage) {
    //TODO refactor, probably when I rethink which mobs want to attack which.
    
    if(victim->parent && victim->parent->relay_damage) {
        bool ret = attack(victim->parent->m, attack_h, victim_h, damage);
        if(!victim->parent->handle_damage) {
            return ret;
        }
    }
    
    float total_damage = 0;
    float attacker_offense = 0;
    float defense_multiplier = 1;
    
    //First, check if this mob cannot be damaged.
    if(victim_h->type != HITBOX_TYPE_NORMAL) {
        //This hitbox can't be damaged! Abort!
        return false;
    }
    
    if(!is_resistant_to_hazards(victim_h->hazards)) {
        //If the hitbox says it has a fire effect, and this
        //mob is not immune to fire, don't let it be a wise-guy;
        //it cannot be able to attack the hitbox.
        return false;
    }
    
    for(size_t h = 0; h < hit_opponents.size(); ++h) {
        if(hit_opponents[h].second == victim) {
            //This opponent has already been hit by this mob recently.
            //Don't let it attack again. This stops the same attack from
            //hitting every single frame.
            return false;
        }
    }
    
    //Calculate the damage.
    if(attack_h) {
        attacker_offense = attack_h->value;
        
        if(!attack_h->hazards.empty()) {
            float max_vulnerability = 0.0f;
            for(size_t h = 0; h < attack_h->hazards.size(); ++h) {
                max_vulnerability =
                    max(
                        victim->get_hazard_vulnerability(attack_h->hazards[h]),
                        max_vulnerability
                    );
            }
            
            if(max_vulnerability == 0.0f) {
                //The victim is immune to this hazard!
                return false;
            } else {
                defense_multiplier = 1.0 / max_vulnerability;
            }
            
        } else {
        
            if(victim->type->default_vulnerability == 0.0f) {
                //The victim is invulnerable to everything about this attack!
                return false;
            } else {
                defense_multiplier = 1.0 / victim->type->default_vulnerability;
            }
        }
        
    } else {
        attacker_offense = 1;
    }
    
    if(victim_h->value == 0.0f) {
        //Hah, this hitbox is invulnerable!
        return false;
    }
    
    defense_multiplier *= victim_h->value;
    
    for(size_t s = 0; s < statuses.size(); ++s) {
        attacker_offense *= statuses[s].type->attack_multiplier;
    }
    for(size_t s = 0; s < victim->statuses.size(); ++s) {
        defense_multiplier *= victim->statuses[s].type->defense_multiplier;
    }
    
    total_damage = attacker_offense * (1.0 / defense_multiplier);
    
    //Actually perform the damage and script-related events.
    victim->set_health(true, false, -total_damage);
    
    hitbox_interaction ev_info(this, victim_h, attack_h);
    victim->fsm.run_event(MOB_EVENT_DAMAGE, (void*) &ev_info);
    
    victim->cause_spike_damage(victim, false);
    
    //Final setup.
    victim->itch_damage += total_damage;
    hit_opponents.push_back(
        make_pair(OPPONENT_HIT_REGISTER_TIMEOUT, victim)
    );
    
    //Smack particle effect.
    point smack_p_pos =
        pos +
        (victim->pos - pos) *
        (type->radius / (type->radius + victim->type->radius));
    sfx_attack.play(0.06, false, 0.6f);
    particle smack_p(
        PARTICLE_TYPE_SMACK, smack_p_pos,
        max(victim->z + victim->type->height + 1, z + type->height + 1),
        64, SMACK_PARTICLE_DUR, PARTICLE_PRIORITY_MEDIUM
    );
    smack_p.bitmap = bmp_smack;
    smack_p.color = al_map_rgb(255, 160, 128);
    particles.add(smack_p);
    
    if(damage) *damage = total_damage;
    return true;
}


/* ----------------------------------------------------------------------------
 * Sets up data for a mob to become carriable.
 * destination: Where to carry it. Use CARRY_DESTINATION_*.
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
                MOB_EVENT_FOCUSED_MOB_UNCARRIABLE
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
 * added:        The Pikmin that got added, if any.
 * removed:      The Pikmin that got removed, if any.
 * target_mob:   Return the target mob (if any) here.
 * target_point: Return the target point here.
 */
bool mob::calculate_carrying_destination(
    mob* added, mob* removed, mob** target_mob, point* target_point
) {
    if(!carry_info) return false;
    
    //For starters, check if this is to be carried to the ship.
    //Get that out of the way if so.
    if(carry_info->destination == CARRY_DESTINATION_SHIP) {
    
        ship* closest_ship = NULL;
        dist closest_ship_dist;
        
        for(size_t s = 0; s < ships.size(); ++s) {
            ship* s_ptr = ships[s];
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
    unordered_set<pikmin_type*> available_onions;
    
    //First, check which Onions are even available.
    for(size_t o = 0; o < onions.size(); o++) {
        onion* o_ptr = onions[o];
        if(o_ptr->activated) {
            available_onions.insert(o_ptr->oni_type->pik_type);
        }
    }
    
    if(available_onions.empty()) {
        //No Onions?! Well...make the Pikmin stuck.
        *target_mob = NULL;
        return false;
    }
    
    //Count how many of each type there are carrying.
    for(size_t p = 0; p < type->max_carriers; ++p) {
        pikmin* pik_ptr = NULL;
        
        if(carry_info->spot_info[p].state != CARRY_SPOT_USED) continue;
        
        pik_ptr = (pikmin*) carry_info->spot_info[p].pik_ptr;
        
        //If it doesn't have an Onion, it won't even count.
        if(available_onions.find(pik_ptr->pik_type) == available_onions.end()) {
            continue;
        }
        
        type_quantity[pik_ptr->pik_type]++;
    }
    
    //Then figure out what are the majority types.
    unsigned most = 0;
    for(auto t = type_quantity.begin(); t != type_quantity.end(); ++t) {
        if(t->second > most) {
            most = t->second;
            majority_types.clear();
        }
        if(t->second == most) majority_types.push_back(t->first);
    }
    
    //If we ended up with no candidates, pick a type at random,
    //out of all possible types.
    if(majority_types.empty()) {
        for(
            auto t = available_onions.begin();
            t != available_onions.end(); ++t
        ) {
            majority_types.push_back(*t);
        }
    }
    
    pikmin_type* decided_type = NULL;
    
    //Now let's pick an Onion from the candidates.
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
    size_t onion_nr = 0;
    for(; onion_nr < onions.size(); ++onion_nr) {
        if(onions[onion_nr]->oni_type->pik_type == decided_type) {
            break;
        }
    }
    
    //Finally, set the destination data.
    *target_mob = onions[onion_nr];
    *target_point = (*target_mob)->pos;
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Makes the mob cause spike damage to another mob.
 * victim:       The mob that will be damaged.
 * is_ingestion: If true, the attacker just got eaten.
 *   If false, it merely got hurt.
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
        pg.emit(particles);
    }
}


/* ----------------------------------------------------------------------------
 * Sets a target for the mob to follow.
 * offs_*:          Coordinates of the target, relative to either the
 *   world origin, or another point, specified in the next parameters.
 * orig_*:          Pointers to changing coordinates. If NULL, it is
 *   the world origin. Use this to make the mob follow another mob
 *   wherever they go, for instance.
 * teleport:        If true, the mob teleports to that spot,
 *   instead of walking to it.
 * teleport_z:      Teleports to this Z coordinate, too.
 * free_move:       If true, the mob can go to a direction they're not facing.
 * target_distance: Distance from the target in which the mob is
 *   considered as being there.
 * speed:           Speed at which to go to the target. -1 uses the mob's speed.
 */
void mob::chase(
    const point &offset, point* orig_coords,
    const bool teleport, float* teleport_z,
    const bool free_move, const float target_distance, const float speed
) {

    this->chase_offset = offset;
    this->chase_orig_coords = orig_coords;
    this->chase_teleport = teleport;
    this->chase_teleport_z = teleport_z;
    this->chase_free_move = free_move || type->can_free_move;
    this->chase_target_dist = target_distance;
    this->chase_speed = (speed == -1 ? get_base_speed() : speed);
    
    chasing = true;
    reached_destination = false;
}


/* ----------------------------------------------------------------------------
 * Makes a mob chomp another mob. Mostly applicable for enemies chomping
 * on Pikmin.
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
        m, hitbox_info->body_part_index, h_offset_dist, h_offset_angle, true
    );
    
    m->focus_on_mob(this);
    chomping_mobs.push_back(m);
}


/* ----------------------------------------------------------------------------
 * Makes the mob start circling around a point or another mob.
 * m:             The mob to circle around.
 *   If NULL, circle around a point instead.
 * p:             The point to circle around, if any.
 * radius:        Circle these many units around the target.
 * clockwise:     Circle clockwise or counter-clockwise?
 * speed:         Speed at which to move.
 * can_free_move: Can the mob move freely, or only forward?
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
                lose_panic_from_status();
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
        if(statuses[s].type->turns_invisible) has_invisibility_status = true;
        break;
    }
}


/* ----------------------------------------------------------------------------
 * Draws the entirety of the mob.
 * effect_manager: Effect manager to use, if any.
 */
void mob::draw(bitmap_effect_manager* effect_manager) {

    if(hide) return;
    
    draw_mob(effect_manager);
}


/* ----------------------------------------------------------------------------
 * Draws the limb that connects this mob to its parent.
 */
void mob::draw_limb(bitmap_effect_manager* effect_manager) {
    if(hide) return;
    if(!parent) return;
    if(!parent->limb_anim.anim_db) return;
    sprite* sprite_to_use = parent->limb_anim.get_cur_sprite();
    if(!sprite_to_use) return;
    
    bitmap_effect_manager internal_manager;
    if(!effect_manager) {
        effect_manager = &internal_manager;
    }
    add_status_bitmap_effects(effect_manager);
    add_sector_brightness_bitmap_effect(effect_manager);
    
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
    
    draw_bitmap_with_effects(
        sprite_to_use->bitmap,
        (parent_end + child_end) / 2,
        point(length, parent->limb_thickness),
        p2c_angle,
        effect_manager
    );
}


/* ----------------------------------------------------------------------------
 * Draws just the mob. This is a generic function, and can be overwritten
 * by child classes.
 * effect_manager: Effect manager to base on.
 */
void mob::draw_mob(bitmap_effect_manager* effect_manager) {
    sprite* s_ptr = anim.get_cur_sprite();
    
    if(!s_ptr) return;
    
    bitmap_effect_manager internal_manager;
    if(!effect_manager) {
        effect_manager = &internal_manager;
    }
    add_status_bitmap_effects(effect_manager);
    add_sector_brightness_bitmap_effect(effect_manager);
    
    point draw_pos = get_sprite_center(s_ptr);
    point draw_size = get_sprite_dimensions(s_ptr);
    
    draw_bitmap_with_effects(
        s_ptr->bitmap,
        draw_pos, draw_size,
        angle + s_ptr->angle, effect_manager
    );
}


/* ----------------------------------------------------------------------------
 * Makes a mob intend to face a new angle.
 * new_angle: Face this angle.
 * new_pos:   If this is not NULL, turn towards this point every frame, instead.
 */
void mob::face(const float new_angle, point* new_pos) {
    if(carry_info) return; //If it's being carried, it shouldn't rotate.
    intended_turn_angle = new_angle;
    intended_turn_pos = new_pos;
}


//Normally, the spirit's diameter is the enemy's. Multiply the spirit by this.
const float ENEMY_SPIRIT_SIZE_MULT = 0.7;
//Maximum diameter an enemy's spirit can be.
const float ENEMY_SPIRIT_MAX_SIZE = 128;
//Minimum diameter an enemy's spirit can be.
const float ENEMY_SPIRIT_MIN_SIZE = 16;

/* ----------------------------------------------------------------------------
 * Sets up stuff for the end of the mob's dying process.
 */
void mob::finish_dying() {
    if(dead) return;
    dead = true;
    
    if(type->category->id == MOB_CATEGORY_ENEMIES) {
        //TODO move this to the enemy class.
        enemy* e_ptr = (enemy*) this;
        if(e_ptr->ene_type->drops_corpse) {
            become_carriable(CARRY_DESTINATION_ONION);
            e_ptr->fsm.set_state(ENEMY_EXTRA_STATE_CARRIABLE_WAITING);
        }
        particle par(
            PARTICLE_TYPE_ENEMY_SPIRIT, pos, LARGE_FLOAT,
            clamp(
                type->radius * 2 * ENEMY_SPIRIT_SIZE_MULT,
                ENEMY_SPIRIT_MIN_SIZE, ENEMY_SPIRIT_MAX_SIZE
            ),
            2, PARTICLE_PRIORITY_MEDIUM
        );
        par.bitmap = bmp_enemy_spirit;
        par.speed.x = 0;
        par.speed.y = -50;
        par.friction = 0.5;
        par.gravity = 0;
        par.color = al_map_rgb(255, 192, 255);
        particles.add(par);
    }
    
    release_chomped_pikmin();
}


/* ----------------------------------------------------------------------------
 * Makes the mob focus on m2.
 */
void mob::focus_on_mob(mob* m) {
    unfocus_from_mob();
    focused_mob = m;
}


/* ----------------------------------------------------------------------------
 * Makes the mob start following a path. This populates the path_info
 * class member, and calculates a path to take.
 * target:                Target point to reach.
 * can_continue:          If true, it is possible for the new path to continue
 *   from where the old one left off, if there was an old one.
 * speed:                 Speed at which to travel. -1 uses the mob's speed.
 * final_target_distance: For the final chase, from the last path stop to
 *   the destination, use this for the target distance parameter.
 * Returns whether or not there is a path available.
 */
bool mob::follow_path(
    const point &target, const bool can_continue,
    const float speed, const float final_target_distance
) {
    vector<path_stop*> old_path;
    size_t old_path_stop_nr = INVALID;
    
    if(can_continue && path_info) {
        old_path = path_info->path;
        old_path_stop_nr = path_info->cur_path_stop_nr;
    }
    
    if(path_info) {
        delete path_info;
    }
    
    path_info = new path_info_struct(this, target);
    path_info->final_target_distance = final_target_distance;
    
    if(can_continue && old_path.size() >= 2 && path_info->path.size() >= 2) {
        path_stop* next_stop = old_path[old_path_stop_nr];
        for(size_t s = 1; s < path_info->path.size(); ++s) {
            if(path_info->path[s] == next_stop) {
                //If before, the mob was already heading towards this stop,
                //then just continue the new journey from there.
                path_info->cur_path_stop_nr = s;
                break;
            }
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
float mob::get_base_speed() {
    return this->type->move_speed;
}


/* ----------------------------------------------------------------------------
 * Returns the actual location of the movement target.
 */
point mob::get_chase_target() {
    point p = chase_offset;
    if(chase_orig_coords) p += (*chase_orig_coords);
    return p;
}


/* ----------------------------------------------------------------------------
 * Returns the closest hitbox to a point, belonging to a mob's current frame
 * of animation and position.
 * p:      The point.
 * h_type: Type of hitbox. INVALID means any.
 * d:      Return the distance here, optionally.
 */
hitbox* mob::get_closest_hitbox(const point &p, const size_t h_type, dist* d) {
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
 */
float mob::get_hazard_vulnerability(hazard* h_ptr) {
    float vulnerability_value = type->default_vulnerability;
    auto vul = type->hazard_vulnerabilities.find(h_ptr);
    if(vul != type->hazard_vulnerabilities.end()) {
        vulnerability_value = vul->second;
    }
    
    return vulnerability_value;
}


/* ----------------------------------------------------------------------------
 * Returns the hitbox in the current animation with
 * the specified number.
 */
hitbox* mob::get_hitbox(const size_t nr) {
    sprite* s = anim.get_cur_sprite();
    if(!s) return NULL;
    if(s->hitboxes.empty()) return NULL;
    return &s->hitboxes[nr];
}


/* ----------------------------------------------------------------------------
 * When a mob is meant to be held by a hitbox, this function returns where
 * in the hitbox the mob currently is.
 * mob_to_hold:  The mob that will be held.
 * h_ptr:        Pointer to the hitbox to check.
 * offset_dist:  The distance from the center of the hitbox is returned here.
 *   1 means the full radius.
 * offset_angle: The angle the mob to hold makes with the hitbox's center is
 *   returned here.
 */
void mob::get_hitbox_hold_point(
    mob* mob_to_hold, hitbox* h_ptr, float* offset_dist, float* offset_angle
) {
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
size_t mob::get_latched_pikmin_amount() {
    size_t total = 0;
    for(size_t p = 0; p < pikmin_list.size(); ++p) {
        pikmin* p_ptr = pikmin_list[p];
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
float mob::get_latched_pikmin_weight() {
    float total = 0;
    for(size_t p = 0; p < pikmin_list.size(); ++p) {
        pikmin* p_ptr = pikmin_list[p];
        if(p_ptr->focused_mob != this) continue;
        if(p_ptr->holder.m != this) continue;
        if(!p_ptr->latched) continue;
        total += p_ptr->type->weight;
    }
    return total;
}


/* ----------------------------------------------------------------------------
 * Returns where a sprite's center should be, for normal mob drawing routines.
 */
point mob::get_sprite_center(sprite* s) {
    point p;
    p.x = pos.x + angle_cos * s->offset.x - angle_sin * s->offset.y;
    p.y = pos.y + angle_sin * s->offset.x + angle_cos * s->offset.y;
    return p;
}


/* ----------------------------------------------------------------------------
 * Returns what a sprite's dimensions should be,
 * for normal mob drawing routines.
 * s:     the sprite.
 * scale: variable to return the scale used to. Optional.
 */
point mob::get_sprite_dimensions(sprite* s, float* scale) {
    point dim;
    dim.x = s->file_size.x;
    dim.y = s->file_size.y;
    dim.x *= s->scale.x;
    dim.y *= s->scale.y;
    
    float sucking_mult = 1.0;
    float height_mult = 1.0;
    
    if(height_effect_pivot != LARGE_FLOAT) {
        height_mult +=
            (z - height_effect_pivot) * MOB_HEIGHT_EFFECT_FACTOR;
    }
    height_mult = max(height_mult, 1.0f);
    if(ground_sector->is_bottomless_pit && height_mult == 1.0f) {
        height_mult =
            (z - ground_sector->z) /
            (height_effect_pivot - ground_sector->z);
    }
    
    float final_scale = sucking_mult * height_mult;
    if(scale) *scale = final_scale;
    
    dim *= final_scale;
    return dim;
}


/* ----------------------------------------------------------------------------
 * Returns the current sprite of one of the status effects
 * that the mob is under.
 * bmp_scale: Returns the mob size's scale to apply to the image.
 */
ALLEGRO_BITMAP* mob::get_status_bitmap(float* bmp_scale) {
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
 * Starts holding the specified mob.
 * m:            Mob to start holding.
 * hitbox_nr:    Number of the hitbox to hold on. INVALID for mob center.
 * offset_dist:  Distance from the hitbox/body center. 1 is full radius.
 * offset_angle: Hitbox/body angle from which the mob will be held.
 * above_holder: Is the mob meant to appear above the holder?
 */
void mob::hold(
    mob* m, const size_t hitbox_nr,
    const float offset_dist, const float offset_angle,
    const bool above_holder
) {
    holding.push_back(m);
    m->holder.m = this;
    m->holder.hitbox_nr = hitbox_nr;
    m->holder.offset_dist = offset_dist;
    m->holder.offset_angle = offset_angle;
    m->holder.above_holder = above_holder;
    m->fsm.run_event(MOB_EVENT_HELD, (void*) this);
}


/* ----------------------------------------------------------------------------
 * Checks if a mob is completely off-camera.
 */
bool mob::is_off_camera() {
    if(parent) return false;
    
    float m_radius;
    if(type->rectangular_dim.x == 0) {
        m_radius = type->radius;
    } else {
        m_radius =
            max(type->rectangular_dim.x / 2.0, type->rectangular_dim.y / 2.0);
    }
    
    return !bbox_check(cam_box[0], cam_box[1], pos, m_radius);
}


/* ----------------------------------------------------------------------------
 * Checks if a mob is resistant to all of the hazards inside a given list.
 */
bool mob::is_resistant_to_hazards(vector<hazard*> &hazards) {
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
string mob::print_state_history() {
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
 */
void mob::read_script_vars(const string &vars) {
    string team_str = get_var_value(vars, "team", "");
    if(!team_str.empty()) {
        size_t team_nr = string_to_team_nr(team_str);
        if(team_nr == INVALID) {
            log_error(
                "Unknown team name \"" + team_str + "\", when trying to "
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
 */
void mob::release(mob* m) {
    for(size_t h = 0; h < holding.size(); ++h) {
        if(holding[h] == m) {
            m->fsm.run_event(MOB_EVENT_RELEASED, (void*) this);
            holding.erase(holding.begin() + h);
            break;
        }
    }
    
    m->holder.clear();
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
 */
void mob::send_message(mob* receiver, string &msg) {
    mob_event* ev = q_get_event(receiver, MOB_EVENT_RECEIVE_MESSAGE);
    if(!ev) return;
    ev->run(receiver, (void*) &msg, (void*) this);
}


/* ----------------------------------------------------------------------------
 * Sets the mob's animation.
 * nr:         Animation number.
 *   It's the animation instance number from the database.
 * pre_named:  If true, the animation has already been named in-engine.
 * auto_start: After the change, start the new animation from time 0.
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
 * add:    If true, change is relative to the current value
 *   (i.e. add or subtract from current health).
 *   If false, simply set to that number.
 * ratio:  If true, the specified value represents the max health ratio.
 *   If false, it's the number in HP.
 * amount: Health amount.
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
 * time: New time.
 */
void mob::set_timer(const float time) {
    script_timer.duration = time;
    script_timer.start();
}


/* ----------------------------------------------------------------------------
 * Sets a script variable's value.
 * name:  The variable's name
 * value: The variable's new value.
 */
void mob::set_var(const string &name, const string &value) {
    vars[name] = value;
}


/* ----------------------------------------------------------------------------
 * Can this mob damage v? Teams and other factors are used to decide this.
 */
bool mob::can_damage(mob* v) {
    if(team == v->team && team != MOB_TEAM_NEUTRAL) {
        //Teammates can't hurt each other.
        return false;
    }
    if(v->team == MOB_TEAM_PROP) {
        //Props aren't meant to be hurt.
        return false;
    }
    if(type->is_projectile && !v->type->projectiles_can_damage) {
        //Projectiles can't hurt those which are invulnerable to them.
        return false;
    }
    if(
        team == MOB_TEAM_OBSTACLE &&
        (v->team < MOB_TEAM_PLAYER_1 || v->team > MOB_TEAM_PLAYER_4)
    ) {
        //Obstacles can only hurt Pikmin and leaders.
        return false;
    }
    if(
        v->team == MOB_TEAM_OBSTACLE &&
        type->category->id != MOB_CATEGORY_PIKMIN &&
        type->category->id != MOB_CATEGORY_TOOLS &&
        !type->is_projectile
    ) {
        //Only Pikmin, tools, and projectiles can hurt obstacles.
        return false;
    }
    if(
        v->type->category->id == MOB_CATEGORY_PIKMIN &&
        ((pikmin*) v)->is_seed_or_sprout
    ) {
        //Seed/sprout Pikmin should not be attacked or targetted.
        return false;
    }
    if(v->team == MOB_TEAM_TOP) {
        //Top of the foodchain cannot be hurt.
        return false;
    }
    if(team == MOB_TEAM_BOTTOM) {
        //Bottom of the foodchain cannot attack.
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Makes the current mob spawn a new mob, given some spawn information.
 * info:     Structure with information about how to spawn it.
 * type_ptr: If NULL, the pointer to the mob type is obtained given its
 *   name in the information structure. If not NULL, uses this instead.
 */
mob* mob::spawn(mob_type::spawn_struct* info, mob_type* type_ptr) {
    //First, find the mob.
    if(!type_ptr) {
        type_ptr = mob_categories.find_mob_type(info->mob_type_name);
    }
    
    if(!type_ptr) return NULL;
    if(
        type_ptr->category->id == MOB_CATEGORY_PIKMIN &&
        pikmin_list.size() >= max_pikmin_in_field
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
        PARTICLE_TYPE_BITMAP, pos, z + type->height + 1,
        64, 1.5, PARTICLE_PRIORITY_LOW
    );
    p.bitmap = bmp_sparkle;
    p.color = al_map_rgb(255, 192, 192);
    particle_generator pg(0, p, 25);
    pg.number_deviation = 5;
    pg.angle = 0;
    pg.angle_deviation = TAU / 2;
    pg.total_speed = 100;
    pg.total_speed_deviation = 40;
    pg.duration_deviation = 0.5;
    pg.emit(particles);
    
    start_dying_class_specific();
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
    chasing = false;
    reached_destination = false;
    chase_teleport_z = NULL;
    
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
 * Makes a mob stop wanting to turn towards some direciton.
 */
void mob::stop_turning() {
    intended_turn_angle = angle;
    intended_turn_pos = NULL;
}


/* ----------------------------------------------------------------------------
 * Makes the mob swallow some of the opponents it has chomped on.
 * nr: Number of captured opponents to swallow.
 */
void mob::swallow_chomped_pikmin(const size_t nr) {

    size_t total = min(nr, chomping_mobs.size());
    
    for(size_t p = 0; p < total; ++p) {
        chomping_mobs[p]->set_health(false, false, 0.0f);
        chomping_mobs[p]->dead = true;
        chomping_mobs[p]->cause_spike_damage(this, true);
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
 */
void mob::tick() {
    //Since the mob could be marked for deletion after any little
    //interaction with the world, and since doing logic on a mob that already
    //forgot some things due to deletion is dangerous... Let's constantly
    //check if the mob is scheduled for deletion, and bail if so.
    
    if(to_delete) return;
    tick_brain();
    if(to_delete) return;
    tick_physics();
    if(to_delete) return;
    tick_misc_logic();
    if(to_delete) return;
    tick_animation();
    if(to_delete) return;
    tick_script();
    if(to_delete) return;
    tick_class_specifics();
}


/* ----------------------------------------------------------------------------
 * Ticks one game frame into the mob's animations.
 */
void mob::tick_animation() {
    float mult = 1.0f;
    for(size_t s = 0; s < this->statuses.size(); ++s) {
        mult *= this->statuses[s].type->anim_speed_multiplier;
    }
    
    vector<size_t> frame_signals;
    bool finished_anim = anim.tick(delta_t* mult, &frame_signals);
    
    if(finished_anim) {
        fsm.run_event(MOB_EVENT_ANIMATION_END);
    }
    for(size_t s = 0; s < frame_signals.size(); ++s) {
        fsm.run_event(MOB_EVENT_FRAME_SIGNAL, &frame_signals[s]);
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
 */
void mob::tick_brain() {
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
    if(chasing && !chase_teleport && speed_z == 0) {
    
        //Calculate where the target is.
        point final_target_pos = get_chase_target();
        
        if(
            dist(pos, final_target_pos) > chase_target_dist
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
            
            bool stuck_at_obstacle = false;
            if(path_info && !path_info->go_straight) {
                path_info->cur_path_stop_nr++;
                if(path_info->cur_path_stop_nr == path_info->path.size()) {
                    //Reached the final stop of the path, but not the goal.
                    
                    if(!path_info->obstacle_ptrs.empty()) {
                        //If there's an obstacle in the path, the last stop
                        //on the path actually means it's the last possible
                        //stop before the obstacle. Meaning the object
                        //is now facing an obstacle.
                        stuck_at_obstacle = true;
                        reached_destination = true;
                        
                    } else {
                        //Time to head towards the actual goal.
                        chase(
                            path_info->target_point,
                            NULL, false, NULL, true,
                            path_info->final_target_distance, chase_speed
                        );
                    }
                    
                } else if(
                    path_info->cur_path_stop_nr == path_info->path.size() + 1
                ) {
                    //Reached the final destination.
                    reached_destination = true;
                    
                } else {
                    //Reached a stop while traversing the path.
                    //Think about going to the next.
                    chase(
                        path_info->path[path_info->cur_path_stop_nr]->pos,
                        NULL, false, NULL, true, 3.0f, chase_speed
                    );
                }
                
            } else {
                reached_destination = true;
            }
            
            if(reached_destination) {
                //Reached the final destination. Think about stopping.
                chase_speed = 0;
                fsm.run_event(
                    MOB_EVENT_REACHED_DESTINATION,
                    (stuck_at_obstacle ? (void*) stuck_at_obstacle : NULL)
                );
            }
        }
        
    }
}


/* ----------------------------------------------------------------------------
 * Code specific for each class. Meant to be overwritten by the child classes.
 */
void mob::tick_class_specifics() {
}


/* ----------------------------------------------------------------------------
 * Performs some logic code for this game frame.
 */
void mob::tick_misc_logic() {
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
        particle_generators[g].tick(delta_t, particles);
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
}


/* ----------------------------------------------------------------------------
 * Ticks the mob's actual physics procedures:
 * falling because of gravity, moving forward, etc.
 */
void mob::tick_physics() {
    if(!ground_sector) {
        //Object is placed out of bounds.
        return;
    }
    
    //Movement.
    bool finished_moving = false;
    bool doing_slide = false;
    
    point new_pos = pos;
    float new_z = z;
    sector* new_ground_sector = ground_sector;
    float pre_move_ground_z = ground_sector->z;
    
    point move_speed = speed;
    
    float radius_to_use = type->radius;
    
    //Change the facing angle to the angle the mob wants to face.
    if(angle > TAU / 2)  angle -= TAU;
    if(angle < -TAU / 2) angle += TAU;
    if(intended_turn_pos) {
        intended_turn_angle = get_angle(pos, *intended_turn_pos);
    }
    if(intended_turn_angle > TAU / 2)  intended_turn_angle -= TAU;
    if(intended_turn_angle < -TAU / 2) intended_turn_angle += TAU;
    
    float angle_dif = intended_turn_angle - angle;
    if(angle_dif > TAU / 2)  angle_dif -= TAU;
    if(angle_dif < -TAU / 2) angle_dif += TAU;
    
    float movement_speed_mult = 1.0f;
    for(size_t s = 0; s < this->statuses.size(); ++s) {
        movement_speed_mult *= this->statuses[s].type->speed_multiplier;
    }
    
    angle +=
        sign(angle_dif) * min(
            (double) (type->rotation_speed * movement_speed_mult * delta_t),
            (double) fabs(angle_dif)
        );
        
    if(holder.m) {
        point final_pos = holder.get_final_pos(&z);
        z += 1.0f; //Added visibility for latched Pikmin.
        speed_z = 0;
        angle = get_angle(final_pos, holder.m->pos);
        stop_turning();
        chase(final_pos, NULL, true);
    }
    
    angle_cos = cos(angle);
    angle_sin = sin(angle);
    
    if(chasing) {
        point final_target_pos = get_chase_target();
        
        if(chase_teleport) {
            sector* sec =
                get_sector(final_target_pos, NULL, true);
            if(!sec) {
                //No sector, invalid teleport. No move.
                return;
                
            } else {
                if(chase_teleport_z) {
                    z = *chase_teleport_z;
                }
                ground_sector = sec;
                center_sector = sec;
                speed.x = speed.y = 0;
                pos = final_target_pos;
                finished_moving = true;
            }
            
        } else {
        
            //Make it go to the direction it wants.
            float d = dist(pos, final_target_pos).to_float();
            
            float move_amount =
                min(
                    (double) (d / delta_t),
                    (double) chase_speed * movement_speed_mult
                );
                
            bool can_free_move = chase_free_move || d <= 10.0;
            
            float movement_angle =
                can_free_move ?
                get_angle(pos, final_target_pos) :
                angle;
                
            move_speed.x = cos(movement_angle) * move_amount;
            move_speed.y = sin(movement_angle) * move_amount;
        }
        
    }
    
    //If another mob is pushing it.
    if(push_amount != 0.0f) {
        //Overly-aggressive pushing results in going through walls.
        //Let's place a cap.
        push_amount =
            min(push_amount, (float) (type->radius / delta_t) * 4);
            
        //If the mob spawned recently, throttle its push. This avoids a bundle
        //of recently-spawned objects from pushing each other with insane force.
        //Setting the amount to 0 means it'll use the push provided by
        //MOB_PUSH_EXTRA_AMOUNT exclusively.
        if(time_alive < MOB_PUSH_THROTTLE_TIMEOUT) {
            push_amount = 0;
        }
        
        move_speed.x +=
            cos(push_angle) * (push_amount + MOB_PUSH_EXTRA_AMOUNT);
        move_speed.y +=
            sin(push_angle) * (push_amount + MOB_PUSH_EXTRA_AMOUNT);
    }
    
    push_amount = 0;
    bool touched_wall = false;
    
    
    //Try placing it in the place it should be at, judging
    //from the movement speed.
    while(!finished_moving) {
    
        if(move_speed.x == 0 && move_speed.y == 0) break;
        
        //Start by checking sector collisions.
        //For this, we will only check if the mob is intersecting
        //with any edge. With this, we trust that mobs can't go so fast
        //that they're fully on one side of an edge in one frame,
        //and the other side on the next frame.
        //It's pretty naive...but it works!
        bool successful_move = true;
        
        new_pos.x = pos.x + delta_t* move_speed.x;
        new_pos.y = pos.y + delta_t* move_speed.y;
        new_z = z;
        new_ground_sector = ground_sector;
        set<edge*> intersecting_edges;
        
        //Get the sector the mob will be on.
        sector* new_center_sector = get_sector(new_pos, NULL, true);
        sector* step_sector = new_center_sector;
        
        if(!new_center_sector) {
            //Out of bounds. No movement.
            break;
        } else {
            new_ground_sector = new_center_sector;
        }
        
        if(z < new_center_sector->z) {
            //If it'd end up under the ground, refuse the move.
            break;
        }
        
        //Before checking the edges, let's consult the blockmap and look at
        //the edges in the same block the mob is on.
        //This way, we won't check for edges that are really far away.
        //Use the bounding box to know which blockmap blocks the mob will be on.
        size_t bx1 = cur_area_data.bmap.get_col(new_pos.x - radius_to_use);
        size_t bx2 = cur_area_data.bmap.get_col(new_pos.x + radius_to_use);
        size_t by1 = cur_area_data.bmap.get_row(new_pos.y - radius_to_use);
        size_t by2 = cur_area_data.bmap.get_row(new_pos.y + radius_to_use);
        
        if(
            bx1 == INVALID || bx2 == INVALID ||
            by1 == INVALID || by2 == INVALID
        ) {
            //Somehow out of bounds. No movement.
            break;
        }
        
        float move_angle;
        float total_move_speed;
        coordinates_to_angle(
            move_speed, &move_angle, &total_move_speed
        );
        
        //Angle to slide towards.
        float slide_angle = move_angle;
        //Difference between the movement angle and the slide.
        float slide_angle_dif = 0;
        
        edge* e_ptr = NULL;
        
        //Go through the blocks, to find intersections, and set up some things.
        for(size_t bx = bx1; bx <= bx2; ++bx) {
            for(size_t by = by1; by <= by2; ++by) {
            
                vector<edge*>* edges = &cur_area_data.bmap.edges[bx][by];
                
                for(size_t e = 0; e < edges->size(); ++e) {
                
                    e_ptr = (*edges)[e];
                    bool is_edge_blocking = false;
                    
                    if(
                        !circle_intersects_line(
                            new_pos, radius_to_use,
                            point(
                                e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y
                            ),
                            point(
                                e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y
                            ),
                            NULL, NULL
                        )
                    ) {
                        continue;
                    }
                    
                    if(e_ptr->sectors[0] && e_ptr->sectors[1]) {
                    
                        if(
                            e_ptr->sectors[0]->type == SECTOR_TYPE_BLOCKING ||
                            e_ptr->sectors[1]->type == SECTOR_TYPE_BLOCKING
                        ) {
                            is_edge_blocking = true;
                        }
                        
                        if(!is_edge_blocking) {
                            if(
                                e_ptr->sectors[0]->z < z &&
                                e_ptr->sectors[1]->z < z
                            ) {
                                //An edge whose sectors are below the mob?
                                //No collision here.
                                continue;
                            }
                            if(e_ptr->sectors[0]->z == e_ptr->sectors[1]->z) {
                                //No difference in floor height = no wall.
                                //Ignore this.
                                continue;
                            }
                        }
                        
                        sector* tallest_sector; //Tallest of the two.
                        if(
                            e_ptr->sectors[0]->type == SECTOR_TYPE_BLOCKING
                        ) {
                            tallest_sector = e_ptr->sectors[1];
                            
                        } else if(
                            e_ptr->sectors[1]->type == SECTOR_TYPE_BLOCKING
                        ) {
                            tallest_sector = e_ptr->sectors[0];
                            
                        } else {
                            if(e_ptr->sectors[0]->z > e_ptr->sectors[1]->z) {
                                tallest_sector = e_ptr->sectors[0];
                            } else {
                                tallest_sector = e_ptr->sectors[1];
                            }
                        }
                        
                        if(
                            tallest_sector->z > new_ground_sector->z &&
                            tallest_sector->z <= z
                        ) {
                            new_ground_sector = tallest_sector;
                        }
                        
                        //Check if it can go up this step.
                        //It can go up this step if the floor is within
                        //stepping distance of the mob's current Z,
                        //and if this step is larger than any step
                        //encountered of all edges crossed.
                        if(
                            !was_thrown &&
                            tallest_sector->z <= z + SECTOR_STEP &&
                            tallest_sector->z > step_sector->z
                        ) {
                            step_sector = tallest_sector;
                        }
                        
                        //Add this edge to the list of intersections, then.
                        intersecting_edges.insert(e_ptr);
                        
                    } else {
                    
                        //If we're on the edge of out-of-bounds geometry,
                        //block entirely.
                        successful_move = false;
                        break;
                        
                    }
                    
                }
                
                if(!successful_move) break;
            }
            
            if(!successful_move) break;
        }
        
        if(!successful_move) break;
        
        //Check also if it can walk on top of another mob.
        standing_on_mob = NULL;
        
        for(size_t m = 0; m < mobs.size(); ++m) {
            mob* m_ptr = mobs[m];
            if(!m_ptr->type->walkable) {
                continue;
            }
            if(m_ptr == this) {
                continue;
            }
            if(z < m_ptr->z + m_ptr->type->height - SECTOR_STEP) {
                continue;
            }
            if(z > m_ptr->z + m_ptr->type->height) {
                continue;
            }
            
            //Check if they collide on X+Y.
            if(
                type->rectangular_dim.x != 0 &&
                m_ptr->type->rectangular_dim.x != 0
            ) {
                //Rectangle vs rectangle.
                //Not supported.
                continue;
            } else if(type->rectangular_dim.x != 0) {
                //Rectangle vs circle.
                if(
                    !circle_intersects_rectangle(
                        m_ptr->pos, m_ptr->type->radius,
                        new_pos, type->rectangular_dim,
                        angle
                    )
                ) {
                    continue;
                }
            } else if(m_ptr->type->rectangular_dim.x != 0) {
                //Circle vs rectangle.
                if(
                    !circle_intersects_rectangle(
                        new_pos, type->radius,
                        m_ptr->pos, m_ptr->type->rectangular_dim,
                        m_ptr->angle
                    )
                ) {
                    continue;
                }
            } else {
                //Circle vs circle.
                if(
                    dist(new_pos, m_ptr->pos) >
                    (type->radius + m_ptr->type->radius)
                ) {
                    continue;
                }
            }
            
            standing_on_mob = m_ptr;
        }
        
        if(standing_on_mob) {
            new_z = standing_on_mob->z + standing_on_mob->type->height;
        } else {
            if(step_sector->z > new_ground_sector->z) {
                new_ground_sector = step_sector;
            }
            
            if(z < step_sector->z) new_z = step_sector->z;
        }
        
        
        //Check wall angles and heights to check which of these edges
        //really are wall collisions.
        for(
            auto e = intersecting_edges.begin();
            e != intersecting_edges.end(); e++
        ) {
        
            e_ptr = *e;
            bool is_edge_wall = false;
            unsigned char wall_sector = 0;
            
            for(unsigned char s = 0; s < 2; s++) {
                if(e_ptr->sectors[s]->type == SECTOR_TYPE_BLOCKING) {
                    is_edge_wall = true;
                    wall_sector = s;
                }
            }
            
            if(!is_edge_wall) {
                for(unsigned char s = 0; s < 2; s++) {
                    if(e_ptr->sectors[s]->z > new_z) {
                        is_edge_wall = true;
                        wall_sector = s;
                    }
                }
            }
            
            //This isn't a wall... Get out of here, faker.
            if(!is_edge_wall) continue;
            
            //If both floors of this edge are above the mob...
            //then what does that mean? That the mob is under the ground?
            //Nonsense! Throw this edge away!
            //It's a false positive, and the only
            //way for it to get caught is if it's behind a more logical
            //edge that we actually did collide against.
            if(e_ptr->sectors[0] && e_ptr->sectors[1]) {
                if(
                    (
                        e_ptr->sectors[0]->z > new_z ||
                        e_ptr->sectors[0]->type == SECTOR_TYPE_BLOCKING
                    ) &&
                    (
                        e_ptr->sectors[1]->z > new_z ||
                        e_ptr->sectors[1]->type == SECTOR_TYPE_BLOCKING
                    )
                ) {
                    continue;
                }
            }
            
            //Ok, there's obviously been a collision, so let's work out what
            //wall the mob will slide on.
            
            //The wall's normal is the direction the wall is facing.
            //i.e. the direction from the top floor to the bottom floor.
            //We know which side of an edge is which sector because of
            //the vertexes. Imagine you're in first person view,
            //following the edge as a line on the ground.
            //You start on vertex 0 and face vertex 1.
            //Sector 0 will always be on your left.
            if(!doing_slide) {
            
                float wall_normal;
                float wall_angle =
                    get_angle(
                        point(e_ptr->vertexes[0]->x, e_ptr->vertexes[0]->y),
                        point(e_ptr->vertexes[1]->x, e_ptr->vertexes[1]->y)
                    );
                    
                if(wall_sector == 0) {
                    wall_normal = normalize_angle(wall_angle + TAU / 4);
                } else {
                    wall_normal = normalize_angle(wall_angle - TAU / 4);
                }
                
                float nd = get_angle_cw_dif(wall_normal, move_angle);
                if(nd < TAU * 0.25 || nd > TAU * 0.75) {
                    //If the difference between the movement and the wall's
                    //normal is this, that means we came FROM the wall.
                    //No way! There has to be an edge that makes more sense.
                    continue;
                }
                
                //If we were to slide on this edge, this would be
                //the slide angle.
                float tentative_slide_angle;
                if(nd < TAU / 2) {
                    //Coming in from the "left" of the normal. Slide right.
                    tentative_slide_angle = wall_normal + TAU / 4;
                } else {
                    //Coming in from the "right" of the normal. Slide left.
                    tentative_slide_angle = wall_normal - TAU / 4;
                }
                
                float sd =
                    get_angle_smallest_dif(move_angle, tentative_slide_angle);
                if(sd > slide_angle_dif) {
                    slide_angle_dif = sd;
                    slide_angle = tentative_slide_angle;
                }
                
            }
            
            //By the way, if we got to this point, that means there are real
            //collisions happening. Let's mark this move as unsuccessful.
            successful_move = false;
            touched_wall = true;
        }
        
        //If the mob is just slamming against the wall head-on, perpendicularly,
        //then forget any idea about sliding.
        //It'd just be awkwardly walking in place.
        if(!successful_move && slide_angle_dif > TAU / 4 - 0.05) {
            doing_slide = true;
        }
        
        
        //We're done here. If the move was unobstructed, good, go there.
        //If not, we'll use the info we gathered before to calculate sliding,
        //and try again.
        
        if(successful_move) {
            //Good news, the mob can move to this new spot freely.
            pos = new_pos;
            z = new_z;
            ground_sector = new_ground_sector;
            center_sector = new_center_sector;
            finished_moving = true;
            
        } else {
        
            //Try sliding.
            if(doing_slide) {
                //We already tried sliding, and we still hit something...
                //Let's just stop completely. This mob can't go forward.
                finished_moving = true;
                
            } else {
            
                doing_slide = true;
                //To limit the speed, we should use a cross-product of the
                //movement and slide vectors.
                //But nuts to that, this is just as nice, and a lot simpler!
                total_move_speed *= 1 - (slide_angle_dif / TAU / 2);
                move_speed =
                    angle_to_coordinates(
                        slide_angle, total_move_speed
                    );
                    
            }
            
        }
        
    }
    
    if(touched_wall) {
        fsm.run_event(MOB_EVENT_TOUCHED_WALL);
    }
    
    
    //Vertical movement.
    
    if(!standing_on_mob) {
        //If the current ground is one step (or less) below
        //the previous ground, just instantly go down the step.
        if(
            pre_move_ground_z - ground_sector->z <= SECTOR_STEP &&
            z == pre_move_ground_z
        ) {
            z = ground_sector->z;
        }
    }
    
    //Gravity.
    speed_z += delta_t* gravity_mult * GRAVITY_ADDER;
    
    //Actual movement.
    z += delta_t* speed_z;
    
    //Landing.
    hazard* new_on_hazard = NULL;
    if(speed_z <= 0) {
        if(standing_on_mob) {
            z = standing_on_mob->z + standing_on_mob->type->height;
            speed_z = 0;
            was_thrown = false;
            fsm.run_event(MOB_EVENT_LANDED);
        } else if(z <= ground_sector->z) {
            z = ground_sector->z;
            speed_z = 0;
            was_thrown = false;
            fsm.run_event(MOB_EVENT_LANDED);
            stop_height_effect();
            
            if(ground_sector->is_bottomless_pit) {
                fsm.run_event(MOB_EVENT_BOTTOMLESS_PIT);
            }
            
            for(size_t h = 0; h < ground_sector->hazards.size(); ++h) {
                fsm.run_event(
                    MOB_EVENT_TOUCHED_HAZARD,
                    (void*) ground_sector->hazards[h]
                );
                new_on_hazard = ground_sector->hazards[h];
            }
        }
    }
    
    //Due to framerate imperfections, thrown Pikmin/leaders can reach higher
    //than intended. z_cap forces a cap. FLT_MAX = no cap.
    if(speed_z <= 0) {
        z_cap = FLT_MAX;
    } else if(z_cap < FLT_MAX) {
        z = min(z, z_cap);
    }
    
    //On a sector that has a hazard that is not on the floor.
    if(z > ground_sector->z && !ground_sector->hazard_floor) {
        for(size_t h = 0; h < ground_sector->hazards.size(); ++h) {
            fsm.run_event(
                MOB_EVENT_TOUCHED_HAZARD,
                (void*) ground_sector->hazards[h]
            );
            new_on_hazard = ground_sector->hazards[h];
        }
    }
    
    if(new_on_hazard != on_hazard && on_hazard != NULL) {
        fsm.run_event(
            MOB_EVENT_LEFT_HAZARD,
            (void*) on_hazard
        );
    }
    on_hazard = new_on_hazard;
    
    //Quick panic check: if it's somehow inside the ground, pop it out.
    z = max(z, ground_sector->z);
}


/* ----------------------------------------------------------------------------
 * Checks general events in the mob's script for this frame.
 */
void mob::tick_script() {
    if(!fsm.cur_state) return;
    
    //Timer events.
    mob_event* timer_ev = q_get_event(this, MOB_EVENT_TIMER);
    if(script_timer.duration > 0) {
        if(script_timer.time_left > 0) {
            script_timer.tick(delta_t);
            if(script_timer.time_left == 0.0f && timer_ev) {
                timer_ev->run(this);
            }
        }
    }
    
    //Has it reached its home?
    mob_event* reach_dest_ev = q_get_event(this, MOB_EVENT_REACHED_DESTINATION);
    if(reach_dest_ev && reached_destination) {
        reach_dest_ev->run(this);
    }
    
    //Is it dead?
    if(health <= 0 && type->max_health != 0) {
        fsm.run_event(MOB_EVENT_DEATH, this);
    }
    
    //Check the focused mob.
    if(focused_mob) {
    
        if(focused_mob->health <= 0) {
            fsm.run_event(MOB_EVENT_FOCUS_DIED);
            fsm.run_event(MOB_EVENT_FOCUS_OFF_REACH);
        }
        
        //We have to recheck if the focused mob is not NULL, because
        //sending MOB_EVENT_FOCUS_DIED could've set this to NULL.
        if(focused_mob) {
        
            mob* focus = focused_mob;
            
            mob_event* for_ev =
                q_get_event(this, MOB_EVENT_FOCUS_OFF_REACH);
                
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
        
        if(focused_mob) {
            if(!focused_mob->carry_info) {
                fsm.run_event(MOB_EVENT_FOCUSED_MOB_UNCARRIABLE);
            }
        }
        
    }
    
    //Itch event.
    if(type->itch_damage > 0 || type->itch_time > 0) {
        itch_time += delta_t;
        mob_event* itch_ev = q_get_event(this, MOB_EVENT_ITCH);
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
    mob_event* whistled_ev = q_get_event(this, MOB_EVENT_WHISTLED);
    if(whistling && whistled_ev) {
        if(dist(pos, leader_cursor_w) <= whistle_radius) {
            whistled_ev->run(this);
        }
    }
    
    //Following a leader.
    if(following_group) {
        mob_event* spot_near_ev = q_get_event(this, MOB_EVENT_SPOT_IS_NEAR);
        mob_event* spot_far_ev =  q_get_event(this, MOB_EVENT_SPOT_IS_FAR);
        
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
    mob_event* far_from_home_ev = q_get_event(this, MOB_EVENT_FAR_FROM_HOME);
    if(far_from_home_ev) {
        dist d(pos, home);
        if(d >= type->territory_radius) {
            far_from_home_ev->run(this);
        }
    }
    
    //Following a path, and an obstacle was destroyed.
    if(path_info) {
        for(
            auto o = path_info->obstacle_ptrs.begin();
            o != path_info->obstacle_ptrs.end();
            ++o
        ) {
            if((*o)->health == 0) {
                follow_path(
                    path_info->target_point,
                    true, chase_speed,
                    path_info->final_target_distance
                );
                break;
            }
        }
    }
    
    //Being carried, is stuck, and an obstacle was destroyed.
    if(carry_info && carry_info->is_stuck) {
        for(
            auto o = carry_info->obstacle_ptrs.begin();
            o != carry_info->obstacle_ptrs.end();
            ++o
        ) {
            if((*o)->health == 0) {
                fsm.run_event(MOB_EVENT_CARRY_BEGIN_MOVE);
                break;
            }
        }
    }
    
    //Tick event.
    fsm.run_event(MOB_EVENT_ON_TICK);
}


/* ----------------------------------------------------------------------------
 * Makes the mob lose focus on its currently focused mob.
 */
void mob::unfocus_from_mob() {
    focused_mob = nullptr;
}


/* ----------------------------------------------------------------------------
 * Does this mob want to attack mob v? Teams and other factors are used to
 * decide this.
 */
bool mob::wants_to_attack(mob* v) {
    if(v->team == MOB_TEAM_TOOL) {
        return false;
    }
    return can_damage(v);
}


bool mob::can_receive_status(status_type* s) {
    return s->affects & STATUS_AFFECTS_OTHERS;
}
void mob::handle_status_effect(status_type* s) {}
void mob::lose_panic_from_status() {}
void mob::start_dying_class_specific() { }


/* ----------------------------------------------------------------------------
 * Initializes the members of a mob with anim groups.
 */
mob_with_anim_groups::mob_with_anim_groups() :
    cur_base_anim_nr(INVALID) {
    
}


/* ----------------------------------------------------------------------------
 * Returns the number of an animation, given a base animation number and
 * group number.
 */
size_t mob_with_anim_groups::get_animation_nr_from_base_and_group(
    const size_t base_anim_nr, const size_t group_nr,
    const size_t base_anim_total
) {
    return group_nr * base_anim_total + base_anim_nr;
}
