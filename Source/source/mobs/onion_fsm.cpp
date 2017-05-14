/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
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
    
    efc.new_state("idling", ONION_STATE_IDLING); {
        efc.new_event(MOB_EVENT_RECEIVE_DELIVERY); {
            efc.run(onion_fsm::receive_mob);
        }
    }
    
    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idling");
    
    //Check if the number in the enum and the total match up.
    assert(typ->states.size() == N_ONION_STATES);
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
    
    particle p(
        PARTICLE_TYPE_BITMAP, m->pos,
        24, 1.5, PARTICLE_PRIORITY_MEDIUM
    );
    p.bitmap = bmp_smoke;
    particle_generator pg(0, p, 15);
    pg.number_deviation = 5;
    pg.angle = 0;
    pg.angle_deviation = M_PI;
    pg.total_speed = 70;
    pg.total_speed_deviation = 10;
    pg.duration_deviation = 0.5;
    pg.emit(particles);
    
}
