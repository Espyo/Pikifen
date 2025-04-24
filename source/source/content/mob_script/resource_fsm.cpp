/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Resource finite state machine logic.
 */

#include "resource_fsm.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/string_utils.h"
#include "../mob/resource.h"
#include "gen_mob_fsm.h"


/**
 * @brief Creates the finite state machine for the resource's logic.
 *
 * @param typ Mob type to create the finite state machine for.
 */
void ResourceFsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    
    efc.newState("idle_waiting", RESOURCE_STATE_IDLE_WAITING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(ResourceFsm::startWaiting);
            efc.run(GenMobFsm::carryStopMove);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(GenMobFsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(GenMobFsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(GenMobFsm::carryGetPath);
            efc.changeState("idle_moving");
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(ResourceFsm::loseMomentum);
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(ResourceFsm::vanish);
        }
    }
    
    efc.newState("idle_moving", RESOURCE_STATE_IDLE_MOVING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(ResourceFsm::handleStartMoving);
            efc.run(GenMobFsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(GenMobFsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(GenMobFsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_STOP_MOVE); {
            efc.run(ResourceFsm::handleDropped);
            efc.changeState("idle_waiting");
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(GenMobFsm::carryGetPath);
            efc.run(GenMobFsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(ResourceFsm::handleReachDestination);
        }
        efc.newEvent(MOB_EV_PATH_BLOCKED); {
            efc.changeState("idle_stuck");
        }
        efc.newEvent(MOB_EV_PATHS_CHANGED); {
            efc.run(GenMobFsm::carryGetPath);
            efc.run(GenMobFsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_CARRY_DELIVERED); {
            efc.changeState("being_delivered");
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.changeState("idle_thrown");
        }
    }
    
    efc.newState("idle_stuck", RESOURCE_STATE_IDLE_STUCK); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(GenMobFsm::carryBecomeStuck);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(GenMobFsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(GenMobFsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.run(GenMobFsm::carryGetPath);
            efc.changeState("idle_moving");
        }
        efc.newEvent(MOB_EV_CARRY_STOP_MOVE); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.run(ResourceFsm::handleDropped);
            efc.changeState("idle_waiting");
        }
        efc.newEvent(MOB_EV_PATHS_CHANGED); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.run(GenMobFsm::carryGetPath);
            efc.changeState("idle_moving");
        }
    }
    
    efc.newState("idle_thrown", RESOURCE_STATE_IDLE_THROWN); {
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(GenMobFsm::loseMomentum);
            efc.run(GenMobFsm::carryGetPath);
            efc.changeState("idle_moving");
        }
    }
    
    efc.newState("being_delivered", RESOURCE_STATE_BEING_DELIVERED); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(ResourceFsm::startBeingDelivered);
            efc.run(GenMobFsm::startBeingDelivered);
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(ResourceFsm::handleDelivery);
            efc.run(GenMobFsm::handleDelivery);
        }
    }
    
    efc.newState(
        "staying_after_delivery", RESOURCE_STATE_STAYING_AFTER_DELIVERY
    ); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(ResourceFsm::startWaiting);
            efc.run(GenMobFsm::carryStopMove);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(GenMobFsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(GenMobFsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(GenMobFsm::carryGetPath);
            efc.changeState("idle_moving");
        }
        efc.newEvent(MOB_EV_CARRY_STOP_MOVE); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.run(ResourceFsm::handleDropped);
            efc.changeState("idle_waiting");
        }
    }
    
    
    typ->states = efc.finish();
    typ->firstStateIdx = fixStates(typ->states, "idle_waiting", typ);
    
    //Check if the number in the enum and the total match up.
    engineAssert(
        typ->states.size() == N_RESOURCE_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_RESOURCE_STATES) + " in enum."
    );
}


/**
 * @brief When the resource is fully delivered. This should only run
 * code that cannot be handled by ships or Onions.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ResourceFsm::handleDelivery(Mob* m, void* info1, void* info2) {
    Resource* resPtr = (Resource*) m;
    if(
        resPtr->resType->deliveryResult ==
        RESOURCE_DELIVERY_RESULT_DAMAGE_MOB
    ) {
        resPtr->focusedMob->setHealth(
            true, false, -resPtr->resType->damageMobAmount
        );
        
        HitboxInteraction evInfo(resPtr, nullptr, nullptr);
        resPtr->fsm.runEvent(MOB_EV_DAMAGE, (void*) &evInfo);
    }
}


/**
 * @brief When the resource is dropped.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ResourceFsm::handleDropped(Mob* m, void* info1, void* info2) {
    Resource* resPtr = (Resource*) m;
    if(!resPtr->resType->vanishOnDrop) return;
    
    if(resPtr->resType->vanishDelay == 0) {
        ResourceFsm::vanish(m, info1, info2);
    } else {
        resPtr->setTimer(resPtr->resType->vanishDelay);
    }
}


/**
 * @brief When the resource reaches its carry destination.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ResourceFsm::handleReachDestination(Mob* m, void* info1, void* info2) {
    Resource* resPtr = (Resource*) m;
    if(resPtr->resType->deliveryResult == RESOURCE_DELIVERY_RESULT_STAY) {
        m->stopFollowingPath();
        m->fsm.setState(RESOURCE_STATE_STAYING_AFTER_DELIVERY);
    } else {
        GenMobFsm::carryReachDestination(m, info1, info2);
    }
}


/**
 * @brief When the resource starts moving.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ResourceFsm::handleStartMoving(Mob* m, void* info1, void* info2) {
    Resource* resPtr = (Resource*) m;
    resPtr->setTimer(0);
}


/**
 * @brief When the resource lands from being launched in the air.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ResourceFsm::loseMomentum(Mob* m, void* info1, void* info2) {
    m->speed.x = 0;
    m->speed.y = 0;
    m->speedZ = 0;
}


/**
 * @brief When a resource starts being delivered.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ResourceFsm::startBeingDelivered(Mob* m, void* info1, void* info2) {
    if(
        m->carryInfo->intendedMob &&
        m->carryInfo->intendedMob->type->category->id == MOB_CATEGORY_BRIDGES
    ) {
        m->deliveryInfo->animType = DELIVERY_ANIM_TOSS;
    }
}


/**
 * @brief When a resource starts idling, waiting to be carried.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ResourceFsm::startWaiting(Mob* m, void* info1, void* info2) {
    Resource* resPtr = (Resource*) m;
    
    if(resPtr->toDelete) return;
    
    if(resPtr->originPile) {
        resPtr->carryInfo->mustReturn = true;
        resPtr->carryInfo->returnPoint = resPtr->originPile->pos;
        resPtr->carryInfo->returnDist =
            resPtr->originPile->radius +
            game.config.pikmin.standardRadius +
            game.config.pikmin.idleTaskRange / 2.0f;
    } else {
        resPtr->carryInfo->mustReturn = false;
    }
    
    resPtr->setAnimation(
        RESOURCE_ANIM_IDLING, START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
}


/**
 * @brief Vanishes, either disappearing for good, or returning to
 * its origin pile.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void ResourceFsm::vanish(Mob* m, void* info1, void* info2) {
    Resource* resPtr = (Resource*) m;
    if(resPtr->resType->returnToPileOnVanish && resPtr->originPile) {
        resPtr->originPile->changeAmount(+1);
    }
    
    resPtr->becomeUncarriable();
    resPtr->toDelete = true;
}
