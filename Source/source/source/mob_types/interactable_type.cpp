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

#include "../mobs/mob.h"
#include "../utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Creates a new type of interactable mob.
 */
interactable_type::interactable_type() :
    mob_type(MOB_CATEGORY_INTERACTABLES),
    trigger_range(64.0f) {
    
    target_type = MOB_TARGET_TYPE_NONE;
    
}


/* ----------------------------------------------------------------------------
 * Loads properties from a data file.
 * file:
 *   File to read from.
 */
void interactable_type::load_properties(data_node* file) {
    reader_setter rs(file);
    
    rs.set("prompt_text", prompt_text);
    rs.set("trigger_range", trigger_range);
}
