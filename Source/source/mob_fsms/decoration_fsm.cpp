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

#include "../functions.h"
#include "../mobs/decoration.h"
#include "../utils/string_utils.h"
#include "gen_mob_fsm.h"

/* ----------------------------------------------------------------------------
 * Creates the finite state machine for the decoration's logic.
 */
void decoration_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    efc.new_state("idling", DECORATION_STATE_IDLING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(decoration_fsm::become_idle);
        }
        efc.new_event(MOB_EVENT_TOUCHED_OBJECT); {
            efc.change_state("bumped");
        }
    }
    efc.new_state("bumped", DECORATION_STATE_BUMPED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(decoration_fsm::be_bumped);
        }
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.change_state("idling");
        }
    }
    
    
    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idling");
    
    //Check if the number in the enum and the total match up.
    engine_assert(
        typ->states.size() == N_DECORATION_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_DECORATION_STATES) + " in enum."
    );
}


/* ----------------------------------------------------------------------------
 * When the decoration becomes idle.
 */
void decoration_fsm::become_idle(mob* m, void* info1, void* info2) {
    decoration* dec_ptr = (decoration*) m;
    m->set_animation(DECORATION_ANIM_IDLING);
    
    if(!dec_ptr->has_done_first_animation) {
        dec_ptr->has_done_first_animation = true;
        if(dec_ptr->dec_type->random_animation_delay) {
            dec_ptr->anim.skip_ahead_randomly();
        }
    }
}


/* ----------------------------------------------------------------------------
 * When the decoration gets bumped.
 */
void decoration_fsm::be_bumped(mob* m, void* info1, void* info2) {
    m->set_animation(DECORATION_ANIM_BUMPED);
}
