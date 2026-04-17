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
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::beAttacked(ScriptVM* scriptVM, void* info1, void* info2) {
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    HitboxInteraction* info = (HitboxInteraction*) info1;
    float offenseMultiplier = 1.0f;
    float defenseMultiplier = 1.0f;
    float damage = 0;
    
    if(
        !info->mob2->calculateAttackBasics(
            scriptVM->mob, info->h2, info->h1,
            &offenseMultiplier, &defenseMultiplier
        )
    ) {
        return;
    }
    if(
        !info->mob2->calculateAttackDamage(
            scriptVM->mob, info->h2, info->h1,
            offenseMultiplier, defenseMultiplier, &damage
        )
    ) {
        return;
    }
    
    scriptVM->mob->applyAttackDamage(info->mob2, info->h2, info->h1, damage);
    scriptVM->mob->doAttackEffects(
        info->mob2, info->h2, info->h1, damage, 0.0f
    );
}


/**
 * @brief When it's time to become stuck and move in circles.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::carryBecomeStuck(ScriptVM* scriptVM, void* info1, void* info2) {
    engineAssert(
        scriptVM->mob->carryInfo != nullptr, scriptVM->fsm.getStateHistoryStr()
    );
    
    scriptVM->mob->circleAround(
        nullptr, scriptVM->mob->pos, MOB::CARRY_STUCK_CIRCLING_RADIUS, true,
        scriptVM->mob->carryInfo->getSpeed() *
        MOB::CARRY_STUCK_SPEED_MULTIPLIER,
        true
    );
}


/**
 * @brief When it's time to check if a carried object should begin moving,
 * or update its path.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::carryBeginMove(ScriptVM* scriptVM, void* info1, void* info2) {
    scriptVM->mob->carryInfo->isMoving = true;

    bool canFly =
        hasFlag(
            scriptVM->mob->pathInfo->settings.flags, PATH_FOLLOW_FLAG_AIRBORNE
        );
    
    canFly ?
    enableFlag(scriptVM->mob->flags, MOB_FLAG_CAN_MOVE_MIDAIR) :
    disableFlag(scriptVM->mob->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    
    if(!scriptVM->mob->carryInfo->destinationExists) {
        scriptVM->mob->pathInfo->result = PATH_RESULT_NO_DESTINATION;
    }
    
    if(scriptVM->mob->pathInfo->result < 0) {
            scriptVM->fsm.runEvent(FSM_EV_PATH_BLOCKED);
        return;
    }
}


/**
 * @brief When a mob wants a new path.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::carryGetPath(ScriptVM* scriptVM, void* info1, void* info2) {
    PathFollowSettings settings;
    enableFlag(settings.flags, PATH_FOLLOW_FLAG_CAN_CONTINUE);
    
    if(scriptVM->mob->carryInfo->destination == CARRY_DESTINATION_SHIP) {
        //Special case: ships.
        //Because the ship's control point can be offset, and because
        //the ship is normally in the way, let's consider a
        //"reached destination" event if the treasure is
        //covering the control point, and not necessarily if the treasure
        //is on the same coordinates as the control point.
        if(scriptVM->mob->carryInfo->intendedMob) {
            Ship* shiPtr = (Ship*) scriptVM->mob->carryInfo->intendedMob;
            settings.finalTargetDistance =
                std::max(
                    scriptVM->mob->radius - shiPtr->shiType->controlPointRadius,
                    3.0f
                );
        }
        
    } else if(
        scriptVM->mob->carryInfo->destination == CARRY_DESTINATION_ONION
    ) {
        //Special case: Onions.
        //Like ships, Onions can have their delivery area larger than a
        //single point.
        if(scriptVM->mob->carryInfo->intendedMob) {
            Onion* oniPtr = (Onion*) scriptVM->mob->carryInfo->intendedMob;
            if(oniPtr->oniType->deliveryAreaRadius != 0.0f) {
                settings.finalTargetDistance =
                    scriptVM->mob->radius + oniPtr->oniType->deliveryAreaRadius;
            }
        }
        
    } else if(
        scriptVM->mob->carryInfo->destination == CARRY_DESTINATION_LINKED_MOB
    ) {
        //Special case: bridges.
        //Pikmin are meant to carry to the current tip of the bridge,
        //but whereas the start of the bridge is on firm ground, the tip may
        //be above a chasm or water, so the Pikmin might want to take a
        //different path, or be unable to take a path at all.
        //Let's fake the end point to be the start of the bridge,
        //for the sake of path calculations.
        if(
            scriptVM->mob->carryInfo->intendedMob &&
            scriptVM->mob->carryInfo->intendedMob->type->category->id ==
            MOB_CATEGORY_BRIDGES
        ) {
            Bridge* briPtr = (Bridge*) scriptVM->mob->carryInfo->intendedMob;
            enableFlag(settings.flags, PATH_FOLLOW_FLAG_FAKED_END);
            enableFlag(settings.flags, PATH_FOLLOW_FLAG_FOLLOW_MOB);
            settings.fakedEnd = briPtr->getStartPoint();
        }
    }
    
    settings.targetPoint = scriptVM->mob->carryInfo->intendedPoint;
    settings.targetMob = scriptVM->mob->carryInfo->intendedMob;
    
    scriptVM->mob->followPath(
        settings, scriptVM->mob->carryInfo->getSpeed(),
        scriptVM->mob->chaseInfo.acceleration
    );
    
    if(!scriptVM->mob->carryInfo->destinationExists) {
        scriptVM->mob->pathInfo->result = PATH_RESULT_NO_DESTINATION;
    }
    if(scriptVM->mob->pathInfo->result < 0) {
        scriptVM->mob->pathInfo->blockReason = PATH_BLOCK_REASON_NO_PATH;
            scriptVM->fsm.runEvent(FSM_EV_PATH_BLOCKED);
    }
}


/**
 * @brief When a mob reaches the destination or an obstacle when being carried.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::carryReachDestination(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    scriptVM->mob->stopFollowingPath();
    
    if(scriptVM->mob->deliveryInfo) {
        delete scriptVM->mob->deliveryInfo;
    }
    scriptVM->mob->deliveryInfo = new DeliveryInfo();
    if(scriptVM->mob->carryInfo->intendedPikType) {
        scriptVM->mob->deliveryInfo->color =
            scriptVM->mob->carryInfo->intendedPikType->mainColor;
        scriptVM->mob->deliveryInfo->intendedPikType =
            scriptVM->mob->carryInfo->intendedPikType;
    }
    scriptVM->mob->deliveryInfo->playerTeamIdx =
        scriptVM->mob->carryInfo->getPlayerTeamIdx();
    scriptVM->mob->deliveryInfo->finalPoint =
        scriptVM->mob->carryInfo->intendedPoint;
    
        scriptVM->fsm.runEvent(FSM_EV_CARRY_DELIVERED);
}


/**
 * @brief When a mob is no longer stuck waiting to be carried.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::carryStopBeingStuck(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    scriptVM->mob->stopCircling();
}


/**
 * @brief When a carried object stops moving.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::carryStopMove(ScriptVM* scriptVM, void* info1, void* info2) {
    if(!scriptVM->mob->carryInfo) return;
    scriptVM->mob->carryInfo->isMoving = false;
    disableFlag(scriptVM->mob->flags, MOB_FLAG_CAN_MOVE_MIDAIR);
    scriptVM->mob->stopFollowingPath();
    scriptVM->mob->stopChasing();
}


/**
 * @brief Event handler that makes a mob fall into a pit and vanish.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::fallDownPit(ScriptVM* scriptVM, void* info1, void* info2) {
    scriptVM->mob->setHealth(false, false, 0);
    scriptVM->mob->startDying();
    scriptVM->mob->finishDying();
    scriptVM->mob->toDelete = true;
}


/**
 * @brief Event handler that makes a mob die.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::goToDyingState(ScriptVM* scriptVM, void* info1, void* info2) {
    if(scriptVM->mob->type->dyingStateIdx == INVALID) return;
    scriptVM->fsm.setState(scriptVM->mob->type->dyingStateIdx, info1, info2);
}


/**
 * @brief Event handler for a Pikmin being added as a carrier.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::handleCarrierAdded(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) info1;
    
    //Save some data before changing anything.
    bool couldMove =
        scriptVM->mob->carryInfo->curCarryingStrength >=
        scriptVM->mob->type->weight;
    Mob* prevDestination = scriptVM->mob->carryInfo->intendedMob;
    
    //Update the numbers and such.
    scriptVM->mob->carryInfo->spotInfo[pikPtr->tempI].pikPtr = pikPtr;
    scriptVM->mob->carryInfo->spotInfo[pikPtr->tempI].state =
        CARRY_SPOT_STATE_USED;
    scriptVM->mob->carryInfo->curCarryingStrength +=
        pikPtr->pikType->carryStrength;
    scriptVM->mob->carryInfo->curNCarriers++;
    
    scriptVM->mob->chaseInfo.maxSpeed = scriptVM->mob->carryInfo->getSpeed();
    scriptVM->mob->chaseInfo.acceleration = MOB::CARRIED_MOB_ACCELERATION;
    
    bool canMove =
        scriptVM->mob->carryInfo->curCarryingStrength >=
        scriptVM->mob->type->weight;
    
    if(!canMove) {
        return;
    }
    
    scriptVM->mob->carryInfo->destinationExists =
        scriptVM->mob->calculateCarryingDestination(
            &scriptVM->mob->carryInfo->intendedPikType,
            &scriptVM->mob->carryInfo->intendedMob,
            &scriptVM->mob->carryInfo->intendedPoint
        );
        
    //Check if we need to update the path.
    bool mustUpdate = false;
    
    //Start by checking if the mob can now start moving,
    //or if it already could and the target changed.
    if(canMove) {
        if(
            !couldMove ||
            (prevDestination != scriptVM->mob->carryInfo->intendedMob)
        ) {
            mustUpdate = true;
        }
    }
    
    //Now, check if the fact that it can fly or not changed.
    if(!mustUpdate && scriptVM->mob->pathInfo) {
        bool oldIsAirborne =
            hasFlag(
                scriptVM->mob->pathInfo->settings.flags,
                PATH_FOLLOW_FLAG_AIRBORNE
            );
        bool newIsAirborne = scriptVM->mob->carryInfo->canFly();
        mustUpdate = oldIsAirborne != newIsAirborne;
    }
    
    //Check if the list of invulnerabilities changed.
    if(!mustUpdate && scriptVM->mob->pathInfo) {
        vector<Hazard*> newInvulnerabilities =
            scriptVM->mob->carryInfo->getCarrierInvulnerabilities();
            
        if(
            !isPermutation(
                newInvulnerabilities,
                scriptVM->mob->pathInfo->settings.invulnerabilities
            )
        ) {
            mustUpdate = true;
        }
    }
    
    if(mustUpdate) {
        //Send a move begin event, so that the mob can calculate
        //a (new) path and start taking it.
            scriptVM->fsm.runEvent(FSM_EV_CARRY_BEGIN_MOVE);
    }
}


/**
 * @brief Event handler for a carrier Pikmin being removed.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::handleCarrierRemoved(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    Pikmin* pikPtr = (Pikmin*) info1;
    
    //Save some data before changing anything.
    bool couldMove =
        scriptVM->mob->carryInfo->curCarryingStrength >=
        scriptVM->mob->type->weight;
    Mob* prevDestination = scriptVM->mob->carryInfo->intendedMob;
    
    //Update the numbers and such.
    scriptVM->mob->carryInfo->spotInfo[pikPtr->tempI].pikPtr = nullptr;
    scriptVM->mob->carryInfo->spotInfo[pikPtr->tempI].state =
        CARRY_SPOT_STATE_FREE;
    scriptVM->mob->carryInfo->curCarryingStrength -=
        pikPtr->pikType->carryStrength;
    scriptVM->mob->carryInfo->curNCarriers--;
    
    scriptVM->mob->chaseInfo.maxSpeed = scriptVM->mob->carryInfo->getSpeed();
    scriptVM->mob->chaseInfo.acceleration = MOB::CARRIED_MOB_ACCELERATION;
    
    bool canMove =
        scriptVM->mob->carryInfo->curCarryingStrength >=
        scriptVM->mob->type->weight;
    
    if(!canMove) {
        if(couldMove) {
            //If the mob can no longer move, send a move stop event,
            //so the mob, well, stops.
                scriptVM->fsm.runEvent(FSM_EV_CARRY_STOP_MOVE);
        }
        return;
    }
    
    scriptVM->mob->calculateCarryingDestination(
        &scriptVM->mob->carryInfo->intendedPikType,
        &scriptVM->mob->carryInfo->intendedMob,
        &scriptVM->mob->carryInfo->intendedPoint
    );
    
    //Check if we need to update the path.
    bool mustUpdate = false;
    
    //Start by checking if the target changed.
    if(canMove && (prevDestination != scriptVM->mob->carryInfo->intendedMob)) {
        mustUpdate = true;
    }
    
    //Now, check if the fact that it can fly or not changed.
    if(!mustUpdate && scriptVM->mob->pathInfo) {
        bool oldIsAirborne =
            hasFlag(
                scriptVM->mob->pathInfo->settings.flags,
                PATH_FOLLOW_FLAG_AIRBORNE
            );
        bool newIsAirborne = scriptVM->mob->carryInfo->canFly();
        mustUpdate = oldIsAirborne != newIsAirborne;
    }
    
    //Check if the list of invulnerabilities changed.
    if(!mustUpdate && scriptVM->mob->pathInfo) {
        vector<Hazard*> newInvulnerabilities =
            scriptVM->mob->carryInfo->getCarrierInvulnerabilities();
            
        if(
            !isPermutation(
                newInvulnerabilities,
                scriptVM->mob->pathInfo->settings.invulnerabilities
            )
        ) {
            mustUpdate = true;
        }
    }
    
    if(mustUpdate) {
        //Send a move begin event, so that the mob can calculate
        //a (new) path and start taking it.
            scriptVM->fsm.runEvent(FSM_EV_CARRY_BEGIN_MOVE);
    }
}


/**
 * @brief Generic handler for when a mob was delivered to an Onion/ship.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::handleDelivery(ScriptVM* scriptVM, void* info1, void* info2) {
    if(scriptVM->focusedMob) {
        scriptVM->focusedMob->scriptVM.fsm.runEvent(
            FSM_EV_FINISHED_RECEIVING_DELIVERY, (void*) scriptVM->mob
        );
    }
    
    if(game.curArea->type == AREA_TYPE_MISSION) {
        forIdx(g, game.states.gameplay->missionMobGroups) {
            game.states.gameplay->missionMobGroups[g].remove(scriptVM->mob);
        }
    }
    scriptVM->mob->toDelete = true;
}


/**
 * @brief When a mob has to lose its momentum.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::loseMomentum(ScriptVM* scriptVM, void* info1, void* info2) {
    scriptVM->mob->speed.x = scriptVM->mob->speed.y = 0.0f;
}


/**
 * @brief When a mob starts the process of being delivered to an Onion/ship.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::startBeingDelivered(
    ScriptVM* scriptVM, void* info1, void* info2
) {
    forIdx(p, scriptVM->mob->carryInfo->spotInfo) {
        Mob* pikPtr = scriptVM->mob->carryInfo->spotInfo[p].pikPtr;
        if(pikPtr) {
            pikPtr->scriptVM.fsm.runEvent(FSM_EV_FINISHED_TASK);
        }
    }
    
    if(
        scriptVM->mob->carryInfo->intendedMob->type->category->id ==
        MOB_CATEGORY_ONIONS
    ) {
        Onion* oniPtr = (Onion*) scriptVM->mob->carryInfo->intendedMob;
        scriptVM->mob->deliveryInfo->animType = oniPtr->oniType->deliveryAnim;
    }
    
    scriptVM->focusOnMob(scriptVM->mob->carryInfo->intendedMob);
    enableFlag(scriptVM->mob->flags, MOB_FLAG_INTANGIBLE);
    scriptVM->mob->becomeUncarriable();
    
    scriptVM->focusedMob->scriptVM.fsm.runEvent(
        FSM_EV_STARTED_RECEIVING_DELIVERY, scriptVM->mob
    );
    
    switch(scriptVM->mob->deliveryInfo->animType) {
    case DELIVERY_ANIM_SUCK: {
        scriptVM->setTimer(MOB::DELIVERY_SUCK_TIME);
        break;
    }
    case DELIVERY_ANIM_TOSS: {
        scriptVM->setTimer(MOB::DELIVERY_TOSS_TIME);
        break;
    }
    }
}


/**
 * @brief Generic handler for a mob touching a hazard.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void GenMobFsm::touchHazard(ScriptVM* scriptVM, void* info1, void* info2) {
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    Hazard* h = (Hazard*) info1;
    
    forIdx(e, h->effects) {
        scriptVM->mob->applyStatus(h->effects[e], false, true);
    }
}


/**
 * @brief Generic handler for a mob touching a spray.
 *
 * @param scriptVM The script VM responsible.
 * @param info1 Pointer to the spray type.
 * @param info2 Pointer to the mob that sprayed, if any.
 */
void GenMobFsm::touchSpray(ScriptVM* scriptVM, void* info1, void* info2) {
    engineAssert(info1 != nullptr, scriptVM->fsm.getStateHistoryStr());
    
    SprayType* s = (SprayType*) info1;
    Mob* sprayer = (Mob*) info2;
    
    forIdx(e, s->effects) {
        scriptVM->mob->applyStatus(s->effects[e], false, false, sprayer);
    }
}
