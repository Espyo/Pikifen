/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Onion type class and Onion type-related functions.
 */

#include "../functions.h"
#include "onion.h"
#include "onion_fsm.h"
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
 * Loads data about the Onion type from a data file.
 */
void onion_type::load_from_file(
    data_node* file, const bool load_resources,
    vector<pair<size_t, string> >* anim_conversions
) {
    data_node* pik_type_node = file->get_child_by_name("pikmin_type");
    if(pikmin_types.find(pik_type_node->value) == pikmin_types.end()) {
        log_error(
            "Unknown Pikmin type \"" + pik_type_node->value + "\"!",
            pik_type_node
        );
    }
    pik_type = pikmin_types[pik_type_node->value];
    
    anim_conversions->push_back(make_pair(ANIM_IDLING, "idling"));
}
