/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pellet type class and pellet type-related functions.
 */

#include "../functions.h"
#include "mob_fsm.h"
#include "pellet_fsm.h"
#include "pellet_type.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a type of pellet.
 */
pellet_type::pellet_type() :
    mob_type(MOB_CATEGORY_PELLETS),
    pik_type(nullptr),
    number(0),
    match_seeds(0),
    non_match_seeds(0),
    bmp_number(nullptr) {
    
    pellet_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Loads data about the pellet type from a data file.
 */
void pellet_type::load_from_file(
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
    number = s2i(file->get_child_by_name("number")->value);
    weight = number;
    match_seeds = s2i(file->get_child_by_name("match_seeds")->value);
    non_match_seeds = s2i(file->get_child_by_name("non_match_seeds")->value);
    
    if(load_resources) {
        bmp_number =
            bitmaps.get(file->get_child_by_name("number_image")->value, file);
    }
    
    anim_conversions->push_back(make_pair(ANIM_IDLING, "idling"));
}
