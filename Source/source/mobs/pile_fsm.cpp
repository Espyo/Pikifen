/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pile finite state machine logic.
 */

#include "../functions.h"
#include "mob_fsm.h"
#include "pile.h"
#include "pile_fsm.h"
#include "../utils/string_utils.h"

/* ----------------------------------------------------------------------------
 * Creates the finite state machine for the pile's logic.
 */
void pile_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    
    efc.new_state("idling", PILE_STATE_IDLING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(pile_fsm::become_idle);
        }
    }
    
    
    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idling");
    
    //Check if the number in the enum and the total match up.
    engine_assert(
        typ->states.size() == N_PILE_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_PILE_STATES) + " in enum."
    );
}


/* ----------------------------------------------------------------------------
 * When a pile starts idling.
 */
void pile_fsm::become_idle(mob* m, void* info1, void* info2) {
    m->set_animation(PILE_ANIM_IDLING);
}
