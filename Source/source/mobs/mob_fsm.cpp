/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Generic mob finite state machine logic.
 */

#include <algorithm>

#include "../const.h"
#include "enemy.h"
#include "mob.h"
#include "mob_fsm.h"
#include "onion.h"
#include "pikmin.h"
#include "ship.h"
#include "../spray_type.h"

/* ----------------------------------------------------------------------------
 * Generic handler for a mob touching a hazard.
 */
void gen_mob_fsm::touch_hazard(mob* m, void* info1, void* info2) {
    hazard* h = (hazard*) info1;
    
    for(size_t e = 0; e < h->effects.size(); ++e) {
        m->apply_status_effect(h->effects[e], false);
    }
}


/* ----------------------------------------------------------------------------
 * Generic handler for a mob touching a spray.
 */
void gen_mob_fsm::touch_spray(mob* m, void* info1, void* info2) {
    spray_type* s = (spray_type*) info1;
    
    for(size_t e = 0; e < s->effects.size(); ++e) {
        m->apply_status_effect(s->effects[e], false);
    }
}


/* ----------------------------------------------------------------------------
 * Generic handler for when a mob was delivered to an Onion/ship.
 */
void gen_mob_fsm::handle_delivery(mob* m, void* info1, void* info2) {
    enemy* e_ptr = (enemy*) m;
    onion* o_ptr = (onion*) e_ptr->carrying_target;
    
    size_t seeds = e_ptr->ene_type->pikmin_seeds;
    
    o_ptr->fsm.run_event(MOB_EVENT_RECEIVE_DELIVERY, (void*) seeds);
    
    e_ptr->to_delete = true;
}


/* ----------------------------------------------------------------------------
 * Event handler that makes a mob lose health by being damaged by another.
 */
void gen_mob_fsm::lose_health(mob* m, void* info1, void* info2) {
    hitbox_touch_info* info = (hitbox_touch_info*) info1;
    if(!should_attack(info->mob2, m)) return;
    
    float damage = 0;
    
    damage = calculate_damage(info->mob2, m, info->h2, info->h1);
    m->health -= damage;
    
    m->fsm.run_event(MOB_EVENT_DAMAGE, info->mob2);
    
    //If before taking damage, the interval was dividable X times,
    //and after it's only dividable by Y (X>Y), an interval was crossed.
    if(
        m->type->big_damage_interval > 0 &&
        m->health != m->type->max_health
    ) {
        if(
            floor((m->health + damage) / m->type->big_damage_interval) >
            floor(m->health / m->type->big_damage_interval)
        ) {
            m->big_damage_ev_queued = true;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Event handler for a Pikmin being added as a carrier.
 */
void gen_mob_fsm::handle_carrier_added(mob* m, void* info1, void* info2) {
    if(!info1) {
        m->calculate_carrying_destination(NULL, NULL);
        return;
    }
    
    pikmin* pik_ptr = (pikmin*) info1;
    
    m->carry_info->spot_info[pik_ptr->carrying_spot].pik_ptr = pik_ptr;
    m->carry_info->spot_info[pik_ptr->carrying_spot].state = CARRY_SPOT_USED;
    m->carry_info->cur_carrying_strength += pik_ptr->pik_type->carry_strength;
    m->carry_info->cur_n_carriers++;
    
    m->chase_speed = m->carry_info->get_speed();
    
    m->calculate_carrying_destination(pik_ptr, NULL);
}


/* ----------------------------------------------------------------------------
 * Event handler for a carrier Pikmin being removed.
 */
void gen_mob_fsm::handle_carrier_removed(mob* m, void* info1, void* info2) {
    if(!info1) {
        m->calculate_carrying_destination(NULL, NULL);
        return;
    }
    
    pikmin* pik_ptr = (pikmin*) info1;
    
    m->carry_info->spot_info[pik_ptr->carrying_spot].pik_ptr = NULL;
    m->carry_info->spot_info[pik_ptr->carrying_spot].state = CARRY_SPOT_FREE;
    m->carry_info->cur_carrying_strength -= pik_ptr->pik_type->carry_strength;
    m->carry_info->cur_n_carriers--;
    
    m->chase_speed = m->carry_info->get_speed();
    
    m->calculate_carrying_destination(NULL, pik_ptr);
}


/* ----------------------------------------------------------------------------
 * When a carried object begins moving.
 */
void gen_mob_fsm::carry_begin_move(mob* m, void* info1, void* info2) {
    mob* obs = NULL;
    bool go_straight = false;
    vector<path_stop*> old_path = m->path;
    
    if(m->carrying_target) {
        m->path =
            get_path(
                m->pos, m->carry_info->final_destination,
                &obs, &go_straight, NULL
            );
        m->carry_info->obstacle_ptr = obs;
        m->carry_info->go_straight = go_straight;
        
        if(
            m->path.size() >= 2 &&
            m->cur_path_stop_nr < old_path.size() &&
            m->path[1] == old_path[m->cur_path_stop_nr]
        ) {
            //If the second stop of the old path is the
            //same as the stop it was already going towards,
            //then just go there right away, instead of doing a back-and-forth.
            m->cur_path_stop_nr = 0;
        } else {
            m->cur_path_stop_nr = INVALID;
        }
        
        if(m->path.empty() && !go_straight) {
            m->carry_info->stuck_state = 1;
        } else {
            m->carry_info->stuck_state = 0;
        }
    }
    
    m->carry_info->is_moving = true;
}


/* ----------------------------------------------------------------------------
 * When a carried object stops moving.
 */
void gen_mob_fsm::carry_stop_move(mob* m, void* info1, void* info2) {
    if(!m->carry_info) return;
    m->carry_info->is_moving = false;
    m->stop_chasing();
}


/* ----------------------------------------------------------------------------
 * When a Pikmin checks if it should start carrying the mob.
 */
void gen_mob_fsm::check_carry_begin(mob* m, void* info1, void* info2) {

    if(m->carry_info->cur_carrying_strength >= m->type->weight) {
        m->fsm.run_event(MOB_EVENT_CARRY_BEGIN_MOVE);
    }
}


/* ----------------------------------------------------------------------------
 * When a Pikmin checks if it should stop carrying the mob.
 */
void gen_mob_fsm::check_carry_stop(mob* m, void* info1, void* info2) {
    bool run_event = false;
    
    for(size_t s = 0; s < m->carry_info->spot_info.size(); ++s) {
        if(m->carry_info->spot_info[s].state == CARRY_SPOT_RESERVED) {
            //If a Pikmin is coming, we should wait.
            run_event = true;
            break;
        }
    }
    
    if(m->carry_info->cur_carrying_strength < m->type->weight || run_event) {
        m->fsm.run_event(MOB_EVENT_CARRY_STOP_MOVE);
    }
}


/* ----------------------------------------------------------------------------
 * When a mob starts the process of being delivered to an Onion/ship.
 */
void gen_mob_fsm::start_being_delivered(mob* m, void* info1, void* info2) {
    m->tangible = false;
    m->become_uncarriable();
    m->set_timer(DELIVERY_SUCK_TIME);
}


/* ----------------------------------------------------------------------------
 * When a mob sets the next target when following a path.
 */
void gen_mob_fsm::set_next_target(mob* m, void* info1, void* info2) {
    m->cur_path_stop_nr =
        (m->cur_path_stop_nr == INVALID ? 0 : m->cur_path_stop_nr + 1);
        
    if(m->carry_info->stuck_state > 0) {
        //Stuck... Let's go back and forth between point A and B.
        float final_x = m->pos.x;
        if(m->carry_info->stuck_state == 1) {
            m->carry_info->stuck_state = 2;
            final_x += CARRYING_STUCK_SWAY_AMOUNT;
        } else {
            m->carry_info->stuck_state = 1;
            final_x -= CARRYING_STUCK_SWAY_AMOUNT;
        }
        
        m->chase(
            point(final_x, m->pos.y),
            NULL, false, NULL, true, 3.0f,
            m->carry_info->get_speed() * CARRYING_STUCK_SPEED_MULT
        );
        
    } else if(m->cur_path_stop_nr == m->path.size()) {
        //Reached the final stop.
        
        if(m->carry_info->obstacle_ptr) {
            //If there's an obstacle in the path, the last stop on the path
            //actually means it's the last possible stop before the obstacle.
            //Meaning the object should get stuck.
            m->carry_info->stuck_state = 1;
            
        } else {
            //Go to the final destination.
            float target_distance = 3.0f;
            if(m->carry_info->carry_to_ship) {
                //Because the ship's beam can be offset, and because
                //the ship is normally in the way, let's consider a
                //"reached destination" event if the treasure is
                //covering the beam, and not necessarily if the treasure
                //is on the same coordinates as the beam.
                target_distance =
                    max(
                        m->type->radius -
                        ((ship*) m->carrying_target)->shi_type->beam_radius,
                        3.0f
                    );
            }
            m->chase(
                m->carry_info->final_destination, NULL, false, NULL, true, target_distance,
                m->carry_info->get_speed()
            );
            
        }
        
    } else if(m->cur_path_stop_nr == m->path.size() + 1) {
        //Reached the final destination.
        //Send event.
        m->stop_chasing();
        m->fsm.run_event(MOB_EVENT_CARRY_DELIVERED);
        
    } else {
        //Reached a stop.
        //Go to the next.
        m->chase(
            m->path[m->cur_path_stop_nr]->pos,
            NULL, false, NULL, true, 3.0f,
            m->carry_info->get_speed()
        );
        
    }
}
