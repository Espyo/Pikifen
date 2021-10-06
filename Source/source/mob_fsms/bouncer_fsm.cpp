/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Bouncer finite state machine logic.
 */

#include <algorithm>

#include "bouncer_fsm.h"

#include "../functions.h"
#include "../mobs/bouncer.h"
#include "../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Creates the finite state machine for the bouncer's logic.
 * typ:
 *   Mob type to create the finite state machine for.
 */
void bouncer_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    efc.new_state("idling", BOUNCER_STATE_IDLING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(bouncer_fsm::set_idling_animation);
        }
        efc.new_event(MOB_EV_RIDER_ADDED); {
            efc.run(bouncer_fsm::handle_mob);
            efc.change_state("bouncing");
        }
    }
    efc.new_state("bouncing", BOUNCER_STATE_BOUNCING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(bouncer_fsm::set_bouncing_animation);
        }
        efc.new_event(MOB_EV_RIDER_ADDED); {
            efc.run(bouncer_fsm::handle_mob);
        }
        efc.new_event(MOB_EV_ANIMATION_END); {
            efc.change_state("idling");
        }
    }
    
    
    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idling", typ);
    
    //Check if the number in the enum and the total match up.
    engine_assert(
        typ->states.size() == N_BOUNCER_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_BOUNCER_STATES) + " in enum."
    );
}


/* ----------------------------------------------------------------------------
 * When something is on top of the bouncer.
 * m:
 *   The mob.
 * info1:
 *   Points to the mob that is on top of it.
 * info2:
 *   Unused.
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
        bou_ptr->bou_type->riders & BOUNCER_RIDER_PIKMIN &&
        toucher->type->category->id == MOB_CATEGORY_PIKMIN
    ) {
    
        //Pikmin is about to be bounced.
        ev = toucher->fsm.get_event(MOB_EV_TOUCHED_BOUNCER);
        
    } else if(
        bou_ptr->bou_type->riders & BOUNCER_RIDER_LEADERS &&
        toucher->type->category->id == MOB_CATEGORY_LEADERS
    ) {
    
        //Leader is about to be bounced.
        ev = toucher->fsm.get_event(MOB_EV_TOUCHED_BOUNCER);
        
    } else if(
        bou_ptr->bou_type->riders & BOUNCER_RIDER_PIKMIN &&
        toucher->path_info &&
        (toucher->path_info->taker_flags & PATH_TAKER_FLAG_LIGHT_LOAD)
    ) {
    
        //Pikmin carrying light load is about to be bounced.
        ev = toucher->fsm.get_event(MOB_EV_TOUCHED_BOUNCER);
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
    float max_h = std::max(128.0f, (target_mob->z - toucher->z) * 1.5f);
    calculate_throw(
        toucher->pos,
        toucher->z,
        target_mob->pos,
        target_mob->z + target_mob->height,
        max_h, GRAVITY_ADDER,
        &toucher->speed,
        &toucher->speed_z,
        &angle
    );
    
    toucher->face(angle, NULL, true);
    
    ev->run(toucher, (void*) m);
}


/* ----------------------------------------------------------------------------
 * When it must change to the bouncing animation.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void bouncer_fsm::set_bouncing_animation(mob* m, void* info1, void* info2) {
    m->set_animation(BOUNCER_ANIM_BOUNCING);
}


/* ----------------------------------------------------------------------------
 * When it must change to the idling animation.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void bouncer_fsm::set_idling_animation(mob* m, void* info1, void* info2) {
    m->set_animation(BOUNCER_ANIM_IDLING);
}
