/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pikmin class and Pikmin-related functions.
 */

#include <iostream>

#include "functions.h"
#include "mob.h"
#include "pikmin.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Creates a Pikmin.
 */
pikmin::pikmin(const float x, const float y, pikmin_type* type, const float angle, const string &vars) :
    mob(x, y, type, angle, vars),
    pik_type(type),
    hazard_time_left(-1),
    attack_time(0),
    being_chomped(false),
    pluck_reserved(false),
    carrying_spot(0),
    grabbing_carriable_mob(false),
    maturity(s2i(get_var_value(vars, "maturity", "2"))),
    enemy_hitbox_nr(0),
    enemy_hitbox_dist(0),
    enemy_hitbox_angle(0),
    is_idle(true) {
    
    team = MOB_TEAM_PLAYER_1; // TODO
    if(s2b(get_var_value(vars, "buried", "0"))) {
        fsm.set_state(PIKMIN_STATE_BURIED);
        this->set_first_state = true;
    }
}


pikmin::~pikmin() { }


/* ----------------------------------------------------------------------------
 * Returns a Pikmin's base speed, without status effects and the like.
 * This depends on the maturity.
 */
float pikmin::get_base_speed() {
    return pik_type->move_speed + pik_type->move_speed * this->maturity * MATURITY_SPEED_MULT;
}


/* ----------------------------------------------------------------------------
 * Returns the buried Pikmin closest to a leader. Used when auto-plucking.
 * x/y:             Coordinates of the leader.
 * d:               Variable to return the distance to. NULL for none.
 * ignore_reserved: If true, ignore any buried Pikmin that are "reserved"
   * (i.e. already chosen to be plucked by another leader).
 */
pikmin* get_closest_buried_pikmin(const float x, const float y, dist* d, const bool ignore_reserved) {
    dist closest_distance = 0;
    pikmin* closest_pikmin = NULL;
    
    size_t n_pikmin = pikmin_list.size();
    for(size_t p = 0; p < n_pikmin; p++) {
        if(pikmin_list[p]->fsm.cur_state->id != PIKMIN_STATE_BURIED) continue;
        
        dist dis(x, y, pikmin_list[p]->x, pikmin_list[p]->y);
        if(closest_pikmin == NULL || dis < closest_distance) {
        
            if(!(ignore_reserved || pikmin_list[p]->pluck_reserved)) {
                closest_distance = dis;
                closest_pikmin = pikmin_list[p];
            }
        }
    }
    
    if(d) *d = closest_distance;
    return closest_pikmin;
}


/* ----------------------------------------------------------------------------
 * Gives an Onion some Pikmin, and makes the Onion spew seeds out,
 ** depending on how many Pikmin there are in the field (don't spew if 100).
 */
void give_pikmin_to_onion(onion* o, const unsigned amount) {
    unsigned total_after = pikmin_list.size() + amount;
    unsigned pikmin_to_spit = amount;
    unsigned pikmin_to_keep = 0; //Pikmin to keep inside the Onion, without spitting.
    
    if(total_after > max_pikmin_in_field) {
        pikmin_to_keep = total_after - max_pikmin_in_field;
        pikmin_to_spit = amount - pikmin_to_keep;
    }
    
    for(unsigned p = 0; p < pikmin_to_spit; p++) {
        float angle = randomf(0, M_PI * 2);
        float sx = cos(angle) * 60;
        float sy = sin(angle) * 60;
        
        pikmin* new_pikmin = new pikmin(o->x, o->y, o->oni_type->pik_type, 0, "");
        new_pikmin->set_state(PIKMIN_STATE_BURIED);
        new_pikmin->z = 320;
        new_pikmin->speed_z = 200;
        new_pikmin->speed_x = sx;
        new_pikmin->speed_y = sy;
        create_mob(new_pikmin);
    }
    
    for(unsigned p = 0; p < pikmin_to_keep; p++) {
        pikmin_in_onions[o->oni_type->pik_type]++;
    }
}


/* ----------------------------------------------------------------------------
 * Makes a mob move to a spot because it's being carried.
 * m:  Mob to start moving (the treasure, for instance).
 * np: New Pikmin; the Pikmin that justed joined the carriers. Used to detect ties and tie-breaking.
 * lp: Leaving Pikmin; the Pikmin that just left the carriers. Used to detect ties and tie-breaking.
 */
void start_moving_carried_object(mob* m, pikmin* np, pikmin* lp) {
    //TODO what if an Onion hasn't been revelead yet?
    if(!m->carrier_info) return;
    
    if(m->carrier_info->carry_to_ship) {
    
        m->set_target(
            ships[0]->x + ships[0]->type->radius + m->type->radius + 8,
            ships[0]->y,
            NULL,
            NULL,
            false);
        m->carrier_info->decided_type = NULL;
        
    } else {
    
        map<pikmin_type*, unsigned> type_quantity; //How many of each Pikmin type are carrying.
        vector<pikmin_type*> majority_types; //The Pikmin type with the most carriers.
        
        //First, count how many of each type there are carrying.
        for(size_t p = 0; p < m->carrier_info->max_carriers; p++) {
            pikmin* pik_ptr = NULL;
            
            if(m->carrier_info->carrier_spots[p] == NULL) continue;
            if(typeid(*m->carrier_info->carrier_spots[p]) != typeid(pikmin)) continue;
            
            pik_ptr = (pikmin*) m->carrier_info->carrier_spots[p];
            
            if(!pik_ptr->pik_type->has_onion) continue; //If it doesn't have an Onion, it won't even count. //TODO what if it hasn't been discovered / Onion not on this area?
            
            type_quantity[pik_ptr->pik_type]++;
        }
        
        //Then figure out what are the majority types.
        unsigned most = 0;
        for(auto t = type_quantity.begin(); t != type_quantity.end(); t++) {
            if(t->second > most) {
                most = t->second;
                majority_types.clear();
            }
            if(t->second == most) majority_types.push_back(t->first);
        }
        
        //If we ended up with no candidates, pick a type at random, out of all possible types.
        if(majority_types.empty()) {
            for(auto t = pikmin_types.begin(); t != pikmin_types.end(); t++) {
                if(t->second->has_onion) { //TODO what if it hasn't been discovered / Onion not on this area?
                    majority_types.push_back(t->second);
                }
            }
        }
        
        //Now let's pick an Onion.
        if(majority_types.empty()) {
            return; //TODO warn that something went horribly wrong?
            
        } else if(majority_types.size() == 1) {
            //If there's only one possible type to pick, pick it.
            m->carrier_info->decided_type = majority_types[0];
            
        } else {
            //If there's a tie, let's take a careful look.
            bool new_tie = false;
            
            //Is the Pikmin that just joined part of the majority types?
            //If so, that means this Pikmin just created a NEW tie!
            //So let's pick a random Onion again.
            if(np) {
                for(size_t mt = 0; mt < majority_types.size(); mt++) {
                    if(np->type == majority_types[mt]) {
                        new_tie = true;
                        break;
                    }
                }
            }
            
            //If a Pikmin left, check if it is related to the majority types.
            //If not, then a new tie wasn't made, no worries.
            //If it was related, a new tie was created.
            if(lp) {
                new_tie = false;
                for(size_t mt = 0; mt < majority_types.size(); mt++) {
                    if(lp->type == majority_types[mt]) {
                        new_tie = true;
                        break;
                    }
                }
            }
            
            //Check if the previously decided type belongs to one of the majorities.
            //If so, it can be chosen again, but if not, it cannot.
            bool can_continue = false;
            for(size_t mt = 0; mt < majority_types.size(); mt++) {
                if(majority_types[mt] == m->carrier_info->decided_type) {
                    can_continue = true;
                    break;
                }
            }
            if(!can_continue) m->carrier_info->decided_type = NULL;
            
            //If the Pikmin that just joined is not a part of the majorities,
            //then it had no impact on the existing ties.
            //Go with the Onion that had been decided before.
            if(new_tie || !m->carrier_info->decided_type) {
                m->carrier_info->decided_type = majority_types[randomi(0, majority_types.size() - 1)];
            }
        }
        
        
        //Figure out where that type's Onion is.
        size_t onion_nr = 0;
        for(; onion_nr < onions.size(); onion_nr++) {
            if(onions[onion_nr]->oni_type->pik_type == m->carrier_info->decided_type) {
                break;
            }
        }
        
        //Finally, start moving the mob.
        m->set_target(onions[onion_nr]->x, onions[onion_nr]->y, NULL, NULL, false);
        m->set_state(MOB_STATE_BEING_CARRIED);
        sfx_pikmin_carrying.play(-1, true);
    }
}

void pikmin::become_buried(mob* m, void* info1, void* info2) {
    m->anim.change(PIKMIN_ANIM_BURROWED, true, false, false);
}

void pikmin::begin_pluck(mob* m, void* info1, void* info2) {
    pikmin* pik = (pikmin*) m;
    mob* lea = (mob*) info1;
    
    if(lea->following_party) {
        if(typeid(*lea->following_party) == typeid(leader)) {
            //If this leader is following another one, then the new Pikmin should be in the party of that top leader.
            lea = lea->following_party;
        }
    }
    
    pik->anim.change(PIKMIN_ANIM_PLUCKING, true, false, false);
    add_to_party(lea, pik);
}

void pikmin::end_pluck(mob* m, void* info1, void* info2) {
    pikmin* pik = (pikmin*) m;
    pik->anim.change(PIKMIN_ANIM_IDLE, true, false, false);
    sfx_pikmin_plucked.play(0, false);
    sfx_pikmin_pluck.play(0, false);
}

void pikmin::be_grabbed_by_friend(mob* m, void* info1, void* info2) {
    sfx_pikmin_held.play(0, false);
    m->anim.change(PIKMIN_ANIM_IDLE, true, false, false);
}

void pikmin::be_grabbed_by_enemy(mob* m, void* info1, void* info2) {
    //TODO merge this code with the latch-on code.
    pikmin* pik_ptr = (pikmin*) m;
    mob* mob_ptr = (mob*) info1;
    hitbox_instance* hi_ptr = (hitbox_instance*) info2;
    
    if(!hi_ptr) return;
    
    pik_ptr->enemy_hitbox_nr = hi_ptr->hitbox_nr;
    pik_ptr->speed_x = pik_ptr->speed_y = pik_ptr->speed_z = 0;
    
    float actual_hx, actual_hy;
    rotate_point(hi_ptr->x, hi_ptr->y, mob_ptr->angle, &actual_hx, &actual_hy);
    actual_hx += mob_ptr->x; actual_hy += mob_ptr->y;
    
    float x_dif = pik_ptr->x - actual_hx;
    float y_dif = pik_ptr->y - actual_hy;
    coordinates_to_angle(x_dif, y_dif, &pik_ptr->enemy_hitbox_angle, &pik_ptr->enemy_hitbox_dist);
    pik_ptr->enemy_hitbox_angle -= mob_ptr->angle; //Relative to 0 degrees.
    pik_ptr->enemy_hitbox_dist /= hi_ptr->radius; //Distance in units to distance in percentage.
    
    pik_ptr->focused_mob = mob_ptr;
    
    sfx_pikmin_caught.play(0.2, 0);
    pik_ptr->anim.change(PIKMIN_ANIM_IDLE, true, false, false);
    remove_from_party(pik_ptr);
}

void pikmin::be_dismissed(mob* m, void* info1, void* info2) {
    float angle = *((float*) info1);
    
    m->set_target(
        cur_leader_ptr->x + cos(angle) * DISMISS_DISTANCE,
        cur_leader_ptr->y + sin(angle) * DISMISS_DISTANCE,
        NULL,
        NULL,
        false
    );
    
    m->anim.change(PIKMIN_ANIM_IDLE, true, false, false);
}

void pikmin::reach_dismiss_spot(mob* m, void* info1, void* info2) {
    m->remove_target(true);
    m->anim.change(PIKMIN_ANIM_IDLE, true, false, false);
}

void pikmin::become_idle(mob* m, void* info1, void* info2) {
    m->anim.change(PIKMIN_ANIM_IDLE, true, false, false);
    unfocus_mob(m);
    ((pikmin*) m)->is_idle = true;
}

void pikmin::be_thrown(mob* m, void* info1, void* info2) {
    sfx_pikmin_held.stop();
    sfx_pikmin_thrown.stop();
    sfx_pikmin_thrown.play(0, false);
    m->anim.change(PIKMIN_ANIM_THROWN, true, false, false);
}

void pikmin::be_released(mob* m, void* info1, void* info2) {
    cur_leader_ptr->holding_pikmin = NULL;
}

void pikmin::land(mob* m, void* info1, void* info2) {
    m->anim.change(PIKMIN_ANIM_IDLE, true, false, false);
}

void pikmin::go_to_task(mob* m, void* info1, void* info2) {

}

void pikmin::called(mob* m, void* info1, void* info2) {
    pikmin* pik = (pikmin*) m;
    
    pik->attack_time = 0;
    add_to_party(cur_leader_ptr, pik);
    sfx_pikmin_called.play(0.03, false);
}

void pikmin::get_knocked_down(mob* m, void* info1, void* info2) {
    mob* m2 = (mob*) info1;
    hitbox_instance* hi = (hitbox_instance*) info2;
    
    float angle = m2->angle;
    if(hi->knockback_outward) {
        angle += atan2(m->y - m2->y, m->x - m2->x);
    } else {
        angle += hi->knockback_angle;
    }
    
    mob::attack((mob*) info1, m, false, 0, angle, hi->knockback, 0, 0);
    m->anim.change(PIKMIN_ANIM_LYING, true, false, false);
    
    remove_from_party(m);
}

void pikmin::go_to_opponent(mob* m, void* info1, void* info2) {
    focus_mob(m, (mob*) info1);
    m->set_target(
        0, 0,
        &m->focused_mob->x, &m->focused_mob->y,
        false, nullptr, false,
        m->focused_mob->type->radius + m->type->radius
    );
    m->anim.change(PIKMIN_ANIM_WALK, true, false, false);
    remove_from_party(m);
}

void pikmin::rechase_opponent(mob* m, void* info1, void* info2) {
    if(
        dist(m->x, m->y, m->focused_mob->x, m->focused_mob->y) <=
        (m->focused_mob->type->radius + m->type->radius + 2)
    ) {
        return;
    }
    
    m->fsm.set_state(PIKMIN_STATE_IDLE);
}

void pikmin::prepare_to_attack(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    p->anim.change(PIKMIN_ANIM_ATTACK, true, false, false);
    ((pikmin*) p)->attack_time = p->pik_type->attack_interval;
    p->was_thrown = false;
}

void pikmin::land_on_mob(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    mob* mob_ptr = (mob*) info1;
    hitbox_instance* hi_ptr = (hitbox_instance*) info2;
    
    if(!hi_ptr) return;
    
    pik_ptr->enemy_hitbox_nr = hi_ptr->hitbox_nr;
    pik_ptr->speed_x = pik_ptr->speed_y = pik_ptr->speed_z = 0;
    
    float actual_hx, actual_hy;
    rotate_point(hi_ptr->x, hi_ptr->y, mob_ptr->angle, &actual_hx, &actual_hy);
    actual_hx += mob_ptr->x; actual_hy += mob_ptr->y;
    
    float x_dif = pik_ptr->x - actual_hx;
    float y_dif = pik_ptr->y - actual_hy;
    coordinates_to_angle(x_dif, y_dif, &pik_ptr->enemy_hitbox_angle, &pik_ptr->enemy_hitbox_dist);
    pik_ptr->enemy_hitbox_angle -= mob_ptr->angle; //Relative to 0 degrees.
    pik_ptr->enemy_hitbox_dist /= hi_ptr->radius; //Distance in units to distance in percentage.
    
    pik_ptr->focused_mob = mob_ptr;
    
}

void pikmin::tick_grabbed_by_enemy(mob* m, void* info1, void* info2) {
    //TODO merge this code with the one on tick_latched.
    pikmin* pik_ptr = (pikmin*) m;
    if(!pik_ptr->focused_mob) return;
    
    hitbox_instance* h_ptr = get_hitbox_instance(pik_ptr->focused_mob, pik_ptr->enemy_hitbox_nr);
    if(h_ptr) {
        float actual_hx, actual_hy;
        rotate_point(h_ptr->x, h_ptr->y, pik_ptr->focused_mob->angle, &actual_hx, &actual_hy);
        actual_hx += pik_ptr->focused_mob->x; actual_hy += pik_ptr->focused_mob->y;
        
        float final_px, final_py;
        angle_to_coordinates(
            pik_ptr->enemy_hitbox_angle + pik_ptr->focused_mob->angle,
            pik_ptr->enemy_hitbox_dist * h_ptr->radius,
            &final_px, &final_py);
        final_px += actual_hx; final_py += actual_hy;
        
        pik_ptr->set_target(final_px, final_py, NULL, NULL, true);
        pik_ptr->face(atan2(pik_ptr->focused_mob->y - pik_ptr->y, pik_ptr->focused_mob->x - pik_ptr->x));
        if(pik_ptr->attack_time == 0) pik_ptr->attack_time = pik_ptr->pik_type->attack_interval;
        
    }
}

void pikmin::tick_latched(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    if(!pik_ptr->focused_mob) return;
    
    hitbox_instance* h_ptr = get_hitbox_instance(pik_ptr->focused_mob, pik_ptr->enemy_hitbox_nr);
    if(h_ptr) {
        float actual_hx, actual_hy;
        rotate_point(h_ptr->x, h_ptr->y, pik_ptr->focused_mob->angle, &actual_hx, &actual_hy);
        actual_hx += pik_ptr->focused_mob->x; actual_hy += pik_ptr->focused_mob->y;
        
        float final_px, final_py;
        angle_to_coordinates(
            pik_ptr->enemy_hitbox_angle + pik_ptr->focused_mob->angle,
            pik_ptr->enemy_hitbox_dist * h_ptr->radius,
            &final_px, &final_py);
        final_px += actual_hx; final_py += actual_hy;
        
        pik_ptr->set_target(final_px, final_py, NULL, NULL, true);
        pik_ptr->face(atan2(pik_ptr->focused_mob->y - pik_ptr->y, pik_ptr->focused_mob->x - pik_ptr->x));
        if(pik_ptr->attack_time == 0) pik_ptr->attack_time = pik_ptr->pik_type->attack_interval;
        
    }
    
    pik_ptr->attack_time -= delta_t;
    
    //TODO damage caused should depend on hitbox.
    //TODO merge this code and the one on tick_attacking_grounded in a single function.
    if(pik_ptr->attack_time <= 0) {
        pik_ptr->attack_time = pik_ptr->pik_type->attack_interval;
        mob::attack(pik_ptr, pik_ptr->focused_mob, true, pik_ptr->pik_type->attack_power, 0, 0, 0, 0);
        sfx_attack.play(0.06f, false, 0.4f);
        sfx_pikmin_attack.play(0.06f, false, 0.8f);
        particles.push_back(
            particle(
                PARTICLE_TYPE_SMACK, bmp_smack,
                pik_ptr->x, pik_ptr->y,
                0, 0, 0, 0,
                SMACK_PARTICLE_DUR,
                64,
                al_map_rgb(255, 160, 128)
            )
        );
    }
}

void pikmin::tick_attacking_grounded(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    pik_ptr->attack_time -= delta_t;
    
    //TODO damage caused should depend on hitbox.
    //TODO merge this code and the one on tick_latched in a single function.
    if(pik_ptr->attack_time <= 0) {
        pik_ptr->attack_time = pik_ptr->pik_type->attack_interval;
        mob::attack(pik_ptr, pik_ptr->focused_mob, true, pik_ptr->pik_type->attack_power, 0, 0, 0, 0);
        sfx_attack.play(0.06, false, 0.4f);
        sfx_pikmin_attack.play(0.06, false, 0.8f);
        particles.push_back(
            particle(
                PARTICLE_TYPE_SMACK, bmp_smack,
                pik_ptr->x, pik_ptr->y,
                0, 0, 0, 0,
                SMACK_PARTICLE_DUR,
                64,
                al_map_rgb(255, 160, 128)
            )
        );
    }
    
    pik_ptr->face(atan2(pik_ptr->focused_mob->y - pik_ptr->y, pik_ptr->focused_mob->x - pik_ptr->x));
}

void pikmin::work_on_task(mob* m, void* info1, void* info2) {

}

void pikmin::finish_carrying(mob* m, void* info1, void* info2) {

}

void pikmin::chase_leader(mob* m, void* info1, void* info2) {
    m->set_target(0, 0, &m->following_party->x, &m->following_party->y, false);
    m->anim.change(PIKMIN_ANIM_WALK, true, false, false);
    focus_mob(m, m->following_party);
}

void pikmin::stop_being_idle(mob* m, void* info1, void* info2) {
    ((pikmin*) m)->is_idle = false;
}

void pikmin::stop_in_group(mob* m, void* info1, void* info2) {
    m->remove_target(true);
    m->anim.change(PIKMIN_ANIM_IDLE, true, false, false);
}

void pikmin::go_to_carriable_object(mob* m, void* info1, void* info2) {
    mob* carriable_mob_ptr = (mob*) info1;
    pikmin* pik_ptr = (pikmin*) m;
    
    focus_mob(pik_ptr, carriable_mob_ptr);
    pik_ptr->grabbing_carriable_mob = false;
    remove_from_party(pik_ptr);
    
    //TODO remove this random cycle and replace with something more optimal.
    bool valid_spot = false;
    unsigned int spot = 0;
    while(!valid_spot) {
        spot = randomi(0, carriable_mob_ptr->carrier_info->max_carriers - 1);
        valid_spot = !carriable_mob_ptr->carrier_info->carrier_spots[spot];
    }
    
    pik_ptr->set_target(
        carriable_mob_ptr->carrier_info->carrier_spots_x[spot],
        carriable_mob_ptr->carrier_info->carrier_spots_y[spot],
        &carriable_mob_ptr->x,
        &carriable_mob_ptr->y,
        false
    );
    
    carriable_mob_ptr->carrier_info->carrier_spots[spot] = pik_ptr;
    carriable_mob_ptr->carrier_info->current_n_carriers++;
    
    pik_ptr->carrying_spot = spot;
    
}

void pikmin::grab_carriable_object(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) m;
    
    pik_ptr->grabbing_carriable_mob = true;
    
    pik_ptr->set_target(
        pik_ptr->focused_mob->carrier_info->carrier_spots_x[pik_ptr->carrying_spot],
        pik_ptr->focused_mob->carrier_info->carrier_spots_y[pik_ptr->carrying_spot],
        &pik_ptr->focused_mob->x,
        &pik_ptr->focused_mob->y,
        true, &pik_ptr->focused_mob->z
    );
    pik_ptr->face(atan2(pik_ptr->focused_mob->y - pik_ptr->y, pik_ptr->focused_mob->x - pik_ptr->x));
    
    pik_ptr->focused_mob->carrier_info->current_carrying_strength += pik_ptr->pik_type->carry_strength;
    
    //Enough strength to carry it? Do so!
    if(pik_ptr->focused_mob->carrier_info->current_carrying_strength >= pik_ptr->focused_mob->type->weight) {
        start_moving_carried_object(pik_ptr->focused_mob, pik_ptr, NULL);
    }
    
    pik_ptr->unwhistlable_period = 0;
    sfx_pikmin_carrying_grab.play(0.03, false);
}

void pikmin::forget_about_carrying(mob* m, void* info1, void* info2) {
    pikmin* p = (pikmin*) m;
    if(!p->focused_mob) return;
    
    if(p->focused_mob->carrier_info) {
        //TODO optimize this instead of running through the spot vector.
        for(size_t s = 0; s < p->focused_mob->carrier_info->max_carriers; s++) {
            if(p->focused_mob->carrier_info->carrier_spots[s] == p) {
                p->focused_mob->carrier_info->carrier_spots[s] = nullptr;
                break;
            }
        }
        p->focused_mob->carrier_info->current_n_carriers--;
        
        if(p->grabbing_carriable_mob) {
            p->focused_mob->carrier_info->current_carrying_strength -= p->pik_type->carry_strength;
            
            //Did this Pikmin leaving made the mob stop moving?
            if(p->focused_mob->carrier_info->current_carrying_strength < p->focused_mob->type->weight) {
                p->focused_mob->remove_target(true);
                p->focused_mob->carrier_info->decided_type = NULL;
            } else {
                start_moving_carried_object(p->focused_mob, NULL, p); //Enter this code so that if this Pikmin leaving broke a tie, the Onion's picked correctly.
            }
        }
    }
    
    p->focused_mob = NULL;
    p->grabbing_carriable_mob = false;
    p->remove_target(true);
    
    sfx_pikmin_carrying.stop();
    
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