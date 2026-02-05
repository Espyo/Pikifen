/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Treasure finite-state machine logic.
 */

#include "treasure_fsm.h"

#include "../../core/misc_functions.h"
#include "../../util/string_utils.h"
#include "../mob/ship.h"
#include "../mob/treasure.h"
#include "gen_mob_fsm.h"


#pragma region FSM


/**
 * @brief Creates the finite-state machine for the treasure's logic.
 *
 * @param typ Mob type to create the finite-state machine for.
 */
void TreasureFsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    
    efc.newState("idle_waiting", TREASURE_STATE_IDLE_WAITING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(GenMobFsm::carryStopMove);
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(TreasureFsm::standStill);
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
    }
    
    efc.newState("idle_moving", TREASURE_STATE_IDLE_MOVING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(GenMobFsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_CARRIER_ADDED); {
            efc.run(GenMobFsm::handleCarrierAdded);
        }
        efc.newEvent(MOB_EV_CARRIER_REMOVED); {
            efc.run(GenMobFsm::handleCarrierRemoved);
        }
        efc.newEvent(MOB_EV_CARRY_STOP_MOVE); {
            efc.changeState("idle_waiting");
        }
        efc.newEvent(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(GenMobFsm::carryGetPath);
            efc.run(GenMobFsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_REACHED_DESTINATION); {
            efc.run(GenMobFsm::carryReachDestination);
        }
        efc.newEvent(MOB_EV_CARRY_DELIVERED); {
            efc.changeState("being_delivered");
        }
        efc.newEvent(MOB_EV_PATH_BLOCKED); {
            efc.changeState("idle_stuck");
        }
        efc.newEvent(MOB_EV_PATHS_CHANGED); {
            efc.run(GenMobFsm::carryGetPath);
            efc.run(GenMobFsm::carryBeginMove);
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(TreasureFsm::respawn);
        }
        efc.newEvent(MOB_EV_TOUCHED_BOUNCER); {
            efc.changeState("idle_thrown");
        }
    }
    
    efc.newState("idle_stuck", TREASURE_STATE_IDLE_STUCK); {
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
            efc.changeState("idle_waiting");
        }
        efc.newEvent(MOB_EV_PATHS_CHANGED); {
            efc.run(GenMobFsm::carryStopBeingStuck);
            efc.run(GenMobFsm::carryGetPath);
            efc.changeState("idle_moving");
        }
        efc.newEvent(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(TreasureFsm::respawn);
        }
    }
    
    efc.newState("idle_thrown", TREASURE_STATE_IDLE_THROWN); {
        efc.newEvent(MOB_EV_LANDED); {
            efc.run(GenMobFsm::loseMomentum);
            efc.run(GenMobFsm::carryGetPath);
            efc.changeState("idle_moving");
        }
    }
    
    efc.newState("being_delivered", TREASURE_STATE_BEING_DELIVERED); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(GenMobFsm::startBeingDelivered);
        }
        efc.newEvent(MOB_EV_TIMER); {
            efc.run(GenMobFsm::handleDelivery);
        }
    }
    
    
    typ->states = efc.finish();
    typ->firstStateIdx = fixStates(typ->states, "idle_waiting", typ);
    
    //Check if the number in the enum and the total match up.
    engineAssert(
        typ->states.size() == N_TREASURE_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_TREASURE_STATES) + " in enum."
    );
}


#pragma endregion
#pragma region FSM functions


/**
 * @brief When a treasure falls into a bottomless pit and should respawn.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void TreasureFsm::respawn(Mob* m, void* info1, void* info2) {
    m->becomeUncarriable(); //Force all Pikmin to let go.
    m->becomeCarriable(CARRY_DESTINATION_SHIP);
    m->respawn();
}


/**
 * @brief When the treasure should lose its momentum and stand still.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void TreasureFsm::standStill(Mob* m, void* info1, void* info2) {
    m->stopChasing();
    m->stopTurning();
}


#pragma endregion
