/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Treasure finite state machine logic.
 */

#include "../functions.h"
#include "mob_fsm.h"
#include "treasure.h"
#include "treasure_fsm.h"
#include "../utils/string_utils.h"
#include "ship.h"

/* ----------------------------------------------------------------------------
 * Creates the finite state machine for the treasure's logic.
 */
void treasure_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;
    
    efc.new_state("idle_waiting", TREASURE_STATE_IDLE_WAITING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_stop_move);
        }
        efc.new_event(MOB_EVENT_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
            efc.run(gen_mob_fsm::check_carry_begin);
        }
        efc.new_event(MOB_EVENT_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
        }
        efc.new_event(MOB_EVENT_CARRY_BEGIN_MOVE); {
            efc.change_state("idle_moving");
        }
    }
    
    efc.new_state("idle_moving", TREASURE_STATE_IDLE_MOVING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_begin_move);
        }
        efc.new_event(MOB_EVENT_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
            efc.run(gen_mob_fsm::check_carry_begin);
        }
        efc.new_event(MOB_EVENT_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
            efc.run(gen_mob_fsm::check_carry_begin);
            efc.run(gen_mob_fsm::check_carry_stop);
        }
        efc.new_event(MOB_EVENT_CARRY_STOP_MOVE); {
            efc.change_state("idle_waiting");
        }
        efc.new_event(MOB_EVENT_CARRY_BEGIN_MOVE); {
            efc.run(gen_mob_fsm::carry_begin_move);
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.run(gen_mob_fsm::carry_reach_destination);
        }
        efc.new_event(MOB_EVENT_CARRY_DELIVERED); {
            efc.change_state("being_delivered");
        }
        efc.new_event(MOB_EVENT_CARRY_STUCK); {
            efc.change_state("idle_stuck");
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(treasure_fsm::respawn);
        }
    }
    
    efc.new_state("idle_stuck", TREASURE_STATE_IDLE_STUCK); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(gen_mob_fsm::carry_become_stuck);
        }
        efc.new_event(MOB_EVENT_ON_LEAVE); {
            efc.run(gen_mob_fsm::carry_stop_being_stuck);
        }
        efc.new_event(MOB_EVENT_CARRIER_ADDED); {
            efc.run(gen_mob_fsm::handle_carrier_added);
        }
        efc.new_event(MOB_EVENT_CARRIER_REMOVED); {
            efc.run(gen_mob_fsm::handle_carrier_removed);
            efc.run(gen_mob_fsm::check_carry_stop);
        }
        efc.new_event(MOB_EVENT_CARRY_STOP_MOVE); {
            efc.change_state("idle_waiting");
        }
        efc.new_event(MOB_EVENT_CARRY_BEGIN_MOVE); {
            efc.change_state("idle_moving");
        }
        efc.new_event(MOB_EVENT_BOTTOMLESS_PIT); {
            efc.run(treasure_fsm::respawn);
        }
    }
    
    efc.new_state("being_delivered", TREASURE_STATE_BEING_DELIVERED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run(gen_mob_fsm::start_being_delivered);
        }
        efc.new_event(MOB_EVENT_TIMER); {
            efc.run(gen_mob_fsm::handle_delivery);
        }
    }
    
    
    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idle_waiting");
    
    //Check if the number in the enum and the total match up.
    engine_assert(
        typ->states.size() == N_TREASURE_STATES,
        i2s(typ->states.size()) + " registered, " +
        i2s(N_TREASURE_STATES) + " in enum."
    );
}


/* ----------------------------------------------------------------------------
 * When a treasure falls into a bottomless pit and should respawn.
 */
void treasure_fsm::respawn(mob* m, void* info1, void* info2) {
    m->become_uncarriable(); //Force all Pikmin to let go.
    m->become_carriable(CARRY_DESTINATION_SHIP);
    m->respawn();
}
