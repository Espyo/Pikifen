/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Onion type class and Onion type-related functions.
 */

#include "functions.h"
#include "onion_type.h"
#include "vars.h"


onion_type::onion_type() :
    mob_type(),
    pik_type(NULL) {
}


void onion_type::load_from_file(data_node* file, const bool load_resources, vector<pair<size_t, string> >* anim_conversions) {
    data_node* pik_type_node = file->get_child_by_name("pikmin_type");
    if(pikmin_types.find(pik_type_node->value) == pikmin_types.end()) {
        error_log("Unknown Pikmin type \"" + pik_type_node->value + "\"!", pik_type_node);
    }
    pik_type = pikmin_types[pik_type_node->value];
    
    anim_conversions->push_back(make_pair(ANIM_IDLE, "idle"));
}