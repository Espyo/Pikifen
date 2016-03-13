/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Treasure type class and treasure type-related functions.
 */

#include "../functions.h"
#include "mob_fsm.h"
#include "treasure.h"
#include "treasure_fsm.h"
#include "treasure_type.h"

//TODO bottomless pits.

treasure_type::treasure_type() :
    mob_type(),
    value(0) {
    
    treasure_fsm::create_fsm(this);
}


void treasure_type::load_from_file(data_node* file, const bool load_resources, vector<pair<size_t, string> >* anim_conversions) {

    value = s2f(file->get_child_by_name("value")->value);
    
    anim_conversions->push_back(make_pair(ANIM_IDLE, "idle"));
}
