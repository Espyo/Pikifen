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
#include "pikmin.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Creates a Pikmin.
 */
pikmin::pikmin(const float x, const float y, pikmin_type* type, const float angle, const string &vars)
    : mob(x, y, type, angle, vars) {
    
    pik_type = type;
    hazard_time_left = -1;
    attacking_mob = NULL;
    latched = false;
    attack_time = 0;
    being_chomped = false;
    carrying_mob = NULL;
    wants_to_carry = NULL;
    pluck_reserved = false;
    team = MOB_TEAM_PLAYER_1; // TODO
    
    maturity = s2i(get_var_value(vars, "maturity", "2"));
    if(s2b(get_var_value(vars, "buried", "0"))) state = PIKMIN_STATE_BURIED;
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
 * Makes a Pikmin release a mob it's carrying.
 */
void drop_mob(pikmin* p) {
    mob* m = (p->carrying_mob ? p->carrying_mob : p->wants_to_carry);
    
    if(!m) return;
    
    // TODO optimize this instead of running through the spot vector.
    for(size_t s = 0; s < m->carrier_info->max_carriers; s++) {
        if(m->carrier_info->carrier_spots[s] == p) {
            m->carrier_info->carrier_spots[s] = NULL;
            break;
        }
    }
    m->carrier_info->current_n_carriers--;
    
    if(p->carrying_mob) {
        m->carrier_info->current_carrying_strength -= p->pik_type->carry_strength;
        
        // Did this Pikmin leaving made the mob stop moving?
        if(p->carrying_mob->carrier_info->current_carrying_strength < p->carrying_mob->type->weight) {
            p->carrying_mob->remove_target(true);
            p->carrying_mob->carrier_info->decided_type = NULL;
            p->carrying_mob->state = MOB_STATE_IDLE;
            sfx_pikmin_carrying.stop();
        } else {
            start_carrying(p->carrying_mob, NULL, p); // Enter this code so that if this Pikmin leaving broke a tie, the Onion's picked correctly.
        }
    }
    
    p->carrying_mob = NULL;
    p->wants_to_carry = NULL;
    p->remove_target(true);
}


/* ----------------------------------------------------------------------------
 * Returns the buried Pikmin closest to a leader. Used when auto-plucking.
 * x/y:             Coordinates of the leader.
 * d:               Variable to return the distance to. NULL for none.
 * ignore_reserved: If true, ignore any buried Pikmin that are "reserved"
   * (i.e. already chosen to be plucked by another leader).
 */
pikmin* get_closest_buried_pikmin(const float x, const float y, float* d, const bool ignore_reserved) {
    float closest_distance = 0;
    pikmin* closest_pikmin = NULL;
    
    size_t n_pikmin = pikmin_list.size();
    for(size_t p = 0; p < n_pikmin; p++) {
        if(pikmin_list[p]->fsm.cur_state->id != PIKMIN_STATE_BURIED) continue;
        
        float dis = dist(x, y, pikmin_list[p]->x, pikmin_list[p]->y);
        if(closest_pikmin == NULL || dis < closest_distance) {
        
            if(!(ignore_reserved && pikmin_list[p]->pluck_reserved)) {
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
    unsigned pikmin_to_keep = 0; // Pikmin to keep inside the Onion, without spitting.
    
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
void start_carrying(mob* m, pikmin* np, pikmin* lp) {
    // TODO what if an Onion hasn't been revelead yet?
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
    
        map<pikmin_type*, unsigned> type_quantity; // How many of each Pikmin type are carrying.
        vector<pikmin_type*> majority_types; // The Pikmin type with the most carriers.
        
        // First, count how many of each type there are carrying.
        for(size_t p = 0; p < m->carrier_info->max_carriers; p++) {
            pikmin* pik_ptr = NULL;
            
            if(m->carrier_info->carrier_spots[p] == NULL) continue;
            if(typeid(*m->carrier_info->carrier_spots[p]) != typeid(pikmin)) continue;
            
            pik_ptr = (pikmin*) m->carrier_info->carrier_spots[p];
            
            if(!pik_ptr->pik_type->has_onion) continue; // If it doesn't have an Onion, it won't even count. // TODO what if it hasn't been discovered / Onion not on this area?
            
            type_quantity[pik_ptr->pik_type]++;
        }
        
        // Then figure out what are the majority types.
        unsigned most = 0;
        for(auto t = type_quantity.begin(); t != type_quantity.end(); t++) {
            if(t->second > most) {
                most = t->second;
                majority_types.clear();
            }
            if(t->second == most) majority_types.push_back(t->first);
        }
        
        // If we ended up with no candidates, pick a type at random, out of all possible types.
        if(majority_types.empty()) {
            for(auto t = pikmin_types.begin(); t != pikmin_types.end(); t++) {
                if(t->second->has_onion) { // TODO what if it hasn't been discovered / Onion not on this area?
                    majority_types.push_back(t->second);
                }
            }
        }
        
        // Now let's pick an Onion.
        if(majority_types.empty()) {
            return; // TODO warn that something went horribly wrong?
            
        } else if(majority_types.size() == 1) {
            // If there's only one possible type to pick, pick it.
            m->carrier_info->decided_type = majority_types[0];
            
        } else {
            // If there's a tie, let's take a careful look.
            bool new_tie = false;
            
            // Is the Pikmin that just joined part of the majority types?
            // If so, that means this Pikmin just created a NEW tie!
            // So let's pick a random Onion again.
            if(np) {
                for(size_t mt = 0; mt < majority_types.size(); mt++) {
                    if(np->type == majority_types[mt]) {
                        new_tie = true;
                        break;
                    }
                }
            }
            
            // If a Pikmin left, check if it is related to the majority types.
            // If not, then a new tie wasn't made, no worries.
            // If it was related, a new tie was created.
            if(lp) {
                new_tie = false;
                for(size_t mt = 0; mt < majority_types.size(); mt++) {
                    if(lp->type == majority_types[mt]) {
                        new_tie = true;
                        break;
                    }
                }
            }
            
            // Check if the previously decided type belongs to one of the majorities.
            // If so, it can be chosen again, but if not, it cannot.
            bool can_continue = false;
            for(size_t mt = 0; mt < majority_types.size(); mt++) {
                if(majority_types[mt] == m->carrier_info->decided_type) {
                    can_continue = true;
                    break;
                }
            }
            if(!can_continue) m->carrier_info->decided_type = NULL;
            
            // If the Pikmin that just joined is not a part of the majorities,
            // then it had no impact on the existing ties.
            // Go with the Onion that had been decided before.
            if(new_tie || !m->carrier_info->decided_type) {
                m->carrier_info->decided_type = majority_types[randomi(0, majority_types.size() - 1)];
            }
        }
        
        
        // Figure out where that type's Onion is.
        size_t onion_nr = 0;
        for(; onion_nr < onions.size(); onion_nr++) {
            if(onions[onion_nr]->oni_type->pik_type == m->carrier_info->decided_type) {
                break;
            }
        }
        
        // Finally, start moving the mob.
        m->set_target(onions[onion_nr]->x, onions[onion_nr]->y, NULL, NULL, false);
        m->set_state(MOB_STATE_BEING_CARRIED);
        sfx_pikmin_carrying.play(-1, true);
    }
}

void pikmin::become_buried(mob* m, void* info) {
    m->anim.change(PIKMIN_ANIM_BURROWED, true, false, false);
}

void pikmin::be_plucked(mob* m, void* info) {
    pluck_info* pi = (pluck_info*) info;
    pikmin* pik = (pikmin*) m;
    
    pi->leader_who_plucked->pluck_time = -1;
    pik->anim.change(PIKMIN_ANIM_IDLE, true, false, false);
    add_to_party(pi->new_leader, pik);
    sfx_pikmin_plucked.play(0, false);
    sfx_pikmin_pluck.play(0, false);
}

void pikmin::be_grabbed_by_friend(mob* m, void* info) {
    sfx_pikmin_held.play(0, false);
}

void pikmin::be_dismissed(mob* m, void* info) {
    float angle = *((float*) info);
    pikmin* pik = (pikmin*) m;
    
    pik->set_target(
        leaders[cur_leader_nr]->x + cos(angle) * DISMISS_DISTANCE,
        leaders[cur_leader_nr]->y + sin(angle) * DISMISS_DISTANCE,
        NULL,
        NULL,
        false
    );
}

void pikmin::be_thrown(mob* m, void* info) {
    m->anim.change(PIKMIN_ANIM_THROWN, true, false, false);
}

void pikmin::land(mob* m, void* info) {
    m->anim.change(PIKMIN_ANIM_IDLE, true, false, false);
}

void pikmin::go_to_task(mob* m, void* info) {

}

void pikmin::called(mob* m, void* info) {
    pikmin* pik = (pikmin*) m;
    
    drop_mob(pik);
    pik->attacking_mob = NULL;
    pik->attack_time = 0;
    add_to_party(leaders[cur_leader_nr], pik);
    sfx_pikmin_called.play(0.03, false);
    
    pik->attacking_mob = NULL;
}

void pikmin::work_on_task(mob* m, void* info) {

}

void pikmin::chase_leader(mob* m, void* info) {
    m->set_target(0, 0, &m->following_party->x, &m->following_party->y, false);
    m->anim.change(PIKMIN_ANIM_WALK, true, false, false);
}

void pikmin::stop_in_group(mob* m, void* info) {
    m->remove_target(true);
    m->anim.change(PIKMIN_ANIM_IDLE, true, false, false);
}