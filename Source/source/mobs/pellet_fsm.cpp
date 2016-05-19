/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pellet finite state machine logic.
 */

#include "../functions.h"
#include "mob_fsm.h"
#include "onion.h"
#include "pellet.h"
#include "pellet_fsm.h"

/* ----------------------------------------------------------------------------
 * Creates the finite state machine for the pellet's logic.
 */
void pellet_fsm::create_fsm(mob_type* typ) {
    easy_fsm_creator efc;

    efc.new_state("idle_waiting", PELLET_STATE_IDLE_WAITING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(gen_mob_fsm::carry_stop_move);
        }
        efc.new_event(MOB_EVENT_CARRY_KEEP_GOING); {
            efc.run_function(gen_mob_fsm::check_carry_begin);
        }
        efc.new_event(MOB_EVENT_CARRIER_ADDED); {
            efc.run_function(gen_mob_fsm::handle_carrier_added);
            efc.run_function(gen_mob_fsm::check_carry_begin);
        }
        efc.new_event(MOB_EVENT_CARRIER_REMOVED); {
            efc.run_function(gen_mob_fsm::handle_carrier_removed);
        }
        efc.new_event(MOB_EVENT_CARRY_BEGIN_MOVE); {
            efc.change_state("idle_moving");
        }
    }

    efc.new_state("idle_moving", PELLET_STATE_IDLE_MOVING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(gen_mob_fsm::carry_begin_move);
            efc.run_function(gen_mob_fsm::set_next_target);
        }
        efc.new_event(MOB_EVENT_CARRIER_REMOVED); {
            efc.run_function(gen_mob_fsm::handle_carrier_removed);
            efc.run_function(gen_mob_fsm::check_carry_stop);
        }
        efc.new_event(MOB_EVENT_CARRY_WAIT_UP); {
            efc.change_state("idle_waiting");
        }
        efc.new_event(MOB_EVENT_CARRY_STOP_MOVE); {
            efc.change_state("idle_waiting");
        }
        efc.new_event(MOB_EVENT_CARRY_BEGIN_MOVE); {
            efc.run_function(gen_mob_fsm::carry_begin_move);
            efc.run_function(gen_mob_fsm::set_next_target);
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.run_function(gen_mob_fsm::set_next_target);
        }
        efc.new_event(MOB_EVENT_CARRY_DELIVERED); {
            efc.change_state("being_delivered");
        }
    }

    efc.new_state("being_delivered", PELLET_STATE_BEING_DELIVERED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(gen_mob_fsm::start_being_delivered);
        }
        efc.new_event(MOB_EVENT_TIMER); {
            efc.run_function(pellet_fsm::handle_delivery);
        }
    }


    typ->states = efc.finish();
    typ->first_state_nr = fix_states(typ->states, "idle_waiting");

    if(typ->states.size() != N_PELLET_STATES) {
        log_error(
            "ENGINE WARNING: Number of pellet states on the FSM (" +
            i2s(typ->states.size()) +
            ") and the enum (" + i2s(N_PELLET_STATES) + ") do not match."
        );
    }
}


/* ----------------------------------------------------------------------------
 * When a pellet gets delivered to an Onion.
 */
void pellet_fsm::handle_delivery(mob* m, void* info1, void* info2) {
    size_t seeds = 0;
    pellet* p_ptr = (pellet*) m;
    onion* o_ptr = (onion*) p_ptr->carrying_target;

    if(p_ptr->pel_type->pik_type == o_ptr->oni_type->pik_type) {
        seeds = p_ptr->pel_type->match_seeds;
    } else {
        seeds = p_ptr->pel_type->non_match_seeds;
    }

    o_ptr->fsm.run_event(MOB_EVENT_RECEIVE_DELIVERY, (void*) seeds);

    p_ptr->to_delete = true;
}
