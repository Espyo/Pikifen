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

#include "../../core/const.h"
#include "../../core/misc_functions.h"
#include "../../util/general_utils.h"
#include "../mob/bridge.h"
#include "../mob/enemy.h"
#include "../mob/onion.h"
#include "../mob/pikmin.h"
#include "../mob/ship.h"
#include "../other/spray_type.h"


/**
 * @brief Event handler that makes a mob lose health by being damaged
 * by another.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::beAttacked(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    HitboxInteraction* info = (HitboxInteraction*) info1;
    
    float damage = 0;
    if(!info->mob2->calculateDamage(m, info->h2, info->h1, &damage)) {
        return;
    }
    
    m->applyAttackDamage(info->mob2, info->h2, info->h1, damage);
    m->doAttackEffects(info->mob2, info->h2, info->h1, damage, 0.0f);
}


/**
 * @brief When it's time to become stuck and move in circles.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::carryBecomeStuck(Mob* m, void* info1, void* info2) {
    engineAssert(m->carry_info != nullptr, m->printStateHistory());
    
    m->circleAround(
        nullptr, m->pos, MOB::CARRY_STUCK_CIRCLING_RADIUS, true,
        m->carry_info->getSpeed() * MOB::CARRY_STUCK_SPEED_MULTIPLIER,
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
void gen_mob_fsm::carryBeginMove(Mob* m, void* info1, void* info2) {
    m->carry_info->is_moving = true;
    
    hasFlag(m->path_info->settings.flags, PATH_FOLLOW_FLAG_AIRBORNE) ?
    enableFlag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR) :
    disableFlag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    
    if(!m->carry_info->destination_exists) {
        m->path_info->result = PATH_RESULT_NO_DESTINATION;
    }
    
    if(m->path_info->result < 0) {
        m->fsm.runEvent(MOB_EV_PATH_BLOCKED);
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
void gen_mob_fsm::carryGetPath(Mob* m, void* info1, void* info2) {
    PathFollowSettings settings;
    enableFlag(settings.flags, PATH_FOLLOW_FLAG_CAN_CONTINUE);
    
    if(m->carry_info->destination == CARRY_DESTINATION_SHIP) {
        //Special case: ships.
        //Because the ship's control point can be offset, and because
        //the ship is normally in the way, let's consider a
        //"reached destination" event if the treasure is
        //covering the control point, and not necessarily if the treasure
        //is on the same coordinates as the control point.
        if(m->carry_info->intended_mob) {
            Ship* shi_ptr = (Ship*) m->carry_info->intended_mob;
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
            Bridge* bri_ptr = (Bridge*) m->carry_info->intended_mob;
            enableFlag(settings.flags, PATH_FOLLOW_FLAG_FAKED_END);
            enableFlag(settings.flags, PATH_FOLLOW_FLAG_FOLLOW_MOB);
            settings.faked_end = bri_ptr->getStartPoint();
        }
    }
    
    settings.target_point = m->carry_info->intended_point;
    settings.target_mob = m->carry_info->intended_mob;
    
    m->followPath(
        settings, m->carry_info->getSpeed(), m->chase_info.acceleration
    );
    
    if(!m->carry_info->destination_exists) {
        m->path_info->result = PATH_RESULT_NO_DESTINATION;
    }
    if(m->path_info->result < 0) {
        m->path_info->block_reason = PATH_BLOCK_REASON_NO_PATH;
        m->fsm.runEvent(MOB_EV_PATH_BLOCKED);
    }
}


/**
 * @brief When a mob reaches the destination or an obstacle when being carried.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::carryReachDestination(Mob* m, void* info1, void* info2) {
    m->stopFollowingPath();
    
    if(m->delivery_info) {
        delete m->delivery_info;
    }
    m->delivery_info = new DeliveryInfo();
    if(m->carry_info->intended_pik_type) {
        m->delivery_info->color = m->carry_info->intended_pik_type->main_color;
        m->delivery_info->intended_pik_type = m->carry_info->intended_pik_type;
    }
    
    m->fsm.runEvent(MOB_EV_CARRY_DELIVERED);
}


/**
 * @brief When a mob is no longer stuck waiting to be carried.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::carryStopBeingStuck(Mob* m, void* info1, void* info2) {
    m->stopCircling();
}


/**
 * @brief When a carried object stops moving.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::carryStopMove(Mob* m, void* info1, void* info2) {
    if(!m->carry_info) return;
    m->carry_info->is_moving = false;
    disableFlag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    m->stopFollowingPath();
    m->stopChasing();
}


/**
 * @brief Event handler that makes a mob fall into a pit and vanish.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::fallDownPit(Mob* m, void* info1, void* info2) {
    m->setHealth(false, false, 0);
    m->startDying();
    m->finishDying();
    m->to_delete = true;
}


/**
 * @brief Event handler that makes a mob die.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::goToDyingState(Mob* m, void* info1, void* info2) {
    if(m->type->dying_state_idx == INVALID) return;
    m->fsm.setState(m->type->dying_state_idx, info1, info2);
}


/**
 * @brief Event handler for a Pikmin being added as a carrier.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::handleCarrierAdded(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) info1;
    
    //Save some data before changing anything.
    bool could_move = m->carry_info->cur_carrying_strength >= m->type->weight;
    Mob* prev_destination = m->carry_info->intended_mob;
    
    //Update the numbers and such.
    m->carry_info->spot_info[pik_ptr->temp_i].pik_ptr = pik_ptr;
    m->carry_info->spot_info[pik_ptr->temp_i].state = CARRY_SPOT_STATE_USED;
    m->carry_info->cur_carrying_strength += pik_ptr->pik_type->carry_strength;
    m->carry_info->cur_n_carriers++;
    
    m->chase_info.max_speed = m->carry_info->getSpeed();
    m->chase_info.acceleration = MOB::CARRIED_MOB_ACCELERATION;
    
    m->carry_info->destination_exists =
        m->calculateCarryingDestination(
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
            hasFlag(m->path_info->settings.flags, PATH_FOLLOW_FLAG_AIRBORNE);
        bool new_is_airborne = m->carry_info->canFly();
        must_update = old_is_airborne != new_is_airborne;
    }
    
    //Check if the list of invulnerabilities changed.
    if(!must_update && m->path_info) {
        vector<Hazard*> new_invulnerabilities =
            m->carry_info->getCarrierInvulnerabilities();
            
        if(
            !vectorsContainSame(
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
        m->fsm.runEvent(MOB_EV_CARRY_BEGIN_MOVE);
    }
}


/**
 * @brief Event handler for a carrier Pikmin being removed.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::handleCarrierRemoved(Mob* m, void* info1, void* info2) {
    Pikmin* pik_ptr = (Pikmin*) info1;
    
    //Save some data before changing anything.
    bool could_move = m->carry_info->cur_carrying_strength >= m->type->weight;
    Mob* prev_destination = m->carry_info->intended_mob;
    
    //Update the numbers and such.
    m->carry_info->spot_info[pik_ptr->temp_i].pik_ptr = nullptr;
    m->carry_info->spot_info[pik_ptr->temp_i].state = CARRY_SPOT_STATE_FREE;
    m->carry_info->cur_carrying_strength -= pik_ptr->pik_type->carry_strength;
    m->carry_info->cur_n_carriers--;
    
    m->chase_info.max_speed = m->carry_info->getSpeed();
    m->chase_info.acceleration = MOB::CARRIED_MOB_ACCELERATION;
    
    m->calculateCarryingDestination(
        nullptr, pik_ptr,
        &m->carry_info->intended_pik_type,
        &m->carry_info->intended_mob, &m->carry_info->intended_point
    );
    
    bool can_move = m->carry_info->cur_carrying_strength >= m->type->weight;
    
    //If the mob can no longer move, send a move stop event,
    //so the mob, well, stops.
    if(could_move && !can_move) {
        m->fsm.runEvent(MOB_EV_CARRY_STOP_MOVE);
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
            hasFlag(m->path_info->settings.flags, PATH_FOLLOW_FLAG_AIRBORNE);
        bool new_is_airborne = m->carry_info->canFly();
        must_update = old_is_airborne != new_is_airborne;
    }
    
    //Check if the list of invulnerabilities changed.
    if(!must_update && m->path_info) {
        vector<Hazard*> new_invulnerabilities =
            m->carry_info->getCarrierInvulnerabilities();
            
        if(
            !vectorsContainSame(
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
        m->fsm.runEvent(MOB_EV_CARRY_BEGIN_MOVE);
    }
}


/**
 * @brief Generic handler for when a mob was delivered to an Onion/ship.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::handleDelivery(Mob* m, void* info1, void* info2) {
    engineAssert(m->focused_mob != nullptr, m->printStateHistory());
    
    m->focused_mob->fsm.runEvent(
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
void gen_mob_fsm::loseMomentum(Mob* m, void* info1, void* info2) {
    m->speed.x = m->speed.y = 0.0f;
}


/**
 * @brief When a mob starts the process of being delivered to an Onion/ship.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::startBeingDelivered(Mob* m, void* info1, void* info2) {
    for(size_t p = 0; p < m->carry_info->spot_info.size(); p++) {
        Mob* pik_ptr = m->carry_info->spot_info[p].pik_ptr;
        if(pik_ptr) {
            pik_ptr->fsm.runEvent(MOB_EV_FINISHED_CARRYING);
        }
    }
    
    m->focusOnMob(m->carry_info->intended_mob);
    enableFlag(m->flags, MOB_FLAG_INTANGIBLE);
    m->becomeUncarriable();
    
    m->focused_mob->fsm.runEvent(MOB_EV_STARTED_RECEIVING_DELIVERY, m);
    
    switch(m->delivery_info->anim_type) {
    case DELIVERY_ANIM_SUCK: {
        m->setTimer(MOB::DELIVERY_SUCK_TIME);
        break;
    }
    case DELIVERY_ANIM_TOSS: {
        m->setTimer(MOB::DELIVERY_TOSS_TIME);
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
void gen_mob_fsm::touchHazard(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Hazard* h = (Hazard*) info1;
    
    for(size_t e = 0; e < h->effects.size(); e++) {
        m->applyStatusEffect(h->effects[e], false, true);
    }
}


/**
 * @brief Generic handler for a mob touching a spray.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::touchSpray(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    SprayType* s = (SprayType*) info1;
    
    for(size_t e = 0; e < s->effects.size(); e++) {
        m->applyStatusEffect(s->effects[e], false, false);
    }
}
