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
    mob_type(MOB_CATEGORY_SCALES) {
    
    target_type = MOB_TARGET_TYPE_NONE;
}


/* ----------------------------------------------------------------------------
 * Loads parameters from a data file.
 */
void scale_type::load_parameters(data_node* file) {
}


scale_type::~scale_type() { }
