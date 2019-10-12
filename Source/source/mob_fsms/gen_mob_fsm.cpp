/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Generic mob finite state machine logic.
 */

#include <algorithm>

#include "gen_mob_fsm.h"

#include "../const.h"
#include "../functions.h"
#include "../mobs/enemy.h"
#include "../mobs/onion.h"
#include "../mobs/pikmin.h"
#include "../mobs/ship.h"
#include "../spray_type.h"

/* ----------------------------------------------------------------------------
 * Generic handler for a mob touching a hazard.
 */
void gen_mob_fsm::touch_hazard(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    hazard* h = (hazard*) info1;
    
    for(size_t e = 0; e < h->effects.size(); ++e) {
        m->apply_status_effect(h->effects[e], false, false);
    }
}


/* ----------------------------------------------------------------------------
 * Generic handler for a mob touching a spray.
 */
void gen_mob_fsm::touch_spray(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    spray_type* s = (spray_type*) info1;
    
    for(size_t e = 0; e < s->effects.size(); ++e) {
        m->apply_status_effect(s->effects[e], false, false);
    }
}


/* ----------------------------------------------------------------------------
 * Generic handler for when a mob was delivered to an Onion/ship.
 */
void gen_mob_fsm::handle_delivery(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != NULL, m->print_state_history());
    
    m->focused_mob->fsm.run_event(
        MOB_EVENT_RECEIVE_DELIVERY, (void*) m
    );
    
    m->to_delete = true;
}


/* ----------------------------------------------------------------------------
 * Event handler that makes a mob lose health by being damaged by another.
 */
void gen_mob_fsm::be_attacked(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    hitbox_interaction* info = (hitbox_interaction*) info1;
    
    float damage = 0;
    if(!info->mob2->calculate_damage(m, info->h2, info->h1, &damage)) {
        return;
    }
    
    m->apply_attack_damage(info->mob2, info->h2, info->h1, damage);
    m->do_attack_effects(info->mob2, info->h2, info->h1, damage);
}


/* ----------------------------------------------------------------------------
 * Event handler that makes a mob die.
 */
void gen_mob_fsm::die(mob* m, void* info1, void* info2) {
    if(m->type->death_state_nr == INVALID) return;
    m->fsm.set_state(m->type->death_state_nr, info1, info2);
}


/* ----------------------------------------------------------------------------
 * Event handler that makes a mob fall into a pit and vanish.
 */
void gen_mob_fsm::fall_down_pit(mob* m, void* info1, void* info2) {
    m->set_health(false, false, 0);
    m->finish_dying();
    m->to_delete = true;
}


/* ----------------------------------------------------------------------------
 * Event handler for a Pikmin being added as a carrier.
 */
void gen_mob_fsm::handle_carrier_added(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) info1;
    
    m->carry_info->spot_info[pik_ptr->carrying_spot].pik_ptr = pik_ptr;
    m->carry_info->spot_info[pik_ptr->carrying_spot].state = CARRY_SPOT_USED;
    m->carry_info->cur_carrying_strength += pik_ptr->pik_type->carry_strength;
    m->carry_info->cur_n_carriers++;
    
    m->chase_speed = m->carry_info->get_speed();
    
    m->calculate_carrying_destination(
        pik_ptr, NULL,
        &m->carry_info->intended_mob, &m->carry_info->intended_point
    );
}


/* ----------------------------------------------------------------------------
 * Event handler for a carrier Pikmin being removed.
 */
void gen_mob_fsm::handle_carrier_removed(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) info1;
    
    m->carry_info->spot_info[pik_ptr->carrying_spot].pik_ptr = NULL;
    m->carry_info->spot_info[pik_ptr->carrying_spot].state = CARRY_SPOT_FREE;
    m->carry_info->cur_carrying_strength -= pik_ptr->pik_type->carry_strength;
    m->carry_info->cur_n_carriers--;
    
    m->chase_speed = m->carry_info->get_speed();
    
    m->calculate_carrying_destination(
        NULL, pik_ptr,
        &m->carry_info->intended_mob, &m->carry_info->intended_point
    );
}


const float CARRY_STUCK_CIRCLING_RADIUS = 8.0f;
const float CARRY_STUCK_SPEED_MULTIPLIER = 0.4f;
/* ----------------------------------------------------------------------------
 * When it's time to become stuck and move in circles.
 */
void gen_mob_fsm::carry_become_stuck(mob* m, void* info1, void* info2) {
    engine_assert(m->carry_info != NULL, m->print_state_history());
    
    m->carry_info->is_stuck = true;
    if(m->path_info) {
        m->carry_info->obstacle_ptrs = m->path_info->obstacle_ptrs;
    }
    m->stop_following_path();
    
    m->circle_around(
        NULL, m->pos, CARRY_STUCK_CIRCLING_RADIUS, true,
        m->carry_info->get_speed() * CARRY_STUCK_SPEED_MULTIPLIER,
        true
    );
}


/* ----------------------------------------------------------------------------
 * When it's time to check if a carried object should begin moving,
 * or update its path.
 */
void gen_mob_fsm::carry_begin_move(mob* m, void* info1, void* info2) {
    m->carry_info->is_moving = true;
    
    if(m->carry_info->intended_mob == NULL) {
        m->fsm.run_event(MOB_EVENT_CARRY_STUCK);
        return;
    }
    
    float target_distance = 3.0f;
    if(m->carry_info->destination == CARRY_DESTINATION_SHIP) {
        //Because the ship's beam can be offset, and because
        //the ship is normally in the way, let's consider a
        //"reached destination" event if the treasure is
        //covering the beam, and not necessarily if the treasure
        //is on the same coordinates as the beam.
        ship* s_ptr = (ship*) m->carry_info->intended_mob;
        target_distance =
            max(
                m->type->radius -
                s_ptr->shi_type->beam_radius,
                3.0f
            );
    }
    
    m->follow_path(
        m->carry_info->intended_point, true,
        m->carry_info->get_speed(), target_distance
    );
    
    m->path_info->target_point = m->carry_info->intended_point;
    
    if(m->path_info->path.empty() && !m->path_info->go_straight) {
        m->fsm.run_event(MOB_EVENT_CARRY_STUCK);
    }
}


/* ----------------------------------------------------------------------------
 * When a carried object stops moving.
 */
void gen_mob_fsm::carry_stop_move(mob* m, void* info1, void* info2) {
    if(!m->carry_info) return;
    if(!m->path_info) return;
    m->carry_info->is_moving = false;
    m->stop_following_path();
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
    for(size_t p = 0; p < m->carry_info->spot_info.size(); ++p) {
        mob* p_ptr = m->carry_info->spot_info[p].pik_ptr;
        if(p_ptr) {
            p_ptr->fsm.run_event(MOB_EVENT_FINISHED_CARRYING);
        }
    }
    
    m->focus_on_mob(m->carry_info->intended_mob);
    m->tangible = false;
    m->become_uncarriable();
    m->set_timer(DELIVERY_SUCK_TIME);
}


/* ----------------------------------------------------------------------------
 * When a mob is no longer stuck waiting to be carried.
 */
void gen_mob_fsm::carry_stop_being_stuck(mob* m, void* info1, void* info2) {
    if(m->carry_info) {
        m->carry_info->is_stuck = false;
        m->carry_info->obstacle_ptrs.clear();
    }
    m->stop_circling();
}


/* ----------------------------------------------------------------------------
 * When a mob reaches the destination or an obstacle when carrying.
 * info1: If not NULL, then it's impossible to progress because of an obstacle.
 */
void gen_mob_fsm::carry_reach_destination(mob* m, void* info1, void* info2) {
    if(info1) {
        //Stuck...
        m->fsm.run_event(MOB_EVENT_CARRY_STUCK);
    } else {
        //Successful delivery!
        m->stop_following_path();
        m->fsm.run_event(MOB_EVENT_CARRY_DELIVERED);
    }
}
