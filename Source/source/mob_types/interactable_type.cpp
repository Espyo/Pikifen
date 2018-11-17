/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Interactable type class and interactable type-related functions.
 */

#include "interactable_type.h"
#include "../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Creates a new type of interactable mob.
 */
interactable_type::interactable_type() :
    mob_type(MOB_CATEGORY_INTERACTABLES),
    trigger_range(64.0f) {
    
}

interactable_type::~interactable_type() { }


/* ----------------------------------------------------------------------------
 * Loads parameters from a data file.
 */
void interactable_type::load_parameters(data_node* file) {
    prompt_text = file->get_child_by_name("prompt_text")->value;
    trigger_range =
        s2f(
            file->get_child_by_name("trigger_range")->get_value_or_default("64")
        );
}
