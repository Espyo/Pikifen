/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Interactable type class and interactable type-related functions.
 */

#include "interactable_type.h"

#include "../../util/string_utils.h"
#include "../mob/mob.h"


/**
 * @brief Constructs a new interactable type object.
 */
interactable_type::interactable_type() :
    mob_type(MOB_CATEGORY_INTERACTABLES) {
    
    target_type = MOB_TARGET_FLAG_NONE;
    
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void interactable_type::load_cat_properties(data_node* file) {
    reader_setter rs(file);
    
    rs.set("prompt_text", prompt_text);
    rs.set("trigger_range", trigger_range);
}
