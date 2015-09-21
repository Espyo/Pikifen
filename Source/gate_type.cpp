/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Gate type class and gate type-related functions.
 */

#include "gate.h"
#include "gate_type.h"
#include "mob_script.h"

gate_type::gate_type() {
    casts_shadow = false;
    init_script();
}

void gate_type::init_script() {
    easy_fsm_creator efc;
    efc.new_state("idle", 0); {
        efc.new_event(MOB_EVENT_ON_ENTER); {
            efc.run_function(gate::set_anim);
        }
        efc.new_event(MOB_EVENT_HITBOX_TOUCH_N_A); {
            efc.run_function(gate::take_damage);
        }
        efc.new_event(MOB_EVENT_DEATH); {
            efc.run_function(gate::open);
            efc.change_state("dead");
        }
    }
    efc.new_state("dead", 1); {
    
    }
    states = efc.finish();
    first_state_nr = fix_states(states, "idle");
}

void gate_type::load_from_file(data_node* file, const bool load_resources, vector<pair<size_t, string> >* anim_conversions) {
    anim_conversions->push_back(make_pair(GATE_ANIM_IDLE, "idle"));
    anim_conversions->push_back(make_pair(GATE_ANIM_NOTHING, "nothing"));
}