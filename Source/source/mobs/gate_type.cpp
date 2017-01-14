/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Gate type class and gate type-related functions.
 */

#include "gate.h"
#include "gate_fsm.h"
#include "gate_type.h"
#include "../mob_script.h"

/* ----------------------------------------------------------------------------
 * Creates a type of gate.
 */
gate_type::gate_type() :
    mob_type() {
    
    casts_shadow = false;
    is_obstacle = true;
    
    gate_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Loads data about the gate type from a data file.
 */
void gate_type::load_from_file(
    data_node* file, const bool load_resources,
    vector<pair<size_t, string> >* anim_conversions
) {
    anim_conversions->push_back(make_pair(GATE_ANIM_IDLING, "idling"));
    anim_conversions->push_back(make_pair(GATE_ANIM_DESTROYED, "destroyed"));
}
