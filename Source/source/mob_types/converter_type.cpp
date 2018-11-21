/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Converter type class and converter type-related functions.
 */

#include "converter_type.h"
#include "../mobs/converter_fsm.h"
#include "../functions.h"


/* ----------------------------------------------------------------------------
 * Creates a type of converter.
 */
converter_type::converter_type() :
    mob_type(MOB_CATEGORY_CONVERTERS) {
    
    converter_fsm::create_fsm(this);
}


converter_type::~converter_type() { }


/* ----------------------------------------------------------------------------
 * Loads parameters from a data file.
 */
void converter_type::load_parameters(data_node* file) {

}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector converter_type::get_anim_conversions() {
    anim_conversion_vector v;
    return v;
}
