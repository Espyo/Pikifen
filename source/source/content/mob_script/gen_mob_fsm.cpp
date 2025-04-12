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
    engineAssert(m->carryInfo != nullptr, m->printStateHistory());
    
    m->circleAround(
        nullptr, m->pos, MOB::CARRY_STUCK_CIRCLING_RADIUS, true,
        m->carryInfo->getSpeed() * MOB::CARRY_STUCK_SPEED_MULTIPLIER,
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
    m->carryInfo->isMoving = true;
    
    hasFlag(m->pathInfo->settings.flags, PATH_FOLLOW_FLAG_AIRBORNE) ?
    enableFlag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR) :
    disableFlag(m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    
    if(!m->carryInfo->destinationExists) {
        m->pathInfo->result = PATH_RESULT_NO_DESTINATION;
    }
    
    if(m->pathInfo->result < 0) {
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
    
    if(m->carryInfo->destination == CARRY_DESTINATION_SHIP) {
        //Special case: ships.
        //Because the ship's control point can be offset, and because
        //the ship is normally in the way, let's consider a
        //"reached destination" event if the treasure is
        //covering the control point, and not necessarily if the treasure
        //is on the same coordinates as the control point.
        if(m->carryInfo->intendedMob) {
            Ship* shi_ptr = (Ship*) m->carryInfo->intendedMob;
            settings.finalTargetDistance =
                std::max(
                    m->radius -
                    shi_ptr->shiType->controlPointRadius,
                    3.0f
                );
        }
        
    } else if(m->carryInfo->destination == CARRY_DESTINATION_LINKED_MOB) {
        //Special case: bridges.
        //Pikmin are meant to carry to the current tip of the bridge,
        //but whereas the start of the bridge is on firm ground, the tip may
        //be above a chasm or water, so the Pikmin might want to take a
        //different path, or be unable to take a path at all.
        //Let's fake the end point to be the start of the bridge,
        //for the sake of path calculations.
        if(
            m->carryInfo->intendedMob &&
            m->carryInfo->intendedMob->type->category->id ==
            MOB_CATEGORY_BRIDGES
        ) {
            Bridge* bri_ptr = (Bridge*) m->carryInfo->intendedMob;
            enableFlag(settings.flags, PATH_FOLLOW_FLAG_FAKED_END);
            enableFlag(settings.flags, PATH_FOLLOW_FLAG_FOLLOW_MOB);
            settings.fakedEnd = bri_ptr->getStartPoint();
        }
    }
    
    settings.targetPoint = m->carryInfo->intendedPoint;
    settings.targetMob = m->carryInfo->intendedMob;
    
    m->followPath(
        settings, m->carryInfo->getSpeed(), m->chaseInfo.acceleration
    );
    
    if(!m->carryInfo->destinationExists) {
        m->pathInfo->result = PATH_RESULT_NO_DESTINATION;
    }
    if(m->pathInfo->result < 0) {
        m->pathInfo->block_reason = PATH_BLOCK_REASON_NO_PATH;
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
    
    if(m->deliveryInfo) {
        delete m->deliveryInfo;
    }
    m->deliveryInfo = new DeliveryInfo();
    if(m->carryInfo->intendedPikType) {
        m->deliveryInfo->color = m->carryInfo->intendedPikType->mainColor;
        m->deliveryInfo->intendedPikType = m->carryInfo->intendedPikType;
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
    if(!m->carryInfo) return;
    m->carryInfo->isMoving = false;
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
    m->toDelete = true;
}


/**
 * @brief Event handler that makes a mob die.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void gen_mob_fsm::goToDyingState(Mob* m, void* info1, void* info2) {
    if(m->type->dyingStateIdx == INVALID) return;
    m->fsm.setState(m->type->dyingStateIdx, info1, info2);
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
    bool could_move = m->carryInfo->curCarryingStrength >= m->type->weight;
    Mob* prev_destination = m->carryInfo->intendedMob;
    
    //Update the numbers and such.
    m->carryInfo->spotInfo[pik_ptr->tempI].pikPtr = pik_ptr;
    m->carryInfo->spotInfo[pik_ptr->tempI].state = CARRY_SPOT_STATE_USED;
    m->carryInfo->curCarryingStrength += pik_ptr->pikType->carryStrength;
    m->carryInfo->curNCarriers++;
    
    m->chaseInfo.maxSpeed = m->carryInfo->getSpeed();
    m->chaseInfo.acceleration = MOB::CARRIED_MOB_ACCELERATION;
    
    m->carryInfo->destinationExists =
        m->calculateCarryingDestination(
            pik_ptr, nullptr,
            &m->carryInfo->intendedPikType,
            &m->carryInfo->intendedMob, &m->carryInfo->intendedPoint
        );
        
    //Check if we need to update the path.
    bool must_update = false;
    
    //Start by checking if the mob can now start moving,
    //or if it already could and the target changed.
    bool can_move = m->carryInfo->curCarryingStrength >= m->type->weight;
    if(can_move) {
        if(
            !could_move ||
            (prev_destination != m->carryInfo->intendedMob)
        ) {
            must_update = true;
        }
    }
    
    //Now, check if the fact that it can fly or not changed.
    if(!must_update && m->pathInfo) {
        bool old_is_airborne =
            hasFlag(m->pathInfo->settings.flags, PATH_FOLLOW_FLAG_AIRBORNE);
        bool new_is_airborne = m->carryInfo->canFly();
        must_update = old_is_airborne != new_is_airborne;
    }
    
    //Check if the list of invulnerabilities changed.
    if(!must_update && m->pathInfo) {
        vector<Hazard*> new_invulnerabilities =
            m->carryInfo->getCarrierInvulnerabilities();
            
        if(
            !vectorsContainSame(
                new_invulnerabilities,
                m->pathInfo->settings.invulnerabilities
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
    bool could_move = m->carryInfo->curCarryingStrength >= m->type->weight;
    Mob* prev_destination = m->carryInfo->intendedMob;
    
    //Update the numbers and such.
    m->carryInfo->spotInfo[pik_ptr->tempI].pikPtr = nullptr;
    m->carryInfo->spotInfo[pik_ptr->tempI].state = CARRY_SPOT_STATE_FREE;
    m->carryInfo->curCarryingStrength -= pik_ptr->pikType->carryStrength;
    m->carryInfo->curNCarriers--;
    
    m->chaseInfo.maxSpeed = m->carryInfo->getSpeed();
    m->chaseInfo.acceleration = MOB::CARRIED_MOB_ACCELERATION;
    
    m->calculateCarryingDestination(
        nullptr, pik_ptr,
        &m->carryInfo->intendedPikType,
        &m->carryInfo->intendedMob, &m->carryInfo->intendedPoint
    );
    
    bool can_move = m->carryInfo->curCarryingStrength >= m->type->weight;
    
    //If the mob can no longer move, send a move stop event,
    //so the mob, well, stops.
    if(could_move && !can_move) {
        m->fsm.runEvent(MOB_EV_CARRY_STOP_MOVE);
        return;
    }
    
    //Check if we need to update the path.
    bool must_update = false;
    
    //Start by checking if the target changed.
    if(can_move && (prev_destination != m->carryInfo->intendedMob)) {
        must_update = true;
    }
    
    //Now, check if the fact that it can fly or not changed.
    if(!must_update && m->pathInfo) {
        bool old_is_airborne =
            hasFlag(m->pathInfo->settings.flags, PATH_FOLLOW_FLAG_AIRBORNE);
        bool new_is_airborne = m->carryInfo->canFly();
        must_update = old_is_airborne != new_is_airborne;
    }
    
    //Check if the list of invulnerabilities changed.
    if(!must_update && m->pathInfo) {
        vector<Hazard*> new_invulnerabilities =
            m->carryInfo->getCarrierInvulnerabilities();
            
        if(
            !vectorsContainSame(
                new_invulnerabilities,
                m->pathInfo->settings.invulnerabilities
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
    engineAssert(m->focusedMob != nullptr, m->printStateHistory());
    
    m->focusedMob->fsm.runEvent(
        MOB_EV_FINISHED_RECEIVING_DELIVERY, (void*) m
    );
    
    m->toDelete = true;
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
    for(size_t p = 0; p < m->carryInfo->spotInfo.size(); p++) {
        Mob* pik_ptr = m->carryInfo->spotInfo[p].pikPtr;
        if(pik_ptr) {
            pik_ptr->fsm.runEvent(MOB_EV_FINISHED_CARRYING);
        }
    }
    
    m->focusOnMob(m->carryInfo->intendedMob);
    enableFlag(m->flags, MOB_FLAG_INTANGIBLE);
    m->becomeUncarriable();
    
    m->focusedMob->fsm.runEvent(MOB_EV_STARTED_RECEIVING_DELIVERY, m);
    
    switch(m->deliveryInfo->animType) {
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
