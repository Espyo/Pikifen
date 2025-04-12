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
void resource_fsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    
    efc.newState("idle_waiting", RESOURCE_STATE_IDLE_WAITING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(resource_fsm::startWaiting);
            efc.run(gen_mob_fsm::carryStopMove);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carryGetPath);
            efc.changeState("idle_moving");
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(resource_fsm::loseMomentum);
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(resource_fsm::vanish);
        }
    }
    
    efc.newState("idle_moving", RESOURCE_STATE_IDLE_MOVING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(resource_fsm::handleStartMoving);
            efc.run(gen_mob_fsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_STOP_MOVE); {
            efc.run(resource_fsm::handleDropped);
            efc.changeState("idle_waiting");
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carryGetPath);
            efc.run(gen_mob_fsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(resource_fsm::handleReachDestination);
        }
        efc.newEvent(MOB_EV_PATH_BLOCKED); {
            efc.changeState("idle_stuck");
        }
        efc.newEvent(MOB_EV_PATHS_CHANGED); {
            efc.run(gen_mob_fsm::carryGetPath);
            efc.run(gen_mob_fsm::carryBeginMove);
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
            efc.run(gen_mob_fsm::carryBecomeStuck);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carryStopBeingStuck);
            efc.run(gen_mob_fsm::carryGetPath);
            efc.changeState("idle_moving");
        }
        efc.newEvent(MOB_EV_CARRY_STOP_MOVE); {
            efc.run(gen_mob_fsm::carryStopBeingStuck);
            efc.run(resource_fsm::handleDropped);
            efc.changeState("idle_waiting");
        }
        efc.newEvent(MOB_EV_PATHS_CHANGED); {
            efc.run(gen_mob_fsm::carryStopBeingStuck);
            efc.run(gen_mob_fsm::carryGetPath);
            efc.changeState("idle_moving");
        }
    }
    
    efc.newState("idle_thrown", RESOURCE_STATE_IDLE_THROWN); {
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(gen_mob_fsm::loseMomentum);
            efc.run(gen_mob_fsm::carryGetPath);
            efc.changeState("idle_moving");
        }
    }
    
    efc.newState("being_delivered", RESOURCE_STATE_BEING_DELIVERED); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(resource_fsm::startBeingDelivered);
            efc.run(gen_mob_fsm::startBeingDelivered);
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(resource_fsm::handleDelivery);
            efc.run(gen_mob_fsm::handleDelivery);
        }
    }
    
    efc.newState(
        "staying_after_delivery", RESOURCE_STATE_STAYING_AFTER_DELIVERY
    ); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(resource_fsm::startWaiting);
            efc.run(gen_mob_fsm::carryStopMove);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carryGetPath);
            efc.changeState("idle_moving");
        }
        efc.newEvent(MOB_EV_CARRY_STOP_MOVE); {
            efc.run(gen_mob_fsm::carryStopBeingStuck);
            efc.run(resource_fsm::handleDropped);
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
void resource_fsm::handleDelivery(Mob* m, void* info1, void* info2) {
    Resource* res_ptr = (Resource*) m;
    if(
        res_ptr->resType->deliveryResult ==
        RESOURCE_DELIVERY_RESULT_DAMAGE_MOB
    ) {
        res_ptr->focusedMob->setHealth(
            true, false, -res_ptr->resType->damageMobAmount
        );
        
        HitboxInteraction ev_info(res_ptr, nullptr, nullptr);
        res_ptr->fsm.runEvent(MOB_EV_DAMAGE, (void*) &ev_info);
    }
}


/**
 * @brief When the resource is dropped.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void resource_fsm::handleDropped(Mob* m, void* info1, void* info2) {
    Resource* res_ptr = (Resource*) m;
    if(!res_ptr->resType->vanishOnDrop) return;
    
    if(res_ptr->resType->vanishDelay == 0) {
        resource_fsm::vanish(m, info1, info2);
    } else {
        res_ptr->setTimer(res_ptr->resType->vanishDelay);
    }
}


/**
 * @brief When the resource reaches its carry destination.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void resource_fsm::handleReachDestination(Mob* m, void* info1, void* info2) {
    Resource* res_ptr = (Resource*) m;
    if(res_ptr->resType->deliveryResult == RESOURCE_DELIVERY_RESULT_STAY) {
        m->stopFollowingPath();
        m->fsm.setState(RESOURCE_STATE_STAYING_AFTER_DELIVERY);
    } else {
        gen_mob_fsm::carryReachDestination(m, info1, info2);
    }
}


/**
 * @brief When the resource starts moving.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void resource_fsm::handleStartMoving(Mob* m, void* info1, void* info2) {
    Resource* res_ptr = (Resource*) m;
    res_ptr->setTimer(0);
}


/**
 * @brief When the resource lands from being launched in the air.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void resource_fsm::loseMomentum(Mob* m, void* info1, void* info2) {
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
void resource_fsm::startBeingDelivered(Mob* m, void* info1, void* info2) {
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
void resource_fsm::startWaiting(Mob* m, void* info1, void* info2) {
    Resource* res_ptr = (Resource*) m;
    
    if(res_ptr->toDelete) return;
    
    if(res_ptr->originPile) {
        res_ptr->carryInfo->mustReturn = true;
        res_ptr->carryInfo->returnPoint = res_ptr->originPile->pos;
        res_ptr->carryInfo->returnDist =
            res_ptr->originPile->radius +
            game.config.pikmin.standardRadius +
            game.config.pikmin.idleTaskRange / 2.0f;
    } else {
        res_ptr->carryInfo->mustReturn = false;
    }
    
    res_ptr->setAnimation(
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
void resource_fsm::vanish(Mob* m, void* info1, void* info2) {
    Resource* res_ptr = (Resource*) m;
    if(res_ptr->resType->returnToPileOnVanish && res_ptr->originPile) {
        res_ptr->originPile->changeAmount(+1);
    }
    
    res_ptr->becomeUncarriable();
    res_ptr->toDelete = true;
}
