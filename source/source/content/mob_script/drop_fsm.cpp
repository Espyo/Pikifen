/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Drop finite state machine logic.
 */

#include "drop_fsm.h"

#include "../../core/misc_functions.h"
#include "../../util/string_utils.h"
#include "../mob/drop.h"


/**
 * @brief Creates the finite state machine for the drop's logic.
 *
 * @param typ Mob type to create the finite state machine for.
 */
void drop_fsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    efc.newState("falling", DROP_STATE_FALLING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(drop_fsm::setFallingAnim);
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.changeState("landing");
        }
    }
    efc.newState("landing", DROP_STATE_LANDING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(drop_fsm::land);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("idling");
        }
    }
    efc.newState("idling", DROP_STATE_IDLING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(drop_fsm::setIdlingAnim);
        }
        efc.newEvent(MOB_EV_TOUCHED_OBJECT); {
            efc.run(drop_fsm::onTouched);
        }
    }
    efc.newState("bumped", DROP_STATE_BUMPED); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(drop_fsm::setBumpedAnim);
        }
        efc.newEvent(MOB_EV_TOUCHED_OBJECT); {
            efc.run(drop_fsm::onTouched);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("idling");
        }
    }
    
    
    typ->states = efc.finish();
    typ->first_state_idx = fixStates(typ->states, "falling", typ);
    
    //Check if the number in the enum and the total match up.
    engineAssert(
        typ->states.size() == N_DROP_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_DROP_STATES) + " in enum."
    );
}


/**
 * @brief When the drop lands on the floor.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void drop_fsm::land(Mob* m, void* info1, void* info2) {
    m->stopChasing();
    m->setAnimation(DROP_ANIM_LANDING);
}


/**
 * @brief What to do when the drop is touched.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void drop_fsm::onTouched(Mob* m, void* info1, void* info2) {
    Drop* dro_ptr = (Drop*) m;
    Mob* toucher = (Mob*) info1;
    bool will_drink = false;
    
    if(dro_ptr->doses_left == 0) return;
    
    //Check if a compatible mob touched it.
    if(
        dro_ptr->dro_type->consumer == DROP_CONSUMER_PIKMIN &&
        toucher->type->category->id == MOB_CATEGORY_PIKMIN
    ) {
    
        //Pikmin is about to drink it.
        Pikmin* pik_ptr = (Pikmin*) toucher;
        
        switch(dro_ptr->dro_type->effect) {
        case DROP_EFFECT_MATURATE: {
            if(pik_ptr->maturity < N_MATURITIES - 1) {
                will_drink = true;
            }
            break;
        } case DROP_EFFECT_GIVE_STATUS: {
            will_drink = true;
            break;
        } default: {
            break;
        }
        }
        
    } else if(
        dro_ptr->dro_type->consumer == DROP_CONSUMER_LEADERS &&
        toucher->type->category->id == MOB_CATEGORY_LEADERS
    ) {
    
        //Leader is about to drink it.
        switch(dro_ptr->dro_type->effect) {
        case DROP_EFFECT_INCREASE_SPRAYS:
        case DROP_EFFECT_GIVE_STATUS: {
            will_drink = true;
            break;
        } default: {
            break;
        }
        }
        
    }
    
    MobEvent* ev = nullptr;
    
    if(will_drink) {
        ev = toucher->fsm.getEvent(MOB_EV_TOUCHED_DROP);
    }
    
    if(!ev) {
        //Turns out it can't drink in this state after all.
        will_drink = false;
    }
    
    if(will_drink) {
        ev->run(toucher, (void*) m);
        dro_ptr->doses_left--;
    } else {
        //This mob won't drink it. Just a bump.
        bool toucher_is_moving =
            toucher->speed.x != 0 || toucher->speed.y != 0 ||
            toucher->chase_info.state == CHASE_STATE_CHASING;
        if(
            m->fsm.cur_state->id != DROP_STATE_BUMPED &&
            toucher_is_moving
        ) {
            m->fsm.setState(DROP_STATE_BUMPED, info1, info2);
        }
    }
}


/**
 * @brief Sets the animation to the "bumped" one.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void drop_fsm::setBumpedAnim(Mob* m, void* info1, void* info2) {
    m->setAnimation(DROP_ANIM_BUMPED);
}


/**
 * @brief Sets the animation to the "falling" one.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void drop_fsm::setFallingAnim(Mob* m, void* info1, void* info2) {
    m->setAnimation(
        DROP_ANIM_FALLING,
        START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
}


/**
 * @brief Sets the standard "idling" animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void drop_fsm::setIdlingAnim(Mob* m, void* info1, void* info2) {
    m->setAnimation(DROP_ANIM_IDLING);
}
