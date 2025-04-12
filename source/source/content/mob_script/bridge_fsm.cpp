/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Bridge finite state machine logic.
 */

#include "bridge_fsm.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"
#include "../mob/bridge.h"
#include "gen_mob_fsm.h"


/**
 * @brief Creates the finite state machine for the bridge's logic.
 *
 * @param typ Mob type to create the finite state machine for.
 */
void bridge_fsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    efc.newState("idling", BRIDGE_STATE_IDLING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(bridge_fsm::setAnim);
        }
        efc.newEvent(MOB_EV_ON_READY); {
            efc.run(bridge_fsm::setup);
        }
        efc.newEvent(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(gen_mob_fsm::beAttacked);
            efc.run(bridge_fsm::checkHealth);
        }
        efc.newEvent(MOB_EV_FINISHED_RECEIVING_DELIVERY); {
            efc.run(bridge_fsm::checkHealth);
        }
        efc.newEvent(MOB_EV_ZERO_HEALTH); {
            efc.run(bridge_fsm::checkHealth);
            efc.run(bridge_fsm::open);
            efc.changeState("destroyed");
        }
    }
    efc.newState("creating_chunk", BRIDGE_STATE_CREATING_CHUNK); {
        //Sort of a dummy state for text file script enhancements.
        efc.newEvent(MOB_EV_ON_ENTER); {
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


/**
 * @brief Makes the bridge check its health and update its chunks, if needed.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void bridge_fsm::checkHealth(Mob* m, void* info1, void* info2) {
    Bridge* bri_ptr = (Bridge*) m;
    if(bri_ptr->checkHealth()) {
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
void bridge_fsm::open(Mob* m, void* info1, void* info2) {
    Bridge* bri_ptr = (Bridge*) m;
    bri_ptr->setAnimation(BRIDGE_ANIM_DESTROYED);
    bri_ptr->startDying();
    bri_ptr->finishDying();
    enableFlag(bri_ptr->flags, MOB_FLAG_INTANGIBLE);
}


/**
 * @brief Sets the standard "idling" animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void bridge_fsm::setAnim(Mob* m, void* info1, void* info2) {
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
void bridge_fsm::setup(Mob* m, void* info1, void* info2) {
    Bridge* bri_ptr = (Bridge*) m;
    bri_ptr->setup();
}
