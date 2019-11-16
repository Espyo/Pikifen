/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Bouncer finite state machine logic.
 */

#include "bouncer_fsm.h"

#include "../functions.h"
#include "../mobs/bouncer.h"
#include "../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Creates the finite state machine for the bouncer's logic.
 */
void bouncer_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    efc.new_state("idling", BOUNCER_STATE_IDLING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(bouncer_fsm::spawn);
        }
    }
    
    
    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idling");
    
    //Check if the number in the enum and the total match up.
    engine_assert(
        typ->states.size() == N_BOUNCER_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_BOUNCER_STATES) + " in enum."
    );
}


/* ----------------------------------------------------------------------------
 * When the bouncer spawns.
 */
void bouncer_fsm::spawn(mob* m, void* info1, void* info2) {
    m->set_animation(BOUNCER_ANIM_IDLING);
}
