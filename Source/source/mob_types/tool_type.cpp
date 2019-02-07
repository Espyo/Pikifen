/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Tool type class and tool type-related functions.
 */

#include "tool_type.h"

#include "../utils/string_utils.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates a new type of tool mob.
 */
tool_type::tool_type() :
    mob_type(MOB_CATEGORY_TOOLS),
    bmp_icon(NULL),
    can_be_hotswapped(true),
    dropped_when_pikmin_lands(true),
    dropped_when_pikmin_lands_on_opponent(false),
    stuck_when_pikmin_lands_on_opponent(false) {
    
}

tool_type::~tool_type() { }


/* ----------------------------------------------------------------------------
 * Loads parameters from a data file.
 */
void tool_type::load_parameters(data_node* file) {
    dropped_when_pikmin_lands =
        s2b(
            file->get_child_by_name("dropped_when_pikmin_lands")->value
        );
    dropped_when_pikmin_lands_on_opponent =
        s2b(
            file->get_child_by_name(
                "dropped_when_pikmin_lands_on_opponent"
            )->value
        );
    stuck_when_pikmin_lands_on_opponent =
        s2b(
            file->get_child_by_name(
                "stuck_when_pikmin_lands_on_opponent"
            )->value
        );
}


/* ----------------------------------------------------------------------------
 * Loads resources into memory.
 */
void tool_type::load_resources(data_node* file) {
    bmp_icon = bitmaps.get(file->get_child_by_name("icon")->value, file);
}
