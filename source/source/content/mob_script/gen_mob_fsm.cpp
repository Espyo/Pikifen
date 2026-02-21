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
void GenMobFsm::beAttacked(Fsm* fsm, void* info1, void* info2) {
    engineAssert(info1 != nullptr, fsm->printStateHistory());
    
    HitboxInteraction* info = (HitboxInteraction*) info1;
    float offenseMultiplier = 1.0f;
    float defenseMultiplier = 1.0f;
    float damage = 0;
    
    if(
        !info->mob2->calculateAttackBasics(
            fsm->m, info->h2, info->h1,
            &offenseMultiplier, &defenseMultiplier
        )
    ) {
        return;
    }
    if(
        !info->mob2->calculateAttackDamage(
            fsm->m, info->h2, info->h1,
            offenseMultiplier, defenseMultiplier, &damage
        )
    ) {
        return;
    }
    
    fsm->m->applyAttackDamage(info->mob2, info->h2, info->h1, damage);
    fsm->m->doAttackEffects(info->mob2, info->h2, info->h1, damage, 0.0f);
}


/**
 * @brief When it's time to become stuck and move in circles.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::carryBecomeStuck(Fsm* fsm, void* info1, void* info2) {
    engineAssert(fsm->m->carryInfo != nullptr, fsm->printStateHistory());
    
    fsm->m->circleAround(
        nullptr, fsm->m->pos, MOB::CARRY_STUCK_CIRCLING_RADIUS, true,
        fsm->m->carryInfo->getSpeed() * MOB::CARRY_STUCK_SPEED_MULTIPLIER,
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
void GenMobFsm::carryBeginMove(Fsm* fsm, void* info1, void* info2) {
    fsm->m->carryInfo->isMoving = true;
    
    hasFlag(fsm->m->pathInfo->settings.flags, PATH_FOLLOW_FLAG_AIRBORNE) ?
    enableFlag(fsm->m->flags, MOB_FLAG_CAN_MOVE_MIDAIR) :
    disableFlag(fsm->m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    
    if(!fsm->m->carryInfo->destinationExists) {
        fsm->m->pathInfo->result = PATH_RESULT_NO_DESTINATION;
    }
    
    if(fsm->m->pathInfo->result < 0) {
        fsm->runEvent(MOB_EV_PATH_BLOCKED);
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
void GenMobFsm::carryGetPath(Fsm* fsm, void* info1, void* info2) {
    PathFollowSettings settings;
    enableFlag(settings.flags, PATH_FOLLOW_FLAG_CAN_CONTINUE);
    
    if(fsm->m->carryInfo->destination == CARRY_DESTINATION_SHIP) {
        //Special case: ships.
        //Because the ship's control point can be offset, and because
        //the ship is normally in the way, let's consider a
        //"reached destination" event if the treasure is
        //covering the control point, and not necessarily if the treasure
        //is on the same coordinates as the control point.
        if(fsm->m->carryInfo->intendedMob) {
            Ship* shiPtr = (Ship*) fsm->m->carryInfo->intendedMob;
            settings.finalTargetDistance =
                std::max(
                    fsm->m->radius - shiPtr->shiType->controlPointRadius,
                    3.0f
                );
        }
        
    } else if(fsm->m->carryInfo->destination == CARRY_DESTINATION_ONION) {
        //Special case: Onions.
        //Like ships, Onions can have their delivery area larger than a
        //single point.
        if(fsm->m->carryInfo->intendedMob) {
            Onion* oniPtr = (Onion*) fsm->m->carryInfo->intendedMob;
            if(oniPtr->oniType->deliveryAreaRadius != 0.0f) {
                settings.finalTargetDistance =
                    fsm->m->radius + oniPtr->oniType->deliveryAreaRadius;
            }
        }
        
    } else if(fsm->m->carryInfo->destination == CARRY_DESTINATION_LINKED_MOB) {
        //Special case: bridges.
        //Pikmin are meant to carry to the current tip of the bridge,
        //but whereas the start of the bridge is on firm ground, the tip may
        //be above a chasm or water, so the Pikmin might want to take a
        //different path, or be unable to take a path at all.
        //Let's fake the end point to be the start of the bridge,
        //for the sake of path calculations.
        if(
            fsm->m->carryInfo->intendedMob &&
            fsm->m->carryInfo->intendedMob->type->category->id ==
            MOB_CATEGORY_BRIDGES
        ) {
            Bridge* briPtr = (Bridge*) fsm->m->carryInfo->intendedMob;
            enableFlag(settings.flags, PATH_FOLLOW_FLAG_FAKED_END);
            enableFlag(settings.flags, PATH_FOLLOW_FLAG_FOLLOW_MOB);
            settings.fakedEnd = briPtr->getStartPoint();
        }
    }
    
    settings.targetPoint = fsm->m->carryInfo->intendedPoint;
    settings.targetMob = fsm->m->carryInfo->intendedMob;
    
    fsm->m->followPath(
        settings, fsm->m->carryInfo->getSpeed(), fsm->m->chaseInfo.acceleration
    );
    
    if(!fsm->m->carryInfo->destinationExists) {
        fsm->m->pathInfo->result = PATH_RESULT_NO_DESTINATION;
    }
    if(fsm->m->pathInfo->result < 0) {
        fsm->m->pathInfo->blockReason = PATH_BLOCK_REASON_NO_PATH;
        fsm->runEvent(MOB_EV_PATH_BLOCKED);
    }
}


/**
 * @brief When a mob reaches the destination or an obstacle when being carried.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::carryReachDestination(Fsm* fsm, void* info1, void* info2) {
    fsm->m->stopFollowingPath();
    
    if(fsm->m->deliveryInfo) {
        delete fsm->m->deliveryInfo;
    }
    fsm->m->deliveryInfo = new DeliveryInfo();
    if(fsm->m->carryInfo->intendedPikType) {
        fsm->m->deliveryInfo->color = fsm->m->carryInfo->intendedPikType->mainColor;
        fsm->m->deliveryInfo->intendedPikType = fsm->m->carryInfo->intendedPikType;
    }
    fsm->m->deliveryInfo->playerTeamIdx = fsm->m->carryInfo->getPlayerTeamIdx();
    fsm->m->deliveryInfo->finalPoint = fsm->m->carryInfo->intendedPoint;
    
    fsm->runEvent(MOB_EV_CARRY_DELIVERED);
}


/**
 * @brief When a mob is no longer stuck waiting to be carried.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::carryStopBeingStuck(Fsm* fsm, void* info1, void* info2) {
    fsm->m->stopCircling();
}


/**
 * @brief When a carried object stops moving.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::carryStopMove(Fsm* fsm, void* info1, void* info2) {
    if(!fsm->m->carryInfo) return;
    fsm->m->carryInfo->isMoving = false;
    disableFlag(fsm->m->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    fsm->m->stopFollowingPath();
    fsm->m->stopChasing();
}


/**
 * @brief Event handler that makes a mob fall into a pit and vanish.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::fallDownPit(Fsm* fsm, void* info1, void* info2) {
    fsm->m->setHealth(false, false, 0);
    fsm->m->startDying();
    fsm->m->finishDying();
    fsm->m->toDelete = true;
}


/**
 * @brief Event handler that makes a mob die.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::goToDyingState(Fsm* fsm, void* info1, void* info2) {
    if(fsm->m->type->dyingStateIdx == INVALID) return;
    fsm->setState(fsm->m->type->dyingStateIdx, info1, info2);
}


/**
 * @brief Event handler for a Pikmin being added as a carrier.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::handleCarrierAdded(Fsm* fsm, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) info1;
    
    //Save some data before changing anything.
    bool couldMove = fsm->m->carryInfo->curCarryingStrength >= fsm->m->type->weight;
    Mob* prevDestination = fsm->m->carryInfo->intendedMob;
    
    //Update the numbers and such.
    fsm->m->carryInfo->spotInfo[pikPtr->tempI].pikPtr = pikPtr;
    fsm->m->carryInfo->spotInfo[pikPtr->tempI].state = CARRY_SPOT_STATE_USED;
    fsm->m->carryInfo->curCarryingStrength += pikPtr->pikType->carryStrength;
    fsm->m->carryInfo->curNCarriers++;
    
    fsm->m->chaseInfo.maxSpeed = fsm->m->carryInfo->getSpeed();
    fsm->m->chaseInfo.acceleration = MOB::CARRIED_MOB_ACCELERATION;
    
    bool canMove = fsm->m->carryInfo->curCarryingStrength >= fsm->m->type->weight;
    
    if(!canMove) {
        return;
    }
    
    fsm->m->carryInfo->destinationExists =
        fsm->m->calculateCarryingDestination(
            &fsm->m->carryInfo->intendedPikType,
            &fsm->m->carryInfo->intendedMob, &fsm->m->carryInfo->intendedPoint
        );
        
    //Check if we need to update the path.
    bool mustUpdate = false;
    
    //Start by checking if the mob can now start moving,
    //or if it already could and the target changed.
    if(canMove) {
        if(
            !couldMove ||
            (prevDestination != fsm->m->carryInfo->intendedMob)
        ) {
            mustUpdate = true;
        }
    }
    
    //Now, check if the fact that it can fly or not changed.
    if(!mustUpdate && fsm->m->pathInfo) {
        bool oldIsAirborne =
            hasFlag(fsm->m->pathInfo->settings.flags, PATH_FOLLOW_FLAG_AIRBORNE);
        bool newIsAirborne = fsm->m->carryInfo->canFly();
        mustUpdate = oldIsAirborne != newIsAirborne;
    }
    
    //Check if the list of invulnerabilities changed.
    if(!mustUpdate && fsm->m->pathInfo) {
        vector<Hazard*> newInvulnerabilities =
            fsm->m->carryInfo->getCarrierInvulnerabilities();
            
        if(
            !isPermutation(
                newInvulnerabilities,
                fsm->m->pathInfo->settings.invulnerabilities
            )
        ) {
            mustUpdate = true;
        }
    }
    
    if(mustUpdate) {
        //Send a move begin event, so that the mob can calculate
        //a (new) path and start taking it.
        fsm->runEvent(MOB_EV_CARRY_BEGIN_MOVE);
    }
}


/**
 * @brief Event handler for a carrier Pikmin being removed.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::handleCarrierRemoved(Fsm* fsm, void* info1, void* info2) {
    Pikmin* pikPtr = (Pikmin*) info1;
    
    //Save some data before changing anything.
    bool couldMove = fsm->m->carryInfo->curCarryingStrength >= fsm->m->type->weight;
    Mob* prevDestination = fsm->m->carryInfo->intendedMob;
    
    //Update the numbers and such.
    fsm->m->carryInfo->spotInfo[pikPtr->tempI].pikPtr = nullptr;
    fsm->m->carryInfo->spotInfo[pikPtr->tempI].state = CARRY_SPOT_STATE_FREE;
    fsm->m->carryInfo->curCarryingStrength -= pikPtr->pikType->carryStrength;
    fsm->m->carryInfo->curNCarriers--;
    
    fsm->m->chaseInfo.maxSpeed = fsm->m->carryInfo->getSpeed();
    fsm->m->chaseInfo.acceleration = MOB::CARRIED_MOB_ACCELERATION;
    
    bool canMove = fsm->m->carryInfo->curCarryingStrength >= fsm->m->type->weight;
    
    if(!canMove) {
        if(couldMove) {
            //If the mob can no longer move, send a move stop event,
            //so the mob, well, stops.
            fsm->runEvent(MOB_EV_CARRY_STOP_MOVE);
        }
        return;
    }
    
    fsm->m->calculateCarryingDestination(
        &fsm->m->carryInfo->intendedPikType,
        &fsm->m->carryInfo->intendedMob, &fsm->m->carryInfo->intendedPoint
    );
    
    //Check if we need to update the path.
    bool mustUpdate = false;
    
    //Start by checking if the target changed.
    if(canMove && (prevDestination != fsm->m->carryInfo->intendedMob)) {
        mustUpdate = true;
    }
    
    //Now, check if the fact that it can fly or not changed.
    if(!mustUpdate && fsm->m->pathInfo) {
        bool oldIsAirborne =
            hasFlag(fsm->m->pathInfo->settings.flags, PATH_FOLLOW_FLAG_AIRBORNE);
        bool newIsAirborne = fsm->m->carryInfo->canFly();
        mustUpdate = oldIsAirborne != newIsAirborne;
    }
    
    //Check if the list of invulnerabilities changed.
    if(!mustUpdate && fsm->m->pathInfo) {
        vector<Hazard*> newInvulnerabilities =
            fsm->m->carryInfo->getCarrierInvulnerabilities();
            
        if(
            !isPermutation(
                newInvulnerabilities,
                fsm->m->pathInfo->settings.invulnerabilities
            )
        ) {
            mustUpdate = true;
        }
    }
    
    if(mustUpdate) {
        //Send a move begin event, so that the mob can calculate
        //a (new) path and start taking it.
        fsm->runEvent(MOB_EV_CARRY_BEGIN_MOVE);
    }
}


/**
 * @brief Generic handler for when a mob was delivered to an Onion/ship.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::handleDelivery(Fsm* fsm, void* info1, void* info2) {
    if(fsm->m->focusedMob) {
        fsm->m->focusedMob->fsm.runEvent(
            MOB_EV_FINISHED_RECEIVING_DELIVERY, (void*) fsm->m
        );
    }
    
    if(game.curArea->type == AREA_TYPE_MISSION) {
        for(
            size_t c = 0;
            c < game.states.gameplay->missionMobChecklists.size(); c++
        ) {
            game.states.gameplay->missionMobChecklists[c].remove(fsm->m);
        }
    }
    fsm->m->toDelete = true;
}


/**
 * @brief When a mob has to lose its momentum.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::loseMomentum(Fsm* fsm, void* info1, void* info2) {
    fsm->m->speed.x = fsm->m->speed.y = 0.0f;
}


/**
 * @brief When a mob starts the process of being delivered to an Onion/ship.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::startBeingDelivered(Fsm* fsm, void* info1, void* info2) {
    for(size_t p = 0; p < fsm->m->carryInfo->spotInfo.size(); p++) {
        Mob* pikPtr = fsm->m->carryInfo->spotInfo[p].pikPtr;
        if(pikPtr) {
            pikPtr->fsm.runEvent(MOB_EV_FINISHED_TASK);
        }
    }
    
    if(fsm->m->carryInfo->intendedMob->type->category->id == MOB_CATEGORY_ONIONS) {
        Onion* oniPtr = (Onion*) fsm->m->carryInfo->intendedMob;
        fsm->m->deliveryInfo->animType = oniPtr->oniType->deliveryAnim;
    }
    
    fsm->m->focusOnMob(fsm->m->carryInfo->intendedMob);
    enableFlag(fsm->m->flags, MOB_FLAG_INTANGIBLE);
    fsm->m->becomeUncarriable();
    
    fsm->m->focusedMob->fsm.runEvent(MOB_EV_STARTED_RECEIVING_DELIVERY, fsm->m);
    
    switch(fsm->m->deliveryInfo->animType) {
    case DELIVERY_ANIM_SUCK: {
        fsm->m->setTimer(MOB::DELIVERY_SUCK_TIME);
        break;
    }
    case DELIVERY_ANIM_TOSS: {
        fsm->m->setTimer(MOB::DELIVERY_TOSS_TIME);
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
void GenMobFsm::touchHazard(Fsm* fsm, void* info1, void* info2) {
    engineAssert(info1 != nullptr, fsm->printStateHistory());
    
    Hazard* h = (Hazard*) info1;
    
    for(size_t e = 0; e < h->effects.size(); e++) {
        fsm->m->applyStatus(h->effects[e], false, true);
    }
}


/**
 * @brief Generic handler for a mob touching a spray.
 *
 * @param m The mob.
 * @param info1 Pointer to the spray type.
 * @param info2 Pointer to the mob that sprayed, if any.
 */
void GenMobFsm::touchSpray(Fsm* fsm, void* info1, void* info2) {
    engineAssert(info1 != nullptr, fsm->printStateHistory());
    
    SprayType* s = (SprayType*) info1;
    Mob* sprayer = (Mob*) info2;
    
    for(size_t e = 0; e < s->effects.size(); e++) {
        fsm->m->applyStatus(s->effects[e], false, false, sprayer);
    }
}
