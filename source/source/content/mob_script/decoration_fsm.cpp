/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Decoration finite state machine logic.
 */

#include "decoration_fsm.h"

#include "../../core/misc_functions.h"
#include "../../util/string_utils.h"
#include "../mob/decoration.h"
#include "gen_mob_fsm.h"


/**
 * @brief Creates the finite state machine for the decoration's logic.
 *
 * @param typ Mob type to create the finite state machine for.
 */
void DecorationFsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    efc.newState("idling", DECORATION_STATE_IDLING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(DecorationFsm::becomeIdle);
        }
        efc.newEvent(MOB_EV_TOUCHED_OBJECT); {
            efc.run(DecorationFsm::checkBump);
        }
    }
    efc.newState("bumped", DECORATION_STATE_BUMPED); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(DecorationFsm::beBumped);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("idling");
        }
    }
    
    
    typ->states = efc.finish();
    typ->firstStateIdx = fixStates(typ->states, "idling", typ);
    
    //Check if the number in the enum and the total match up.
    engineAssert(
        typ->states.size() == N_DECORATION_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_DECORATION_STATES) + " in enum."
    );
}


/**
 * @brief When the decoration gets bumped.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void DecorationFsm::beBumped(Mob* m, void* info1, void* info2) {
    m->setAnimation(DECORATION_ANIM_BUMPED);
}


/**
 * @brief When the decoration becomes idle.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void DecorationFsm::becomeIdle(Mob* m, void* info1, void* info2) {
    Decoration* decPtr = (Decoration*) m;
    if(
        decPtr->decType->randomAnimationDelay &&
        decPtr->individualRandomAnimDelay
    ) {
        m->setAnimation(
            DECORATION_ANIM_IDLING,
            START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
        );
    } else {
        m->setAnimation(DECORATION_ANIM_IDLING);
    }
}


/**
 * @brief Check if the decoration should really get bumped.
 *
 * @param m The mob.
 * @param info1 Pointer to the mob that touched it.
 * @param info2 Unused.
 */
void DecorationFsm::checkBump(Mob* m, void* info1, void* info2) {
    Mob* toucher = (Mob*) info1;
    if(
        toucher->speed.x == 0 && toucher->speed.y == 0 &&
        toucher->chaseInfo.state != CHASE_STATE_CHASING
    ) {
        //Is the other object not currently moving? Let's not get bumped.
        return;
    }
    
    m->fsm.setState(DECORATION_STATE_BUMPED);
}
