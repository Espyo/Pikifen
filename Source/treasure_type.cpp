/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Treasure type class and treasure type-related functions.
 */

#include "functions.h"
#include "treasure.h"
#include "treasure_type.h"

//TODO bottomless pits.

treasure_type::treasure_type() :
    mob_type(),
    value(0) {
    
    init_script();
}


void treasure_type::load_from_file(data_node* file, const bool load_resources, vector<pair<size_t, string> >* anim_conversions) {

    value = s2f(file->get_child_by_name("value")->value);
    
    anim_conversions->push_back(make_pair(ANIM_IDLE, "idle"));
}


void treasure_type::init_script() {
    easy_fsm_creator efc;
    
    efc.new_state("idle_waiting", TREASURE_STATE_IDLE_WAITING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(mob::carry_stop_move);
        }
        efc.new_event(MOB_EVENT_CARRY_KEEP_GOING); {
            efc.run_function(mob::check_carry_begin);
        }
        efc.new_event(MOB_EVENT_CARRIER_ADDED); {
            efc.run_function(mob::handle_carrier_added);
            efc.run_function(mob::check_carry_begin);
        }
        efc.new_event(MOB_EVENT_CARRIER_REMOVED); {
            efc.run_function(mob::handle_carrier_removed);
        }
        efc.new_event(MOB_EVENT_CARRY_BEGIN_MOVE); {
            efc.change_state("idle_moving");
        }
    }
    
    efc.new_state("idle_moving", TREASURE_STATE_IDLE_MOVING); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(mob::carry_begin_move);
            efc.run_function(mob::set_next_target);
        }
        efc.new_event(MOB_EVENT_CARRIER_REMOVED); {
            efc.run_function(mob::handle_carrier_removed);
            efc.run_function(mob::check_carry_stop);
        }
        efc.new_event(MOB_EVENT_CARRY_WAIT_UP); {
            efc.change_state("idle_waiting");
        }
        efc.new_event(MOB_EVENT_CARRY_STOP_MOVE); {
            efc.change_state("idle_waiting");
        }
        efc.new_event(MOB_EVENT_CARRY_BEGIN_MOVE); {
            efc.run_function(mob::carry_begin_move);
            efc.run_function(mob::set_next_target);
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.run_function(mob::set_next_target);
        }
        efc.new_event(MOB_EVENT_CARRY_DELIVERED); {
            efc.change_state("being_delivered");
        }
    }
    
    efc.new_state("being_delivered", TREASURE_STATE_BEING_DELIVERED); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(mob::start_being_delivered);
        }
        efc.new_event(MOB_EVENT_TIMER); {
            efc.run_function(treasure::handle_delivery);
        }
    }
    
    
    states = efc.finish();
    first_state_nr = fix_states(states, "idle_waiting");
    
    if(states.size() != N_TREASURE_STATES) {
        error_log(
            "ENGINE WARNING: Number of treasure states on the FSM (" + i2s(states.size()) +
            ") and the enum (" + i2s(N_TREASURE_STATES) + ") do not match.");
    }
}
