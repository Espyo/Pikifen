/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Ship type class and ship type-related functions.
 */

#include "../functions.h"
#include "ship.h"
#include "ship_fsm.h"
#include "ship_type.h"

/* ----------------------------------------------------------------------------
 * Creates a type of ship.
 */
ship_type::ship_type() :
    mob_type(),
    can_heal(false),
    beam_offset_x(0.0f),
    beam_offset_y(0.0f),
    beam_radius(0.0f) {
    
    ship_fsm::create_fsm(this);
    always_active = true;
}


/* ----------------------------------------------------------------------------
 * Loads data about the ship type from a data file.
 */
void ship_type::load_from_file(
    data_node* file, const bool load_resources,
    vector<pair<size_t, string> >* anim_conversions
) {
    can_heal = file->get_child_by_name("can_heal");
    beam_offset_x = s2f(file->get_child_by_name("beam_offset_x")->value);
    beam_offset_y = s2f(file->get_child_by_name("beam_offset_y")->value);
    beam_radius = s2f(file->get_child_by_name("beam_radius")->value);
    
    anim_conversions->push_back(make_pair(SHIP_ANIM_IDLE, "idle"));
}
