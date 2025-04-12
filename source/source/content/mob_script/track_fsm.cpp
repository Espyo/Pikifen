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

#include "../../core/misc_functions.h"
#include "../../util/general_utils.h"
#include "../../util/string_utils.h"
#include "../mob/track.h"


/**
 * @brief Creates the finite state machine for the track's logic.
 *
 * @param typ Mob type to create the finite state machine for.
 */
void track_fsm::createFsm(MobType* typ) {
    EasyFsmCreator efc;
    efc.newState("idling", TRACK_STATE_IDLING); {
        efc.newEvent(MOB_EV_ON_ENTER); {
            efc.run(track_fsm::spawn);
        }
        efc.newEvent(MOB_EV_TOUCHED_OBJECT); {
            efc.run(track_fsm::onTouched);
        }
    }
    
    
    typ->states = efc.finish();
    typ->firstStateIdx = fixStates(typ->states, "idling", typ);
    
    //Check if the number in the enum and the total match up.
    engineAssert(
        typ->states.size() == N_TRACK_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_TRACK_STATES) + " in enum."
    );
}


/**
 * @brief What to do when the track is touched.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void track_fsm::onTouched(Mob* m, void* info1, void* info2) {
    Track* tra_ptr = (Track*) m;
    Mob* toucher = (Mob*) info1;
    
    MobEvent* ev = nullptr;
    
    //Check if a compatible mob touched it.
    if(
        hasFlag(tra_ptr->traType->riders, TRACK_RIDER_FLAG_PIKMIN) &&
        toucher->type->category->id == MOB_CATEGORY_PIKMIN
    ) {
    
        //Pikmin is about to ride it.
        ev = toucher->fsm.getEvent(MOB_EV_TOUCHED_TRACK);
        
    } else if(
        hasFlag(tra_ptr->traType->riders, TRACK_RIDER_FLAG_LEADERS) &&
        toucher->type->category->id == MOB_CATEGORY_LEADERS
    ) {
    
        //Leader is about to ride it.
        ev = toucher->fsm.getEvent(MOB_EV_TOUCHED_TRACK);
        
    }
    
    if(!ev) return;
    
    ev->run(toucher, (void*) m);
}


/**
 * @brief When the track spawns.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void track_fsm::spawn(Mob* m, void* info1, void* info2) {
    m->setAnimation(
        TRACK_ANIM_IDLING, START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
}
