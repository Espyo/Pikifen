/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Treasure finite state machine logic.
 */

#include "treasure_fsm.h"

#include "../../core/misc_functions.h"
#include "../../util/string_utils.h"
#include "../mob/ship.h"
#include "../mob/treasure.h"
#include "gen_mob_fsm.h"


/**
 * @brief Creates the finite state machine for the treasure's logic.
 *
 * @param typ Mob type to create the finite state machine for.
 */
void treasure_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    
    efc.new_state("idle_waiting", TREASURE_STATE_IDLE_WAITING); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_stop_move);
        }
        efc.new_event(MOB_EV_LANDED); {
            efc.run(treasure_fsm::stand_still);
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
    
    efc.new_state("idle_moving", TREASURE_STATE_IDLE_MOVING); {
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
        efc.new_event(MOB_EV_CARRY_DELIVERED); {
            efc.change_state("being_delivered");
        }
        efc.new_event(MOB_EV_PATH_BLOCKED); {
            efc.change_state("idle_stuck");
        }
        efc.new_event(MOB_EV_PATHS_CHANGED); {
            efc.run(gen_mob_fsm::carry_get_path);
            efc.run(gen_mob_fsm::carry_begin_move);
        }
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(treasure_fsm::respawn);
        }
        efc.new_event(MOB_EV_TOUCHED_BOUNCER); {
            efc.change_state("idle_thrown");
        }
    }
    
    efc.new_state("idle_stuck", TREASURE_STATE_IDLE_STUCK); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_become_stuck);
        }
        efc.new_event(MOB_EV_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
        }
        efc.new_event(MOB_EV_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
        }
        efc.new_event(MOB_EV_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carry_stop_being_stuck);
            efc.run(gen_mob_fsm::carry_get_path);
            efc.change_state("idle_moving");
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
        efc.new_event(MOB_EV_BOTTOMLESS_PIT); {
            efc.run(treasure_fsm::respawn);
        }
    }
    
    efc.new_state("idle_thrown", TREASURE_STATE_IDLE_THROWN); {
        efc.new_event(MOB_EV_LANDED); {
            efc.run(gen_mob_fsm::lose_momentum);
            efc.run(gen_mob_fsm::carry_get_path);
            efc.change_state("idle_moving");
        }
    }
    
    efc.new_state("being_delivered", TREASURE_STATE_BEING_DELIVERED); {
        efc.new_event(MOB_EV_ON_ENTER); {
            efc.run(gen_mob_fsm::start_being_delivered);
        }
        efc.new_event(MOB_EV_TIMER); {
            efc.run(gen_mob_fsm::handle_delivery);
        }
    }
    
    
    typ->states = efc.finish();
    typ->first_state_idx = fix_states(typ->states, "idle_waiting", typ);
    
    //Check if the number in the enum and the total match up.
    engine_assert(
        typ->states.size() == N_TREASURE_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_TREASURE_STATES) + " in enum."
    );
}


/**
 * @brief When a treasure falls into a bottomless pit and should respawn.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void treasure_fsm::respawn(mob* m, void* info1, void* info2) {
    m->become_uncarriable(); //Force all Pikmin to let go.
    m->become_carriable(CARRY_DESTINATION_SHIP);
    m->respawn();
}


/**
 * @brief When the treasure should lose its momentum and stand still.
 *
 * @param m The mob.
 * @param info1 Unused.
 * @param info2 Unused.
 */
void treasure_fsm::stand_still(mob* m, void* info1, void* info2) {
    m->stop_chasing();
    m->stop_turning();
}
