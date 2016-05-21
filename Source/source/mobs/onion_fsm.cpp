/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Onion finite state machine logic.
 */

#include "../functions.h"
#include "onion.h"
#include "onion_fsm.h"
#include "../particle.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates the finite state machine for the Onion's logic.
 */
void onion_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    
    efc.new_state("idle", ONION_STATE_IDLE); {
        efc.new_event(MOB_EVENT_RECEIVE_DELIVERY); {
            efc.run_function(onion_fsm::receive_mob);
        }
    }
    
    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idle");
    
    if(typ->states.size() != N_ONION_STATES) {
        log_error(
            "ENGINE WARNING: Number of Onion states on the FSM (" +
            i2s(typ->states.size()) +
            ") and the enum (" + i2s(N_ONION_STATES) + ") do not match."
        );
    }
}

/* ----------------------------------------------------------------------------
 * When an Onion receives a mob, carried by Pikmin.
 */
void onion_fsm::receive_mob(mob* m, void* info1, void* info2) {
    size_t seeds = (size_t) info1;
    onion* o_ptr = (onion*) m;
    
    if(o_ptr->spew_queue == 0) {
        o_ptr->full_spew_timer.start();
        o_ptr->next_spew_timer.time_left = 0.0f;
    }
    o_ptr->spew_queue += seeds;
    
    random_particle_explosion(
        PARTICLE_TYPE_BITMAP, bmp_smoke,
        o_ptr->x, o_ptr->y,
        60, 80, 10, 20,
        1, 2, 24, 24, al_map_rgb(255, 255, 255)
    );
    
}
