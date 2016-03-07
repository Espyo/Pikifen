/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Onion type class and Onion type-related functions.
 */

#include "../functions.h"
#include "onion_type.h"
#include "../vars.h"


onion_type::onion_type() :
    mob_type(),
    pik_type(NULL) {
    
    init_script();
}


void onion_type::load_from_file(data_node* file, const bool load_resources, vector<pair<size_t, string> >* anim_conversions) {
    data_node* pik_type_node = file->get_child_by_name("pikmin_type");
    if(pikmin_types.find(pik_type_node->value) == pikmin_types.end()) {
        error_log("Unknown Pikmin type \"" + pik_type_node->value + "\"!", pik_type_node);
    }
    pik_type = pikmin_types[pik_type_node->value];
    
    anim_conversions->push_back(make_pair(ANIM_IDLE, "idle"));
}


void onion_type::init_script() {
    easy_fsm_creator efc;
    
    efc.new_state("idle", ONION_STATE_IDLE); {
        efc.new_event(MOB_EVENT_RECEIVE_DELIVERY); {
            efc.run_function(onion::fsm_receive_mob);
        }
    }
    
    states = efc.finish();
    first_state_nr = fix_states(states, "idle");
    
    if(states.size() != N_ONION_STATES) {
        error_log(
            "ENGINE WARNING: Number of Onion states on the FSM (" + i2s(states.size()) +
            ") and the enum (" + i2s(N_ONION_STATES) + ") do not match.");
    }
}
