/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Drop finite state machine logic.
 */

#include "drop.h"
#include "drop_fsm.h"
#include "../functions.h"
#include "../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Creates the finite state machine for the drop's logic.
 */
void drop_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    efc.new_state("idling", DROP_STATE_IDLING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(drop_fsm::set_anim);
        }
        //TODO
    }
    
    
    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idling");
    
    //Check if the number in the enum and the total match up.
    engine_assert(
        typ->states.size() == N_DROP_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_DROP_STATES) + " in enum."
    );
}


/* ----------------------------------------------------------------------------
 * Sets the standard "idling" animation.
 */
void drop_fsm::set_anim(mob* m, void* info1, void* info2) {
    m->set_animation(DROP_ANIM_IDLING);
}
