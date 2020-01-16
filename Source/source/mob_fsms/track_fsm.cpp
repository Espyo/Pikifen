/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Track finite state machine logic.
 */

#include "track_fsm.h"

#include "../functions.h"
#include "../mobs/track.h"
#include "../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Creates the finite state machine for the track's logic.
 */
void track_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    efc.new_state("idling", TRACK_STATE_IDLING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(track_fsm::spawn);
        }
        efc.new_event(MOB_EVENT_TOUCHED_OBJECT); {
            efc.run(track_fsm::on_touched);
        }
    }
    
    
    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idling");
    
    //Check if the number in the enum and the total match up.
    engine_assert(
        typ->states.size() == N_TRACK_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_TRACK_STATES) + " in enum."
    );
}


/* ----------------------------------------------------------------------------
 * When the track spawns.
 */
void track_fsm::spawn(mob* m, void* info1, void* info2) {
    m->set_animation(TRACK_ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * What to do when the track is touched.
 */
void track_fsm::on_touched(mob* m, void* info1, void* info2) {
    track* tra_ptr = (track*) m;
    mob* toucher = (mob*) info1;
    
    mob_event* ev = NULL;
    
    //Check if a compatible mob touched it.
    if(
        tra_ptr->tra_type->riders & TRACK_RIDER_PIKMIN &&
        toucher->type->category->id == MOB_CATEGORY_PIKMIN
    ) {
    
        //Pikmin is about to ride it.
        ev = q_get_event(toucher, MOB_EVENT_TOUCHED_TRACK);
        
    } else if(
        tra_ptr->tra_type->riders & TRACK_RIDER_LEADERS &&
        toucher->type->category->id == MOB_CATEGORY_LEADERS
    ) {
    
        //Leader is about to ride it.
        ev = q_get_event(toucher, MOB_EVENT_TOUCHED_TRACK);
        
    }
    
    if(!ev) return;
    
    ev->run(toucher, (void*) m);
}
