/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Drop finite-state machine logic.
 */

#include "drop_fsm.h"

#include "../../core/misc_functions.h"
#include "../../util/string_utils.h"
#include "../mob/drop.h"


#pragma region FSM


/**
 * @brief Creates the finite-state machine for the drop's logic.
 *
 * @param typ Mob type to create the finite-state machine for.
 */
void DropFsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    efc.newState("falling", DROP_STATE_FALLING); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(DropFsm::setFallingAnim);
        }
        efc.newEvent(MOB_EV_LANDED); {
            efc.changeState("landing");
        }
    }
    efc.newState("landing", DROP_STATE_LANDING); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(DropFsm::land);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("idling");
        }
    }
    efc.newState("idling", DROP_STATE_IDLING); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(DropFsm::setIdlingAnim);
        }
        efc.newEvent(MOB_EV_TOUCHED_OBJECT); {
            efc.run(DropFsm::onTouched);
        }
    }
    efc.newState("bumped", DROP_STATE_BUMPED); {
        efc.newEvent(SCRIPT_EV_ON_ENTER); {
            efc.run(DropFsm::setBumpedAnim);
        }
        efc.newEvent(MOB_EV_TOUCHED_OBJECT); {
            efc.run(DropFsm::onTouched);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("idling");
        }
    }
    
    
    typ->states = efc.finish();
    typ->firstStateIdx = fixStates(typ->states, "falling", typ);
    
    //Check if the number in the enum and the total match up.
    engineAssert(
        typ->states.size() == N_DROP_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_DROP_STATES) + " in enum."
    );
}


#pragma endregion
#pragma region FSM functions


/**
 * @brief When the drop lands on the floor.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void DropFsm::land(Fsm* fsm, void* info1, void* info2) {
    Drop* droPtr = (Drop*) fsm->m;
    
    droPtr->stopChasing();
    droPtr->setAnimation(DROP_ANIM_LANDING);
}


/**
 * @brief What to do when the drop is touched.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void DropFsm::onTouched(Fsm* fsm, void* info1, void* info2) {
    Drop* droPtr = (Drop*) fsm->m;
    Mob* toucher = (Mob*) info1;

    bool willDrink = false;
    
    if(droPtr->dosesLeft == 0) return;
    
    //Check if a compatible mob touched it.
    if(
        droPtr->droType->consumer == DROP_CONSUMER_PIKMIN &&
        toucher->type->category->id == MOB_CATEGORY_PIKMIN
    ) {
    
        //Pikmin is about to drink it.
        Pikmin* pikPtr = (Pikmin*) toucher;
        
        switch(droPtr->droType->effect) {
        case DROP_EFFECT_MATURATE: {
            if(pikPtr->maturity < N_MATURITIES - 1) {
                willDrink = true;
            }
            break;
        } case DROP_EFFECT_GIVE_STATUS: {
            willDrink = true;
            break;
        } default: {
            break;
        }
        }
        
    } else if(
        droPtr->droType->consumer == DROP_CONSUMER_LEADERS &&
        toucher->type->category->id == MOB_CATEGORY_LEADERS
    ) {
    
        //Leader is about to drink it.
        switch(droPtr->droType->effect) {
        case DROP_EFFECT_INCREASE_SPRAYS:
        case DROP_EFFECT_GIVE_STATUS: {
            willDrink = true;
            break;
        } default: {
            break;
        }
        }
        
    }
    
    ScriptEvent* ev = nullptr;
    
    if(willDrink) {
        ev = toucher->fsm.getEvent(MOB_EV_TOUCHED_DROP);
    }
    
    if(!ev) {
        //Turns out it can't drink in this state after all.
        willDrink = false;
    }
    
    if(willDrink) {
        ev->run(&toucher->fsm, (void*) droPtr);
        droPtr->dosesLeft--;
    } else {
        //This mob won't drink it. Just a bump.
        bool toucherIsMoving =
            toucher->speed.x != 0 || toucher->speed.y != 0 ||
            toucher->chaseInfo.state == CHASE_STATE_CHASING;
        if(
            droPtr->fsm.curState->id != DROP_STATE_BUMPED &&
            toucherIsMoving
        ) {
            droPtr->fsm.setState(DROP_STATE_BUMPED, info1, info2);
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
void DropFsm::setBumpedAnim(Fsm* fsm, void* info1, void* info2) {
    Drop* droPtr = (Drop*) fsm->m;
    
    droPtr->setAnimation(DROP_ANIM_BUMPED);
}


/**
 * @brief Sets the animation to the "falling" one.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void DropFsm::setFallingAnim(Fsm* fsm, void* info1, void* info2) {
    Drop* droPtr = (Drop*) fsm->m;
    
    droPtr->setAnimation(
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
void DropFsm::setIdlingAnim(Fsm* fsm, void* info1, void* info2) {
    Drop* droPtr = (Drop*) fsm->m;
    
    droPtr->setAnimation(DROP_ANIM_IDLING);
}


#pragma endregion
