/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pellet finite state machine logic.
 */

#include "pellet_fsm.h"

#include "../functions.h"
#include "../mobs/onion.h"
#include "../mobs/pellet.h"
#include "../utils/string_utils.h"
#include "gen_mob_fsm.h"


/* ----------------------------------------------------------------------------
 * Creates the finite state machine for the pellet's logic.
 * typ:
 *   Mob type to create the finite state machine for.
 */
void pellet_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    
    efc.new_state("idle_waiting", PELLET_STATE_IDLE_WAITING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_stop_move);
        }
        efc.new_event(MOB_EV_LANDED); {
            efc.run(pellet_fsm::stand_still);
        }
        efc.new_event(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
        }
        efc.new_event(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
        }
        efc.new_event(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carry_get_path);
            efc.change_state("idle_moving");
        }
    }
    
    efc.new_state("idle_moving", PELLET_STATE_IDLE_MOVING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_begin_move);
        }
        efc.new_event(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
        }
        efc.new_event(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
        }
        efc.new_event(MOB_EV_CARRY_STOP_MOVE); {
            efc.change_state("idle_waiting");
        }
        efc.new_event(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carry_get_path);
            efc.run(gen_mob_fsm::carry_begin_move);
        }
        efc.new_event(MOB_EV_REACHED_DESTINATION); {
            efc.run(gen_mob_fsm::carry_reach_destination);
        }
        efc.new_event(MOB_EV_PATH_BLOCKED); {
            efc.change_state("idle_stuck");
        }
        efc.new_event(MOB_EV_PATHS_CHANGED); {
            efc.run(gen_mob_fsm::carry_get_path);
            efc.run(gen_mob_fsm::carry_begin_move);
        }
        efc.new_event(MOB_EV_CARRY_DELIVERED); {
            efc.change_state("being_delivered");
        }
    }
    
    efc.new_state("idle_stuck", PELLET_STATE_IDLE_STUCK); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_become_stuck);
        }
        efc.new_event(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
        }
        efc.new_event(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
        }
        efc.new_event(MOB_EV_CARRY_STOP_MOVE); {
            efc.run(gen_mob_fsm::carry_stop_being_stuck);
            efc.change_state("idle_waiting");
        }
        efc.new_event(MOB_EV_PATHS_CHANGED); {
            efc.run(gen_mob_fsm::carry_stop_being_stuck);
            efc.run(gen_mob_fsm::carry_get_path);
            efc.change_state("idle_moving");
        }
    }
    
    efc.new_state("being_delivered", PELLET_STATE_BEING_DELIVERED); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::start_being_delivered);
        }
        efc.new_event(MOB_EV_TIMER); {
            efc.run(gen_mob_fsm::handle_delivery);
        }
    }
    
    
    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idle_waiting");
    
    //Check if the number in the enum and the total match up.
    engine_assert(
        typ->states.size() == N_PELLET_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_PELLET_STATES) + " in enum."
    );
}


/* ----------------------------------------------------------------------------
 * When the pellet should lose its momentum and stand still.
 * m:
 *   The mob.
 * info1:
 *   Unused.
 * info2:
 *   Unused.
 */
void pellet_fsm::stand_still(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->stop_turning();
}
