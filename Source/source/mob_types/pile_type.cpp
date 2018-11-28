/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pile type class and pile type-related functions.
 */

#include "../functions.h"
#include "../mobs/pile.h"
#include "../mobs/pile_fsm.h"
#include "pile_type.h"
#include "../utils/string_utils.h"

/* ----------------------------------------------------------------------------
 * Creates a type of pile.
 */
pile_type::pile_type() :
    mob_type(MOB_CATEGORY_PILES) {
    
    pile_fsm::create_fsm(this);
}


/* ----------------------------------------------------------------------------
 * Loads parameters from a data file.
 */
void pile_type::load_parameters(data_node* file) {
    reader_setter rs(file);
    //TODO
}


/* ----------------------------------------------------------------------------
 * Returns the vector of animation conversions.
 */
anim_conversion_vector pile_type::get_anim_conversions() {
    anim_conversion_vector v;
    v.push_back(make_pair(PILE_ANIM_IDLING, "idling"));
    return v;
}


pile_type::~pile_type() { }
