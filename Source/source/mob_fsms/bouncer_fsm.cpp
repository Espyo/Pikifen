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
#include <algorithm>
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
        efc.new_event(MOB_EVENT_WEIGHT_ADDED); {
            efc.run(bouncer_fsm::handle_mob);
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
 * When something is on top of the bouncer.
 * info1: Points to the mob that is on top of it.
 */
void bouncer_fsm::handle_mob(mob* m, void* info1, void* info2) {
    if(m->links.empty()) {
        log_error(
            "The bouncer at " + p2s(m->pos) + ", of the type \"" +
            m->type->name + "\" has no linked mob to serve as a target!"
        );
        return;
    }
    
    bouncer* bou_ptr = (bouncer*) m;
    mob* toucher = (mob*) info1;
    mob* target_mob = bou_ptr->links[0];
    
    mob_event* ev = NULL;
    
    //Check if a compatible mob touched it.
    if(
        bou_ptr->bou_type->riders | BOUNCER_RIDER_PIKMIN &&
        toucher->type->category->id == MOB_CATEGORY_PIKMIN
    ) {
    
        //Pikmin is about to be bounced.
        ev = q_get_event(toucher, MOB_EVENT_TOUCHED_BOUNCER);
        
    } else if(
        bou_ptr->bou_type->riders | BOUNCER_RIDER_LEADERS &&
        toucher->type->category->id == MOB_CATEGORY_LEADERS
    ) {
    
        //Leader is about to be bounced.
        ev = q_get_event(toucher, MOB_EVENT_TOUCHED_BOUNCER);
        
    }
    
    if(!ev) return;
    
    toucher->stop_chasing();
    toucher->leave_group();
    toucher->was_thrown = true;
    toucher->start_height_effect();
    
    float angle;
    //The maximum height has a guaranteed minimum (useful if the destination
    //is below the bouncer), and scales up with how much higher the thrown
    //mob needs to go, to make for a nice smooth arc.
    float max_h = max(128.0f, (target_mob->z - toucher->z) * 1.5f);
    toucher->calculate_throw(
        target_mob->pos,
        target_mob->z,
        max_h,
        &toucher->speed,
        &toucher->speed_z,
        &angle
    );
    
    toucher->angle = angle;
    toucher->angle_cos = cos(angle);
    toucher->angle_sin = sin(angle);
    toucher->face(angle, NULL);
    
    ev->run(toucher, (void*) m);
}


/* ----------------------------------------------------------------------------
 * When the bouncer spawns.
 */
void bouncer_fsm::spawn(mob* m, void* info1, void* info2) {
    m->set_animation(BOUNCER_ANIM_IDLING);
}
