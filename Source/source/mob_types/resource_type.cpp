/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Resource type class and resource type-related functions.
 */

#include "../functions.h"
#include "../mobs/resource.h"
#include "../mobs/resource_fsm.h"
#include "resource_type.h"
#include "../utils/string_utils.h"

/* ----------------------------------------------------------------------------
 * Creates a type of resource.
 */
resource_type::resource_type() :
    mob_type(MOB_CATEGORY_RESOURCES) {
    
    resource_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Loads parameters from a data file.
 */
void resource_type::load_parameters(data_node* file) {
    reader_setter rs(file);
    //TODO
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector resource_type::get_anim_conversions() {
    anim_conversion_vector v;
    v.push_back(make_pair(RESOURCE_ANIM_IDLING, "idling"));
    return v;
}


resource_type::~resource_type() { }
