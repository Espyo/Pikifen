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
void decoration_fsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    efc.newState("idling", DECORATION_STATE_IDLING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(decoration_fsm::becomeIdle);
        }
        efc.newEvent(MOB_EV_TOUCHED_OBJECT); {
            efc.run(decoration_fsm::checkBump);
        }
    }
    efc.newState("bumped", DECORATION_STATE_BUMPED); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(decoration_fsm::beBumped);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("idling");
        }
    }
    
    
    typ->states = efc.finish();
    typ->first_state_idx = fixStates(typ->states, "idling", typ);
    
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
void decoration_fsm::beBumped(Mob* m, void* info1, void* info2) {
    m->setAnimation(DECORATION_ANIM_BUMPED);
}


/**
 * @brief When the decoration becomes idle.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void decoration_fsm::becomeIdle(Mob* m, void* info1, void* info2) {
    Decoration* dec_ptr = (Decoration*) m;
    if(
        dec_ptr->dec_type->random_animation_delay &&
        dec_ptr->individual_random_anim_delay
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
void decoration_fsm::checkBump(Mob* m, void* info1, void* info2) {
    Mob* toucher = (Mob*) info1;
    if(
        toucher->speed.x == 0 && toucher->speed.y == 0 &&
        toucher->chase_info.state != CHASE_STATE_CHASING
    ) {
        //Is the other object not currently moving? Let's not get bumped.
        return;
    }
    
    m->fsm.setState(DECORATION_STATE_BUMPED);
}
