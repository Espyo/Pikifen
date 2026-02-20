/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Bridge finite-state machine logic.
 */

#include "bridge_fsm.h"

#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"
#include "../mob/bridge.h"
#include "gen_mob_fsm.h"


#pragma region FSM


/**
 * @brief Creates the finite-state machine for the bridge's logic.
 *
 * @param typ Mob type to create the finite-state machine for.
 */
void BridgeFsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    efc.newState("idling", BRIDGE_STATE_IDLING); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(BridgeFsm::setAnim);
        }
        efc.newEvent(SCRIPT_EV_ON_READY); {
            efc.run(BridgeFsm::setup);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(GenMobFsm::beAttacked);
            efc.run(BridgeFsm::checkHealth);
        }
        efc.newEvent(MOB_EV_FINISHED_RECEIVING_DELIVERY); {
            efc.run(BridgeFsm::checkHealth);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.run(BridgeFsm::checkHealth);
            efc.run(BridgeFsm::open);
            efc.changeState("destroyed");
        }
    }
    efc.newState("creating_chunk", BRIDGE_STATE_CREATING_CHUNK); {
        //Sort of a dummy state for text file script enhancements.
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.changeState("idling");
        }
    }
    efc.newState("destroyed", BRIDGE_STATE_DESTROYED); {
    
    }
    
    
    typ->states = efc.finish();
    typ->firstStateIdx = fixStates(typ->states, "idling", typ);
    
    //Check if the number in the enum and the total match up.
    engineAssert(
        typ->states.size() == N_BRIDGE_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_BRIDGE_STATES) + " in enum."
    );
}


#pragma endregion
#pragma region FSM functions


/**
 * @brief Makes the bridge check its health and update its chunks, if needed.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void BridgeFsm::checkHealth(Mob* m, void* info1, void* info2) {
    Bridge* briPtr = (Bridge*) m;
    if(briPtr->checkHealth()) {
        m->fsm.setState(BRIDGE_STATE_CREATING_CHUNK);
    }
}


/**
 * @brief Opens up the bridge.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void BridgeFsm::open(Mob* m, void* info1, void* info2) {
    Bridge* briPtr = (Bridge*) m;
    briPtr->setAnimation(BRIDGE_ANIM_DESTROYED);
    briPtr->startDying();
    briPtr->finishDying();
    enableFlag(briPtr->flags, MOB_FLAG_INTANGIBLE);
}


/**
 * @brief Sets the standard "idling" animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void BridgeFsm::setAnim(Mob* m, void* info1, void* info2) {
    m->setAnimation(
        BRIDGE_ANIM_IDLING, START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
}


/**
 * @brief Sets up the bridge with the data surrounding it,
 * like its linked destination object.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void BridgeFsm::setup(Mob* m, void* info1, void* info2) {
    Bridge* briPtr = (Bridge*) m;
    briPtr->setup();
}


#pragma endregion
