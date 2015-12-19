/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Treasure class and treasure-related functions.
 */

#include "functions.h"
#include "treasure.h"

/* ----------------------------------------------------------------------------
 * Creates a treasure.
 */
treasure::treasure(const float x, const float y, treasure_type* type, const float angle, const string &vars) :
    mob(x, y, type, angle, vars),
    buried(s2f(get_var_value(vars, "buried", "0"))) {
    
    carrier_info = new carrier_info_struct(this, type->max_carriers, true);
    
    set_animation(ANIM_IDLE);
    
}
