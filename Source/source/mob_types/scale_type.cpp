/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Scale type class and scale type-related functions.
 */

#include "scale_type.h"

#include "../mobs/scale.h"
#include "../utils/string_utils.h"

/* ----------------------------------------------------------------------------
 * Creates a type of scale.
 */
scale_type::scale_type() :
    mob_type(MOB_CATEGORY_SCALES),
    goal_number(0) {
    
    target_type = MOB_TARGET_TYPE_NONE;
    walkable = true;
}


scale_type::~scale_type() { }


/* ----------------------------------------------------------------------------
 * Loads properties from a data file.
 */
void scale_type::load_properties(data_node* file) {
    reader_setter rs(file);
    
    rs.set("goal_number", goal_number);
}
