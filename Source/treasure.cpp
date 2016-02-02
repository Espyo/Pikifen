/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
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
    
    become_carriable();
    
    set_animation(ANIM_IDLE);
    
}
