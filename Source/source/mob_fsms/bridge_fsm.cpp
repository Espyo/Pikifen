/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Bridge finite state machine logic.
 */

#include "bridge_fsm.h"

#include "../functions.h"
#include "../game.h"
#include "../mobs/bridge.h"
#include "../utils/general_utils.h"
#include "../utils/string_utils.h"
#include "gen_mob_fsm.h"


/**
 * @brief Creates the finite state machine for the bridge's logic.
 *
 * @param typ Mob type to create the finite state machine for.
 */
void bridge_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    efc.new_state("idling", BRIDGE_STATE_IDLING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(bridge_fsm::set_anim);
        }
        efc.new_event(MOB_EV_ON_READY); {
            efc.run(bridge_fsm::setup);
        }
        efc.new_event(MOB_EV_HITBOX_TOUCH_N_A); {
            efc.run(gen_mob_fsm::be_attacked);
            efc.run(bridge_fsm::check_health);
        }
        efc.new_event(MOB_EV_FINISHED_RECEIVING_DELIVERY); {
            efc.run(bridge_fsm::check_health);
        }
        efc.new_event(MOB_EV_DEATH); {
            efc.run(bridge_fsm::check_health);
            efc.run(bridge_fsm::open);
            efc.change_state("destroyed");
        }
    }
    efc.new_state("creating_chunk", BRIDGE_STATE_CREATING_CHUNK); {
        //Sort of a dummy state for text file script enhancements.
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.change_state("idling");
        }
    }
    efc.new_state("destroyed", BRIDGE_STATE_DESTROYED); {
    
    }
    
    
    typ->states = efc.finish();
    typ->first_state_idx = fix_states(typ->states, "idling", typ);
    
    //Check if the number in the enum and the total match up.
    engine_assert(
        typ->states.size() == N_BRIDGE_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_BRIDGE_STATES) + " in enum."
    );
}


/**
 * @brief Makes the bridge check its health and update its chunks, if needed.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void bridge_fsm::check_health(mob* m, void* info1, void* info2) {
    bridge* bri_ptr = (bridge*) m;
    if(bri_ptr->check_health()) {
        m->fsm.set_state(BRIDGE_STATE_CREATING_CHUNK);
    }
}


/**
 * @brief Opens up the bridge.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void bridge_fsm::open(mob* m, void* info1, void* info2) {
    bridge* bri_ptr = (bridge*) m;
    bri_ptr->set_animation(BRIDGE_ANIM_DESTROYED);
    bri_ptr->start_dying();
    bri_ptr->finish_dying();
    enable_flag(bri_ptr->flags, MOB_FLAG_INTANGIBLE);
}


/**
 * @brief Sets the standard "idling" animation.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void bridge_fsm::set_anim(mob* m, void* info1, void* info2) {
    m->set_animation(
        BRIDGE_ANIM_IDLING, true, START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN
    );
}


/**
 * @brief Sets up the bridge with the data surrounding it,
 * like its linked destination object.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void bridge_fsm::setup(mob* m, void* info1, void* info2) {
    bridge* bri_ptr = (bridge*) m;
    bri_ptr->setup();
}
