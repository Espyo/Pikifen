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
void decoration_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    efc.new_state("idling", DECORATION_STATE_IDLING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(decoration_fsm::become_idle);
        }
        efc.new_event(MOB_EV_TOUCHED_OBJECT); {
            efc.run(decoration_fsm::check_bump);
        }
    }
    efc.new_state("bumped", DECORATION_STATE_BUMPED); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(decoration_fsm::be_bumped);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.change_state("idling");
        }
    }
    
    
    typ->states = efc.finish();
    typ->first_state_idx = fix_states(typ->states, "idling", typ);
    
    //Check if the number in the enum and the total match up.
    engine_assert(
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
void decoration_fsm::be_bumped(mob* m, void* info1, void* info2) {
    m->set_animation(DECORATION_ANIM_BUMPED);
}


/**
 * @brief When the decoration becomes idle.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void decoration_fsm::become_idle(mob* m, void* info1, void* info2) {
    decoration* dec_ptr = (decoration*) m;
    if(
        dec_ptr->dec_type->random_animation_delay &&
        dec_ptr->individual_random_anim_delay
    ) {
        m->set_animation(
            DECORATION_ANIM_IDLING,
            START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
        );
    } else {
        m->set_animation(DECORATION_ANIM_IDLING);
    }
}


/**
 * @brief Check if the decoration should really get bumped.
 *
 * @param m The mob.
 * @param info1 Pointer to the mob that touched it.
 * @param info2 Unused.
 */
void decoration_fsm::check_bump(mob* m, void* info1, void* info2) {
    mob* toucher = (mob*) info1;
    if(
        toucher->speed.x == 0 && toucher->speed.y == 0 &&
        toucher->chase_info.state != CHASE_STATE_CHASING
    ) {
        //Is the other object not currently moving? Let's not get bumped.
        return;
    }
    
    m->fsm.set_state(DECORATION_STATE_BUMPED);
}
