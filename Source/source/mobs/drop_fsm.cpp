/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
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
            efc.run(drop_fsm::set_idling_anim);
        }
        efc.new_event(MOB_EVENT_TOUCHED_OBJECT); {
            efc.run(drop_fsm::on_touched);
        }
    }
    efc.new_state("falling", DROP_STATE_FALLING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(drop_fsm::set_falling_anim);
        }
        efc.new_event(MOB_EVENT_LANDED); {
            efc.change_state("landing");
        }
    }
    efc.new_state("landing", DROP_STATE_LANDING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(drop_fsm::set_landing_anim);
        }
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.change_state("idling");
        }
    }
    efc.new_state("bumped", DROP_STATE_BUMPED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(drop_fsm::set_bumped_anim);
        }
        efc.new_event(MOB_EVENT_TOUCHED_OBJECT); {
            efc.run(drop_fsm::on_touched);
        }
        efc.new_event(MOB_EVENT_ANIMATION_END); {
            efc.change_state("idling");
        }
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
 * What to do when the drop is touched.
 */
void drop_fsm::on_touched(mob* m, void* info1, void* info2) {
    drop* d_ptr = (drop*) m;
    mob* toucher = (mob*) info1;
    bool will_drink = false;
    
    if(d_ptr->doses_left == 0) return;
    
    //Check if a compatible mob touched it.
    if(
        d_ptr->dro_type->consumer == DROP_CONSUMER_PIKMIN &&
        toucher->type->category->id == MOB_CATEGORY_PIKMIN
    ) {
    
        //Pikmin is about to drink it.
        pikmin* p_ptr = (pikmin*) toucher;
        
        if(
            d_ptr->dro_type->effect == DROP_EFFECT_MATURATE &&
            p_ptr->maturity < N_MATURITIES - 1
        ) {
            will_drink = true;
        } else if(d_ptr->dro_type->effect == DROP_EFFECT_GIVE_STATUS) {
            will_drink = true;
        }
        
        
    } else if(
        d_ptr->dro_type->consumer == DROP_CONSUMER_LEADERS &&
        toucher->type->category->id == MOB_CATEGORY_LEADERS
    ) {
    
        //Leader is about to drink it.
        leader* l_ptr = (leader*) toucher;
        if(d_ptr->dro_type->effect == DROP_EFFECT_INCREASE_SPRAYS) {
            will_drink = true;
        } else if(d_ptr->dro_type->effect == DROP_EFFECT_GIVE_STATUS) {
            will_drink = true;
        }
        
    }
    
    mob_event* ev = NULL;
    
    if(will_drink) {
        ev = q_get_event(toucher, MOB_EVENT_TOUCHED_DROP);
    }
    
    if(!ev) {
        //Turns out it can't drink in this state after all.
        will_drink = false;
    }
    
    if(will_drink) {
        ev->run(toucher, (void*) m);
        d_ptr->doses_left--;
    } else {
        //This mob won't drink it. Just a bump.
        if(m->fsm.cur_state->id != DROP_STATE_BUMPED) {
            m->fsm.set_state(DROP_STATE_BUMPED, info1, info2);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Sets the animation to the "bumped" one.
 */
void drop_fsm::set_bumped_anim(mob* m, void* info1, void* info2) {
    m->set_animation(DROP_ANIM_BUMPED);
}


/* ----------------------------------------------------------------------------
 * Sets the animation to the "falling" one.
 */
void drop_fsm::set_falling_anim(mob* m, void* info1, void* info2) {
    m->set_animation(DROP_ANIM_FALLING);
}


/* ----------------------------------------------------------------------------
 * Sets the standard "idling" animation.
 */
void drop_fsm::set_idling_anim(mob* m, void* info1, void* info2) {
    m->set_animation(DROP_ANIM_IDLING);
}


/* ----------------------------------------------------------------------------
 * Sets the animation to the "landing" one.
 */
void drop_fsm::set_landing_anim(mob* m, void* info1, void* info2) {
    m->set_animation(DROP_ANIM_LANDING);
}
