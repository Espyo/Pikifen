/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Generic mob finite state machine logic.
 */

#include "../const.h"
#include "enemy.h"
#include "mob.h"
#include "mob_fsm.h"
#include "onion.h"
#include "pikmin.h"

/* ----------------------------------------------------------------------------
 * Generic handler for when the mob was delivered to an Onion/ship.
 */
void gen_mob_fsm::handle_delivery(mob* m, void* info1, void* info2) {
    enemy* e_ptr = (enemy*) m;
    onion* o_ptr = (onion*) e_ptr->carrying_target;
    
    size_t seeds = e_ptr->ene_type->pikmin_seeds;
    
    o_ptr->fsm.run_event(MOB_EVENT_RECEIVE_DELIVERY, (void*) seeds);
    
    e_ptr->to_delete = true;
}


/* ----------------------------------------------------------------------------
 * Event handler that makes the mob lose health by being damaged by another.
 */
void gen_mob_fsm::lose_health(mob* m, void* info1, void* info2) {
    hitbox_touch_info* info = (hitbox_touch_info*) info1;
    if(!should_attack(m, info->mob2)) return;
    
    float damage = 0;
    
    damage = calculate_damage(info->mob2, m, info->hi2, info->hi1);
    m->health -= damage;
    
    m->fsm.run_event(MOB_EVENT_DAMAGE, info->mob2);
    
    //If before taking damage, the interval was dividable X times, and after it's only dividable by Y (X>Y), an interval was crossed.
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
 * Begin moving a carried object.
 */
void gen_mob_fsm::carry_begin_move(mob* m, void* info1, void* info2) {
    mob* obs = NULL;
    bool go_straight = false;
    vector<path_stop*> old_path = m->path;
    
    m->path = get_path(
                  m->x, m->y,
                  m->carry_info->final_destination_x,
                  m->carry_info->final_destination_y,
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
    
    m->carry_info->is_moving = true;
}


/* ----------------------------------------------------------------------------
 * Stop moving a carried object.
 */
void gen_mob_fsm::carry_stop_move(mob* m, void* info1, void* info2) {
    if(!m->carry_info) return;
    m->carry_info->is_moving = false;
    m->stop_chasing();
}


/* ----------------------------------------------------------------------------
 * Checks if the Pikmin should start carrying the mob.
 */
void gen_mob_fsm::check_carry_begin(mob* m, void* info1, void* info2) {

    for(size_t s = 0; s < m->carry_info->spot_info.size(); ++s) {
        if(m->carry_info->spot_info[s].state == CARRY_SPOT_RESERVED) {
            //If a Pikmin is coming, no, we can't move yet.
            return;
        }
    }
    
    if(m->carry_info->cur_carrying_strength >= m->type->weight) {
        m->fsm.run_event(MOB_EVENT_CARRY_BEGIN_MOVE);
    }
}


/* ----------------------------------------------------------------------------
 * Checks if the Pikmin should stop carrying the mob.
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
 * Starts the process of a mob being delivered to an Onion/ship.
 */
void gen_mob_fsm::start_being_delivered(mob* m, void* info1, void* info2) {
    m->tangible = false;
    m->become_uncarriable();
    m->set_timer(DELIVERY_SUCK_TIME);
}


const float CARRYING_STUCK_SWAY_AMOUNT = 20.0f;
const float CARRYING_STUCK_SPEED_MULT = 0.3f;
/* ----------------------------------------------------------------------------
 * Sets the next target when following a path.
 */
void gen_mob_fsm::set_next_target(mob* m, void* info1, void* info2) {
    m->cur_path_stop_nr = (m->cur_path_stop_nr == INVALID ? 0 : m->cur_path_stop_nr + 1);
    
    if(m->carry_info->stuck_state > 0) {
        //Stuck... Let's go back and forth between point A and B.
        float final_x = m->x;
        if(m->carry_info->stuck_state == 1) {
            m->carry_info->stuck_state = 2;
            final_x += CARRYING_STUCK_SWAY_AMOUNT;
        } else {
            m->carry_info->stuck_state = 1;
            final_x -= CARRYING_STUCK_SWAY_AMOUNT;
        }
        
        m->chase(
            final_x, m->y,
            NULL, NULL,
            false, NULL, true, 3.0f,
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
            m->chase(
                m->carry_info->final_destination_x,
                m->carry_info->final_destination_y,
                NULL, NULL, false, NULL, true, 3.0f,
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
            m->path[m->cur_path_stop_nr]->x,
            m->path[m->cur_path_stop_nr]->y,
            NULL, NULL, false, NULL, true, 3.0f,
            m->carry_info->get_speed()
        );
        
    }
}
