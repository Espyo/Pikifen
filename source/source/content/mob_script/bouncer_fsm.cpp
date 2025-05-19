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

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"
#include "../mob/bouncer.h"


/**
 * @brief Creates the finite state machine for the bouncer's logic.
 *
 * @param typ Mob type to create the finite state machine for.
 */
void BouncerFsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    efc.newState("idling", BOUNCER_STATE_IDLING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(BouncerFsm::setIdlingAnimation);
        }
        efc.newEvent(MOB_EV_RIDER_ADDED); {
            efc.run(BouncerFsm::handleMob);
        }
    }
    efc.newState("bouncing", BOUNCER_STATE_BOUNCING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(BouncerFsm::setBouncingAnimation);
        }
        efc.newEvent(MOB_EV_RIDER_ADDED); {
            efc.run(BouncerFsm::handleMob);
        }
        efc.newEvent(MOB_EV_ANIMATION_END); {
            efc.changeState("idling");
        }
    }
    
    
    typ->states = efc.finish();
    typ->firstStateIdx = fixStates(typ->states, "idling", typ);
    
    //Check if the number in the enum and the total match up.
    engineAssert(
        typ->states.size() == N_BOUNCER_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_BOUNCER_STATES) + " in enum."
    );
}


/**
 * @brief When something is on top of the bouncer.
 *
 * @param m The mob.
 * @param info1 Points to the mob that is on top of it.
 * @param info2 Unused.
 */
void BouncerFsm::handleMob(Mob* m, void* info1, void* info2) {
    Bouncer* bouPtr = (Bouncer*) m;
    Mob* toucher = (Mob*) info1;
    Mob* targetMob = nullptr;
    
    if(!bouPtr->link_anon_size ==0 && bouPtr->links["0"]) {
        targetMob = bouPtr->links["0"];
    }
    
    if(!targetMob) {
        game.errors.report(
            "The bouncer (" + getErrorMessageMobInfo(m) +
            ") has no linked mob to serve as a target!"
        );
        return;
    }
    
    MobEvent* ev = nullptr;
    
    //Check if a compatible mob touched it.
    if(
        hasFlag(bouPtr->bouType->riders, BOUNCER_RIDER_FLAG_PIKMIN) &&
        toucher->type->category->id == MOB_CATEGORY_PIKMIN
    ) {
    
        //Pikmin is about to be bounced.
        ev = toucher->fsm.getEvent(MOB_EV_TOUCHED_BOUNCER);
        
    } else if(
        hasFlag(bouPtr->bouType->riders, BOUNCER_RIDER_FLAG_LEADERS) &&
        toucher->type->category->id == MOB_CATEGORY_LEADERS
    ) {
    
        //Leader is about to be bounced.
        ev = toucher->fsm.getEvent(MOB_EV_TOUCHED_BOUNCER);
        
    } else if(
        hasFlag(bouPtr->bouType->riders, BOUNCER_RIDER_FLAG_PIKMIN) &&
        toucher->pathInfo &&
        hasFlag(
            toucher->pathInfo->settings.flags,
            PATH_FOLLOW_FLAG_LIGHT_LOAD
        )
    ) {
    
        //Pikmin carrying light load is about to be bounced.
        ev = toucher->fsm.getEvent(MOB_EV_TOUCHED_BOUNCER);
    }
    
    if(!ev) return;
    
    toucher->stopChasing();
    toucher->leaveGroup();
    enableFlag(toucher->flags, MOB_FLAG_WAS_THROWN);
    toucher->startHeightEffect();
    
    float angle;
    //The maximum height has a guaranteed minimum (useful if the destination
    //is below the bouncer), and scales up with how much higher the thrown
    //mob needs to go, to make for a nice smooth arc.
    float maxH = std::max(128.0f, (targetMob->z - toucher->z) * 1.5f);
    calculateThrow(
        toucher->pos,
        toucher->z,
        targetMob->pos,
        targetMob->z + targetMob->height,
        maxH, MOB::GRAVITY_ADDER,
        &toucher->speed,
        &toucher->speedZ,
        &angle
    );
    
    toucher->face(angle, nullptr, true);
    
    ev->run(toucher, (void*) m);
    
    m->fsm.setState(BOUNCER_STATE_BOUNCING, info1, info2);
}


/**
 * @brief When it must change to the bouncing animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void BouncerFsm::setBouncingAnimation(Mob* m, void* info1, void* info2) {
    m->setAnimation(BOUNCER_ANIM_BOUNCING);
}


/**
 * @brief When it must change to the idling animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void BouncerFsm::setIdlingAnimation(Mob* m, void* info1, void* info2) {
    m->setAnimation(
        BOUNCER_ANIM_IDLING, START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
}
