/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Generic mob finite-state machine logic.
 */

#include <algorithm>

#include "gen_mob_fsm.h"

#include "../../core/const.h"
#include "../../core/misc_functions.h"
#include "../../util/container_utils.h"
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
void GenMobFsm::beAttacked(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    HitboxInteraction* info = (HitboxInteraction*) info1;
    float offenseMultiplier = 1.0f;
    float defenseMultiplier = 1.0f;
    float damage = 0;
    
    if(
        !info->mob2->calculateAttackBasics(
            m, info->h2, info->h1, &offenseMultiplier, &defenseMultiplier
        )
    ) {
        return;
    }
    if(
        !info->mob2->calculateAttackDamage(
            m, info->h2, info->h1, offenseMultiplier, defenseMultiplier, &damage
        )
    ) {
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
void GenMobFsm::carryBecomeStuck(Mob* m, void* info1, void* info2) {
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
void GenMobFsm::carryBeginMove(Mob* m, void* info1, void* info2) {
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
void GenMobFsm::carryGetPath(Mob* m, void* info1, void* info2) {
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
            Ship* shiPtr = (Ship*) m->carryInfo->intendedMob;
            settings.finalTargetDistance =
                std::max(
                    m->radius - shiPtr->shiType->controlPointRadius,
                    3.0f
                );
        }
        
    } else if(m->carryInfo->destination == CARRY_DESTINATION_ONION) {
        //Special case: Onions.
        //Like ships, Onions can have their delivery area larger than a
        //single point.
        if(m->carryInfo->intendedMob) {
            Onion* oniPtr = (Onion*) m->carryInfo->intendedMob;
            if(oniPtr->oniType->deliveryAreaRadius != 0.0f) {
                settings.finalTargetDistance =
                    m->radius + oniPtr->oniType->deliveryAreaRadius;
            }
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
            Bridge* briPtr = (Bridge*) m->carryInfo->intendedMob;
            enableFlag(settings.flags, PATH_FOLLOW_FLAG_FAKED_END);
            enableFlag(settings.flags, PATH_FOLLOW_FLAG_FOLLOW_MOB);
            settings.fakedEnd = briPtr->getStartPoint();
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
        m->pathInfo->blockReason = PATH_BLOCK_REASON_NO_PATH;
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
void GenMobFsm::carryReachDestination(Mob* m, void* info1, void* info2) {
    m->stopFollowingPath();
    
    if(m->deliveryInfo) {
        delete m->deliveryInfo;
    }
    m->deliveryInfo = new DeliveryInfo();
    if(m->carryInfo->intendedPikType) {
        m->deliveryInfo->color = m->carryInfo->intendedPikType->mainColor;
        m->deliveryInfo->intendedPikType = m->carryInfo->intendedPikType;
    }
    m->deliveryInfo->playerTeamIdx = m->carryInfo->getPlayerTeamIdx();
    m->deliveryInfo->finalPoint = m->carryInfo->intendedPoint;
    
    m->fsm.runEvent(MOB_EV_CARRY_DELIVERED);
}


/**
 * @brief When a mob is no longer stuck waiting to be carried.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::carryStopBeingStuck(Mob* m, void* info1, void* info2) {
    m->stopCircling();
}


/**
 * @brief When a carried object stops moving.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::carryStopMove(Mob* m, void* info1, void* info2) {
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
void GenMobFsm::fallDownPit(Mob* m, void* info1, void* info2) {
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
void GenMobFsm::goToDyingState(Mob* m, void* info1, void* info2) {
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
void GenMobFsm::handleCarrierAdded(Mob* m, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) info1;
    
    //Save some data before changing anything.
    bool couldMove = m->carryInfo->curCarryingStrength >= m->type->weight;
    Mob* prevDestination = m->carryInfo->intendedMob;
    
    //Update the numbers and such.
    m->carryInfo->spotInfo[pikPtr->tempI].pikPtr = pikPtr;
    m->carryInfo->spotInfo[pikPtr->tempI].state = CARRY_SPOT_STATE_USED;
    m->carryInfo->curCarryingStrength += pikPtr->pikType->carryStrength;
    m->carryInfo->curNCarriers++;
    
    m->chaseInfo.maxSpeed = m->carryInfo->getSpeed();
    m->chaseInfo.acceleration = MOB::CARRIED_MOB_ACCELERATION;
    
    bool canMove = m->carryInfo->curCarryingStrength >= m->type->weight;
    
    if(!canMove) {
        return;
    }
    
    m->carryInfo->destinationExists =
        m->calculateCarryingDestination(
            &m->carryInfo->intendedPikType,
            &m->carryInfo->intendedMob, &m->carryInfo->intendedPoint
        );
        
    //Check if we need to update the path.
    bool mustUpdate = false;
    
    //Start by checking if the mob can now start moving,
    //or if it already could and the target changed.
    if(canMove) {
        if(
            !couldMove ||
            (prevDestination != m->carryInfo->intendedMob)
        ) {
            mustUpdate = true;
        }
    }
    
    //Now, check if the fact that it can fly or not changed.
    if(!mustUpdate && m->pathInfo) {
        bool oldIsAirborne =
            hasFlag(m->pathInfo->settings.flags, PATH_FOLLOW_FLAG_AIRBORNE);
        bool newIsAirborne = m->carryInfo->canFly();
        mustUpdate = oldIsAirborne != newIsAirborne;
    }
    
    //Check if the list of invulnerabilities changed.
    if(!mustUpdate && m->pathInfo) {
        vector<Hazard*> newInvulnerabilities =
            m->carryInfo->getCarrierInvulnerabilities();
            
        if(
            !isPermutation(
                newInvulnerabilities,
                m->pathInfo->settings.invulnerabilities
            )
        ) {
            mustUpdate = true;
        }
    }
    
    if(mustUpdate) {
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
void GenMobFsm::handleCarrierRemoved(Mob* m, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) info1;
    
    //Save some data before changing anything.
    bool couldMove = m->carryInfo->curCarryingStrength >= m->type->weight;
    Mob* prevDestination = m->carryInfo->intendedMob;
    
    //Update the numbers and such.
    m->carryInfo->spotInfo[pikPtr->tempI].pikPtr = nullptr;
    m->carryInfo->spotInfo[pikPtr->tempI].state = CARRY_SPOT_STATE_FREE;
    m->carryInfo->curCarryingStrength -= pikPtr->pikType->carryStrength;
    m->carryInfo->curNCarriers--;
    
    m->chaseInfo.maxSpeed = m->carryInfo->getSpeed();
    m->chaseInfo.acceleration = MOB::CARRIED_MOB_ACCELERATION;
    
    bool canMove = m->carryInfo->curCarryingStrength >= m->type->weight;
    
    if(!canMove) {
        if(couldMove) {
            //If the mob can no longer move, send a move stop event,
            //so the mob, well, stops.
            m->fsm.runEvent(MOB_EV_CARRY_STOP_MOVE);
        }
        return;
    }
    
    m->calculateCarryingDestination(
        &m->carryInfo->intendedPikType,
        &m->carryInfo->intendedMob, &m->carryInfo->intendedPoint
    );
    
    //Check if we need to update the path.
    bool mustUpdate = false;
    
    //Start by checking if the target changed.
    if(canMove && (prevDestination != m->carryInfo->intendedMob)) {
        mustUpdate = true;
    }
    
    //Now, check if the fact that it can fly or not changed.
    if(!mustUpdate && m->pathInfo) {
        bool oldIsAirborne =
            hasFlag(m->pathInfo->settings.flags, PATH_FOLLOW_FLAG_AIRBORNE);
        bool newIsAirborne = m->carryInfo->canFly();
        mustUpdate = oldIsAirborne != newIsAirborne;
    }
    
    //Check if the list of invulnerabilities changed.
    if(!mustUpdate && m->pathInfo) {
        vector<Hazard*> newInvulnerabilities =
            m->carryInfo->getCarrierInvulnerabilities();
            
        if(
            !isPermutation(
                newInvulnerabilities,
                m->pathInfo->settings.invulnerabilities
            )
        ) {
            mustUpdate = true;
        }
    }
    
    if(mustUpdate) {
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
void GenMobFsm::handleDelivery(Mob* m, void* info1, void* info2) {
    if(m->focusedMob) {
        m->focusedMob->fsm.runEvent(
            MOB_EV_FINISHED_RECEIVING_DELIVERY, (void*) m
        );
    }
    m->toDelete = true;
}


/**
 * @brief When a mob has to lose its momentum.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::loseMomentum(Mob* m, void* info1, void* info2) {
    m->speed.x = m->speed.y = 0.0f;
}


/**
 * @brief When a mob starts the process of being delivered to an Onion/ship.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::startBeingDelivered(Mob* m, void* info1, void* info2) {
    for(size_t p = 0; p < m->carryInfo->spotInfo.size(); p++) {
        Mob* pikPtr = m->carryInfo->spotInfo[p].pikPtr;
        if(pikPtr) {
            pikPtr->fsm.runEvent(MOB_EV_FINISHED_TASK);
        }
    }
    
    if(m->carryInfo->intendedMob->type->category->id == MOB_CATEGORY_ONIONS) {
        Onion* oniPtr = (Onion*) m->carryInfo->intendedMob;
        m->deliveryInfo->animType = oniPtr->oniType->deliveryAnim;
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
void GenMobFsm::touchHazard(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    Hazard* h = (Hazard*) info1;
    
    for(size_t e = 0; e < h->effects.size(); e++) {
        m->applyStatus(h->effects[e], false, true);
    }
}


/**
 * @brief Generic handler for a mob touching a spray.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::touchSpray(Mob* m, void* info1, void* info2) {
    engineAssert(info1 != nullptr, m->printStateHistory());
    
    SprayType* s = (SprayType*) info1;
    
    for(size_t e = 0; e < s->effects.size(); e++) {
        m->applyStatus(s->effects[e], false, false);
    }
}
