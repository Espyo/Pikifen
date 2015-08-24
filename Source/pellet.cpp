/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pellet class and pellet-related functions.
 */

#include "pellet.h"

/* ----------------------------------------------------------------------------
 * Creates a pellet.
 */
pellet::pellet(float x, float y, pellet_type* type, const float angle, const string &vars) :
    mob(x, y, type, angle, vars),
    pel_type(type) {
    
    carrier_info = new carrier_info_struct(this, type->max_carriers, false);
    
    set_animation(ANIM_IDLE);
}
