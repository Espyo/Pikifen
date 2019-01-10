/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Onion type class and Onion type-related functions.
 */

#include "../functions.h"
#include "../mobs/onion.h"
#include "../mobs/onion_fsm.h"
#include "onion_type.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a type of Onion.
 */
onion_type::onion_type() :
    mob_type(MOB_CATEGORY_ONIONS),
    pik_type(NULL) {
    
    onion_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Loads parameters from a data file.
 */
void onion_type::load_parameters(data_node* file) {
    data_node* pik_type_node = file->get_child_by_name("pikmin_type");
    if(pikmin_types.find(pik_type_node->value) == pikmin_types.end()) {
        log_error(
            "Unknown Pikmin type \"" + pik_type_node->value + "\"!",
            pik_type_node
        );
    }
    pik_type = pikmin_types[pik_type_node->value];
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector onion_type::get_anim_conversions() {
    anim_conversion_vector v;
    v.push_back(make_pair(ANIM_IDLING, "idling"));
    return v;
}


onion_type::~onion_type() { }
