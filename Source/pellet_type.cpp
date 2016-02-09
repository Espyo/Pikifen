/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pellet type class and pellet type-related functions.
 */

#include "functions.h"
#include "pellet_type.h"
#include "vars.h"

pellet_type::pellet_type() :
    mob_type(),
    pik_type(nullptr),
    number(0),
    match_seeds(0),
    non_match_seeds(0),
    bmp_number(nullptr) {
    
    init_script();
    
    move_speed = 60; //TODO
}

void pellet_type::load_from_file(data_node* file, const bool load_resources, vector<pair<size_t, string> >* anim_conversions) {
    data_node* pik_type_node = file->get_child_by_name("pikmin_type");
    if(pikmin_types.find(pik_type_node->value) == pikmin_types.end()) {
        error_log("Unknown Pikmin type \"" + pik_type_node->value + "\"!", pik_type_node);
    }
    
    pik_type = pikmin_types[pik_type_node->value];
    number = s2i(file->get_child_by_name("number")->value);
    weight = number;
    match_seeds = s2i(file->get_child_by_name("match_seeds")->value);
    non_match_seeds = s2i(file->get_child_by_name("non_match_seeds")->value);
    
    if(load_resources) {
        bmp_number = bitmaps.get(file->get_child_by_name("number_image")->value, file);
    }
    
    anim_conversions->push_back(make_pair(ANIM_IDLE, "idle"));
}


void pellet_type::init_script() {
    easy_fsm_creator efc;
    
    efc.new_state("idle", PELLET_STATE_IDLE); {
        efc.new_event(MOB_EVENT_CARRIER_ADDED); {
            efc.run_function(mob::handle_carrier_added);
        }
        efc.new_event(MOB_EVENT_CARRIER_REMOVED); {
            efc.run_function(mob::handle_carrier_removed);
        }
        efc.new_event(MOB_EVENT_CARRY_BEGIN_MOVE); {
            efc.run_function(mob::carry_begin_move);
            efc.run_function(mob::set_next_target);
        }
        efc.new_event(MOB_EVENT_CARRY_STOP_MOVE); {
            efc.run_function(mob::carry_stop_move);
        }
        efc.new_event(MOB_EVENT_CARRY_STUCK); {
            efc.run_function(mob::carry_stop_move);
        }
        efc.new_event(MOB_EVENT_REACHED_DESTINATION); {
            efc.run_function(mob::set_next_target);
        }
        efc.new_event(MOB_EVENT_CARRY_DELIVERED); {
            efc.run_function(mob::start_being_delivered);
            efc.change_state("being_delivered");
        }
    }
    
    efc.new_state("being_delivered", PELLET_STATE_BEING_DELIVERED); {
        efc.new_event(MOB_EVENT_TIMER); {
            efc.run_function(pellet::handle_delivery);
        }
    }
    
    
    states = efc.finish();
    first_state_nr = fix_states(states, "idle");
    carriable_state_id = PELLET_STATE_IDLE;
    
    if(states.size() != N_PELLET_STATES) {
        error_log(
            "ENGINE WARNING: Number of pellet states on the FSM (" + i2s(states.size()) +
            ") and the enum (" + i2s(N_PELLET_STATES) + ") do not match.");
    }
}
