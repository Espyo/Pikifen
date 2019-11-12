/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Track type class and track type-related functions.
 */

#include "track_type.h"

#include "../utils/string_utils.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates a new type of track mob.
 */
track_type::track_type() :
    mob_type(MOB_CATEGORY_TRACKS) {
    
    target_type = MOB_TARGET_TYPE_NONE;
    
}

track_type::~track_type() { }


/* ----------------------------------------------------------------------------
 * Loads parameters from a data file.
 */
void track_type::load_parameters(data_node* file) {

}
