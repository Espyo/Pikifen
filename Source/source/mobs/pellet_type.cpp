/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
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
 * Loads parameters from a data file.
 */
void pellet_type::load_parameters(data_node* file) {
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
    
}


/* ----------------------------------------------------------------------------
 * Loads resources into memory.
 */
void pellet_type::load_resources(data_node* file) {
    bmp_number =
        bitmaps.get(file->get_child_by_name("number_image")->value, file);
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector pellet_type::get_anim_conversions() {
    anim_conversion_vector v;
    v.push_back(make_pair(ANIM_IDLING, "idling"));
    return v;
}


/* ----------------------------------------------------------------------------
 * Unloads resources from memory.
 */
void pellet_type::unload_resources() {
    bitmaps.detach(bmp_number);
}
