/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Ship type class and ship type-related functions.
 */

#include "functions.h"
#include "ship.h"
#include "ship_type.h"

ship_type::ship_type() :
    mob_type(),
    can_heal(false) {
    
    init_script();
    always_active = true;
}


void ship_type::load_from_file(data_node* file, const bool load_resources, vector<pair<size_t, string> >* anim_conversions) {
    can_heal = file->get_child_by_name("can_heal");
}


void ship_type::init_script() {
    easy_fsm_creator efc;
    
    efc.new_state("idle", SHIP_STATE_IDLE); {
        efc.new_event(MOB_EVENT_RECEIVE_DELIVERY); {
            efc.run_function(ship::receive_mob);
        }
    }
    
    states = efc.finish();
    first_state_nr = fix_states(states, "idle");
    
    if(states.size() != N_SHIP_STATES) {
        error_log(
            "ENGINE WARNING: Number of ship states on the FSM (" + i2s(states.size()) +
            ") and the enum (" + i2s(N_SHIP_STATES) + ") do not match.");
    }
}
