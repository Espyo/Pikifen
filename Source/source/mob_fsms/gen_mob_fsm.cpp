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
 * Event handler that makes a mob lose health by being damaged by another.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void gen_mob_fsm::be_attacked(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    hitbox_interaction* info = (hitbox_interaction*) info1;
    
    float damage = 0;
    if(!info->mob2->calculate_damage(m, info->h2, info->h1, &damage)) {
        return;
    }
    
    m->apply_attack_damage(info->mob2, info->h2, info->h1, damage);
    m->do_attack_effects(info->mob2, info->h2, info->h1, damage, 0.0f);
}


const float CARRY_STUCK_CIRCLING_RADIUS = 8.0f;
const float CARRY_STUCK_SPEED_MULTIPLIER = 0.4f;
/* ----------------------------------------------------------------------------
 * When it's time to become stuck and move in circles.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void gen_mob_fsm::carry_become_stuck(mob* m, void* info1, void* info2) {
    engine_assert(m->carry_info != NULL, m->print_state_history());
    
    m->circle_around(
        NULL, m->pos, CARRY_STUCK_CIRCLING_RADIUS, true,
        m->carry_info->get_speed() * CARRY_STUCK_SPEED_MULTIPLIER,
        true
    );
}


/* ----------------------------------------------------------------------------
 * When it's time to check if a carried object should begin moving,
 * or update its path.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void gen_mob_fsm::carry_begin_move(mob* m, void* info1, void* info2) {
    m->carry_info->is_moving = true;
    
    m->can_move_in_midair =
        (m->path_info->taker_flags & PATH_TAKER_FLAG_AIRBORNE);
        
    if(m->carry_info->intended_mob == NULL) {
        m->fsm.run_event(MOB_EV_PATH_BLOCKED);
        return;
    }
}


/* ----------------------------------------------------------------------------
 * When a mob wants a new path.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void gen_mob_fsm::carry_get_path(mob* m, void* info1, void* info2) {
    float target_distance = 3.0f;
    if(m->carry_info->destination == CARRY_DESTINATION_SHIP) {
        //Because the ship's beam can be offset, and because
        //the ship is normally in the way, let's consider a
        //"reached destination" event if the treasure is
        //covering the beam, and not necessarily if the treasure
        //is on the same coordinates as the beam.
        if(m->carry_info->intended_mob) {
            ship* s_ptr = (ship*) m->carry_info->intended_mob;
            target_distance =
                std::max(
                    m->radius -
                    s_ptr->shi_type->beam_radius,
                    3.0f
                );
        }
    }
    
    m->follow_path(
        m->carry_info->intended_point, true,
        m->carry_info->get_speed(), target_distance
    );
    
    m->path_info->target_point = m->carry_info->intended_point;
    
    if(m->path_info->path.empty() && !m->path_info->go_straight) {
        m->fsm.run_event(MOB_EV_PATH_BLOCKED);
    }
}


/* ----------------------------------------------------------------------------
 * When a mob reaches the destination or an obstacle when carrying.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void gen_mob_fsm::carry_reach_destination(mob* m, void* info1, void* info2) {
    m->stop_following_path();
    
    if(m->delivery_info) {
        delete m->delivery_info;
    }
    m->delivery_info = new delivery_info_struct();
    if(m->carry_info->intended_pik_type) {
        m->delivery_info->color = m->carry_info->intended_pik_type->main_color;
        m->delivery_info->intended_pik_type = m->carry_info->intended_pik_type;
    }
    
    m->fsm.run_event(MOB_EV_CARRY_DELIVERED);
}


/* ----------------------------------------------------------------------------
 * When a mob is no longer stuck waiting to be carried.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void gen_mob_fsm::carry_stop_being_stuck(mob* m, void* info1, void* info2) {
    m->stop_circling();
}


/* ----------------------------------------------------------------------------
 * When a carried object stops moving.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void gen_mob_fsm::carry_stop_move(mob* m, void* info1, void* info2) {
    if(!m->carry_info) return;
    if(!m->path_info) return;
    m->carry_info->is_moving = false;
    m->can_move_in_midair = false;
    m->stop_following_path();
    m->stop_chasing();
}


/* ----------------------------------------------------------------------------
 * Event handler that makes a mob die.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void gen_mob_fsm::die(mob* m, void* info1, void* info2) {
    if(m->type->death_state_nr == INVALID) return;
    m->fsm.set_state(m->type->death_state_nr, info1, info2);
}


/* ----------------------------------------------------------------------------
 * Event handler that makes a mob fall into a pit and vanish.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void gen_mob_fsm::fall_down_pit(mob* m, void* info1, void* info2) {
    m->set_health(false, false, 0);
    m->finish_dying();
    m->to_delete = true;
}


/* ----------------------------------------------------------------------------
 * Event handler for a Pikmin being added as a carrier.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void gen_mob_fsm::handle_carrier_added(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) info1;
    
    //Save some data before changing anything.
    bool could_move = m->carry_info->cur_carrying_strength >= m->type->weight;
    mob* prev_destination = m->carry_info->intended_mob;
    
    //Update the numbers and such.
    m->carry_info->spot_info[pik_ptr->temp_i].pik_ptr = pik_ptr;
    m->carry_info->spot_info[pik_ptr->temp_i].state = CARRY_SPOT_USED;
    m->carry_info->cur_carrying_strength += pik_ptr->pik_type->carry_strength;
    m->carry_info->cur_n_carriers++;
    
    m->chase_info.max_speed = m->carry_info->get_speed();
    
    m->calculate_carrying_destination(
        pik_ptr, NULL,
        &m->carry_info->intended_pik_type,
        &m->carry_info->intended_mob, &m->carry_info->intended_point
    );
    
    //Check if we need to update the path.
    bool must_update = false;
    
    //Start by checking if the mob can now start moving,
    //or if it already could and the target changed.
    bool can_move = m->carry_info->cur_carrying_strength >= m->type->weight;
    if(can_move) {
        if(
            !could_move ||
            (prev_destination != m->carry_info->intended_mob)
        ) {
            must_update = true;
        }
    }
    
    //Now, check if the fact that it can fly or not changed.
    if(!must_update && m->path_info) {
        bool old_is_airborne =
            (m->path_info->taker_flags & PATH_TAKER_FLAG_AIRBORNE);
        bool new_is_airborne = m->carry_info->can_fly();
        must_update = old_is_airborne != new_is_airborne;
    }
    
    //Check if the list of invulnerabilities changed.
    if(!must_update && m->path_info) {
        vector<hazard*> new_invulnerabilities =
            m->carry_info->get_carrier_invulnerabilities();
            
        if(
            !vectors_contain_same(
                new_invulnerabilities,
                m->path_info->invulnerabilities
            )
        ) {
            must_update = true;
        }
    }
    
    if(must_update) {
        //Send a move begin event, so that the mob can calculate
        //a (new) path and start taking it.
        m->fsm.run_event(MOB_EV_CARRY_BEGIN_MOVE);
    }
}


/* ----------------------------------------------------------------------------
 * Event handler for a carrier Pikmin being removed.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void gen_mob_fsm::handle_carrier_removed(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) info1;
    
    //Save some data before changing anything.
    bool could_move = m->carry_info->cur_carrying_strength >= m->type->weight;
    mob* prev_destination = m->carry_info->intended_mob;
    
    //Update the numbers and such.
    m->carry_info->spot_info[pik_ptr->temp_i].pik_ptr = NULL;
    m->carry_info->spot_info[pik_ptr->temp_i].state = CARRY_SPOT_FREE;
    m->carry_info->cur_carrying_strength -= pik_ptr->pik_type->carry_strength;
    m->carry_info->cur_n_carriers--;
    
    m->chase_info.max_speed = m->carry_info->get_speed();
    
    m->calculate_carrying_destination(
        NULL, pik_ptr,
        &m->carry_info->intended_pik_type,
        &m->carry_info->intended_mob, &m->carry_info->intended_point
    );
    
    bool can_move = m->carry_info->cur_carrying_strength >= m->type->weight;
    
    //If the mob can no longer move, send a move stop event,
    //so the mob, well, stops.
    if(could_move && !can_move) {
        m->fsm.run_event(MOB_EV_CARRY_STOP_MOVE);
        return;
    }
    
    //Check if we need to update the path.
    bool must_update = false;
    
    //Start by checking if the target changed.
    if(can_move && (prev_destination != m->carry_info->intended_mob)) {
        must_update = true;
    }
    
    //Now, check if the fact that it can fly or not changed.
    if(!must_update && m->path_info) {
        bool old_is_airborne =
            (m->path_info->taker_flags & PATH_TAKER_FLAG_AIRBORNE);
        bool new_is_airborne = m->carry_info->can_fly();
        must_update = old_is_airborne != new_is_airborne;
    }
    
    //Check if the list of invulnerabilities changed.
    if(!must_update && m->path_info) {
        vector<hazard*> new_invulnerabilities =
            m->carry_info->get_carrier_invulnerabilities();
            
        if(
            !vectors_contain_same(
                new_invulnerabilities,
                m->path_info->invulnerabilities
            )
        ) {
            must_update = true;
        }
    }
    
    if(must_update) {
        //Send a move begin event, so that the mob can calculate
        //a (new) path and start taking it.
        m->fsm.run_event(MOB_EV_CARRY_BEGIN_MOVE);
    }
}


/* ----------------------------------------------------------------------------
 * Generic handler for when a mob was delivered to an Onion/ship.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void gen_mob_fsm::handle_delivery(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != NULL, m->print_state_history());
    
    m->focused_mob->fsm.run_event(
        MOB_EV_RECEIVE_DELIVERY, (void*) m
    );
    
    m->to_delete = true;
}


/* ----------------------------------------------------------------------------
 * When a mob has to lose its momentum.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void gen_mob_fsm::lose_momentum(mob* m, void* info1, void* info2) {
    m->speed.x = m->speed.y = 0.0f;
}


/* ----------------------------------------------------------------------------
 * When a mob starts the process of being delivered to an Onion/ship.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void gen_mob_fsm::start_being_delivered(mob* m, void* info1, void* info2) {
    for(size_t p = 0; p < m->carry_info->spot_info.size(); ++p) {
        mob* p_ptr = m->carry_info->spot_info[p].pik_ptr;
        if(p_ptr) {
            p_ptr->fsm.run_event(MOB_EV_FINISHED_CARRYING);
        }
    }
    
    m->focus_on_mob(m->carry_info->intended_mob);
    m->tangible = false;
    m->become_uncarriable();
    m->set_timer(DELIVERY_SUCK_TIME);
}


/* ----------------------------------------------------------------------------
 * Generic handler for a mob touching a hazard.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void gen_mob_fsm::touch_hazard(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    hazard* h = (hazard*) info1;
    
    for(size_t e = 0; e < h->effects.size(); ++e) {
        m->apply_status_effect(h->effects[e], false);
    }
}


/* ----------------------------------------------------------------------------
 * Generic handler for a mob touching a spray.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void gen_mob_fsm::touch_spray(mob* m, void* info1, void* info2) {
    engine_assert(info1 != NULL, m->print_state_history());
    
    spray_type* s = (spray_type*) info1;
    
    for(size_t e = 0; e < s->effects.size(); ++e) {
        m->apply_status_effect(s->effects[e], false);
    }
}
