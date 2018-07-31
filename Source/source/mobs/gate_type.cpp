/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
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
    mob_type(MOB_CATEGORY_GATES) {
    
    casts_shadow = false;
    is_obstacle = true;
    blocks_carrier_pikmin = true;
    projectiles_can_damage = false;
    pushes = true;
    
    gate_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector gate_type::get_anim_conversions() {
    anim_conversion_vector v;
    v.push_back(make_pair(GATE_ANIM_IDLING, "idling"));
    v.push_back(make_pair(GATE_ANIM_DESTROYED, "destroyed"));
    return v;
}
