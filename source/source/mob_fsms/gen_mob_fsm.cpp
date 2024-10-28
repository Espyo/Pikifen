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
#include "../mobs/bridge.h"
#include "../mobs/enemy.h"
#include "../mobs/onion.h"
#include "../mobs/pikmin.h"
#include "../mobs/ship.h"
#include "../spray_type.h"
#include "../utils/general_utils.h"


/**
 * @brief Event handler that makes a mob lose health by being damaged
 * by another.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::be_attacked(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    hitbox_interaction* info = (hitbox_interaction*) info1;
    
    float damage = 0;
    if(!info->mob2->calculate_damage(m, info->h2, info->h1, &damage)) {
        return;
    }
    
    m->apply_attack_damage(info->mob2, info->h2, info->h1, damage);
    m->do_attack_effects(info->mob2, info->h2, info->h1, damage, 0.0f);
}


/**
 * @brief When it's time to become stuck and move in circles.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::carry_become_stuck(mob* m, void* info1, void* info2) {
    engine_assert(m->carry_info != nullptr, m->print_state_history());
    
    m->circle_around(
        nullptr, m->pos, MOB::CARRY_STUCK_CIRCLING_RADIUS, true,
        m->carry_info->get_speed() * MOB::CARRY_STUCK_SPEED_MULTIPLIER,
        true
    );
}


/**
 * @brief When it's time to check if a carried object should begin moving,
 * or update its path.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::carry_begin_move(mob* m, void* info1, void* info2) {
    m->carry_info->is_moving = true;
    
    has_flag(m->path_info->settings.flags, PATH_FOLLOW_FLAG_AIRBORNE) ?
    enable_flag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR) :
    disable_flag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    
    if(!m->carry_info->destination_exists) {
        m->path_info->result = PATH_RESULT_NO_DESTINATION;
    }
    
    if(m->path_info->result < 0) {
        m->fsm.run_event(MOB_EV_PATH_BLOCKED);
        return;
    }
}


/**
 * @brief When a mob wants a new path.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::carry_get_path(mob* m, void* info1, void* info2) {
    path_follow_settings settings;
    enable_flag(settings.flags, PATH_FOLLOW_FLAG_CAN_CONTINUE);
    
    if(m->carry_info->destination == CARRY_DESTINATION_SHIP) {
        //Special case: ships.
        //Because the ship's control point can be offset, and because
        //the ship is normally in the way, let's consider a
        //"reached destination" event if the treasure is
        //covering the control point, and not necessarily if the treasure
        //is on the same coordinates as the control point.
        if(m->carry_info->intended_mob) {
            ship* shi_ptr = (ship*) m->carry_info->intended_mob;
            settings.final_target_distance =
                std::max(
                    m->radius -
                    shi_ptr->shi_type->control_point_radius,
                    3.0f
                );
        }
        
    } else if(m->carry_info->destination == CARRY_DESTINATION_LINKED_MOB) {
        //Special case: bridges.
        //Pikmin are meant to carry to the current tip of the bridge,
        //but whereas the start of the bridge is on firm ground, the tip may
        //be above a chasm or water, so the Pikmin might want to take a
        //different path, or be unable to take a path at all.
        //Let's fake the end point to be the start of the bridge,
        //for the sake of path calculations.
        if(
            m->carry_info->intended_mob &&
            m->carry_info->intended_mob->type->category->id ==
            MOB_CATEGORY_BRIDGES
        ) {
            bridge* bri_ptr = (bridge*) m->carry_info->intended_mob;
            enable_flag(settings.flags, PATH_FOLLOW_FLAG_FAKED_END);
            enable_flag(settings.flags, PATH_FOLLOW_FLAG_FOLLOW_MOB);
            settings.faked_end = bri_ptr->get_start_point();
        }
    }
    
    settings.target_point = m->carry_info->intended_point;
    settings.target_mob = m->carry_info->intended_mob;
    
    m->follow_path(
        settings, m->carry_info->get_speed(), m->chase_info.acceleration
    );
    
    if(!m->carry_info->destination_exists) {
        m->path_info->result = PATH_RESULT_NO_DESTINATION;
    }
    if(m->path_info->result < 0) {
        m->path_info->block_reason = PATH_BLOCK_REASON_NO_PATH;
        m->fsm.run_event(MOB_EV_PATH_BLOCKED);
    }
}


/**
 * @brief When a mob reaches the destination or an obstacle when being carried.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::carry_reach_destination(mob* m, void* info1, void* info2) {
    m->stop_following_path();
    
    if(m->delivery_info) {
        delete m->delivery_info;
    }
    m->delivery_info = new delivery_t();
    if(m->carry_info->intended_pik_type) {
        m->delivery_info->color = m->carry_info->intended_pik_type->main_color;
        m->delivery_info->intended_pik_type = m->carry_info->intended_pik_type;
    }
    
    m->fsm.run_event(MOB_EV_CARRY_DELIVERED);
}


/**
 * @brief When a mob is no longer stuck waiting to be carried.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::carry_stop_being_stuck(mob* m, void* info1, void* info2) {
    m->stop_circling();
}


/**
 * @brief When a carried object stops moving.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::carry_stop_move(mob* m, void* info1, void* info2) {
    if(!m->carry_info) return;
    m->carry_info->is_moving = false;
    disable_flag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    m->stop_following_path();
    m->stop_chasing();
}


/**
 * @brief Event handler that makes a mob die.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::die(mob* m, void* info1, void* info2) {
    if(m->type->death_state_idx == INVALID) return;
    m->fsm.set_state(m->type->death_state_idx, info1, info2);
}


/**
 * @brief Event handler that makes a mob fall into a pit and vanish.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::fall_down_pit(mob* m, void* info1, void* info2) {
    m->set_health(false, false, 0);
    m->start_dying();
    m->finish_dying();
    m->to_delete = true;
}


/**
 * @brief Event handler for a Pikmin being added as a carrier.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::handle_carrier_added(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) info1;
    
    //Save some data before changing anything.
    bool could_move = m->carry_info->cur_carrying_strength >= m->type->weight;
    mob* prev_destination = m->carry_info->intended_mob;
    
    //Update the numbers and such.
    m->carry_info->spot_info[pik_ptr->temp_i].pik_ptr = pik_ptr;
    m->carry_info->spot_info[pik_ptr->temp_i].state = CARRY_SPOT_STATE_USED;
    m->carry_info->cur_carrying_strength += pik_ptr->pik_type->carry_strength;
    m->carry_info->cur_n_carriers++;
    
    m->chase_info.max_speed = m->carry_info->get_speed();
    m->chase_info.acceleration = MOB::CARRIED_MOB_ACCELERATION;
    
    m->carry_info->destination_exists =
        m->calculate_carrying_destination(
            pik_ptr, nullptr,
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
            has_flag(m->path_info->settings.flags, PATH_FOLLOW_FLAG_AIRBORNE);
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
                m->path_info->settings.invulnerabilities
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


/**
 * @brief Event handler for a carrier Pikmin being removed.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::handle_carrier_removed(mob* m, void* info1, void* info2) {
    pikmin* pik_ptr = (pikmin*) info1;
    
    //Save some data before changing anything.
    bool could_move = m->carry_info->cur_carrying_strength >= m->type->weight;
    mob* prev_destination = m->carry_info->intended_mob;
    
    //Update the numbers and such.
    m->carry_info->spot_info[pik_ptr->temp_i].pik_ptr = nullptr;
    m->carry_info->spot_info[pik_ptr->temp_i].state = CARRY_SPOT_STATE_FREE;
    m->carry_info->cur_carrying_strength -= pik_ptr->pik_type->carry_strength;
    m->carry_info->cur_n_carriers--;
    
    m->chase_info.max_speed = m->carry_info->get_speed();
    m->chase_info.acceleration = MOB::CARRIED_MOB_ACCELERATION;
    
    m->calculate_carrying_destination(
        nullptr, pik_ptr,
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
            has_flag(m->path_info->settings.flags, PATH_FOLLOW_FLAG_AIRBORNE);
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
                m->path_info->settings.invulnerabilities
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


/**
 * @brief Generic handler for when a mob was delivered to an Onion/ship.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::handle_delivery(mob* m, void* info1, void* info2) {
    engine_assert(m->focused_mob != nullptr, m->print_state_history());
    
    m->focused_mob->fsm.run_event(
        MOB_EV_FINISHED_RECEIVING_DELIVERY, (void*) m
    );
    
    m->to_delete = true;
}


/**
 * @brief When a mob has to lose its momentum.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::lose_momentum(mob* m, void* info1, void* info2) {
    m->speed.x = m->speed.y = 0.0f;
}


/**
 * @brief When a mob starts the process of being delivered to an Onion/ship.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::start_being_delivered(mob* m, void* info1, void* info2) {
    for(size_t p = 0; p < m->carry_info->spot_info.size(); p++) {
        mob* pik_ptr = m->carry_info->spot_info[p].pik_ptr;
        if(pik_ptr) {
            pik_ptr->fsm.run_event(MOB_EV_FINISHED_CARRYING);
        }
    }
    
    m->focus_on_mob(m->carry_info->intended_mob);
    enable_flag(m->flags, MOB_FLAG_INTANGIBLE);
    m->become_uncarriable();
    
    m->focused_mob->fsm.run_event(MOB_EV_STARTED_RECEIVING_DELIVERY, m);
    
    switch(m->delivery_info->anim_type) {
    case DELIVERY_ANIM_SUCK: {
        m->set_timer(MOB::DELIVERY_SUCK_TIME);
        break;
    }
    case DELIVERY_ANIM_TOSS: {
        m->set_timer(MOB::DELIVERY_TOSS_TIME);
        break;
    }
    }
}


/**
 * @brief Generic handler for a mob touching a hazard.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::touch_hazard(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    hazard* h = (hazard*) info1;
    
    for(size_t e = 0; e < h->effects.size(); e++) {
        m->apply_status_effect(h->effects[e], false, true);
    }
}


/**
 * @brief Generic handler for a mob touching a spray.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::touch_spray(mob* m, void* info1, void* info2) {
    engine_assert(info1 != nullptr, m->print_state_history());
    
    spray_type* s = (spray_type*) info1;
    
    for(size_t e = 0; e < s->effects.size(); e++) {
        m->apply_status_effect(s->effects[e], false, false);
    }
}
