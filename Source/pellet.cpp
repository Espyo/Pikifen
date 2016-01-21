/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pellet class and pellet-related functions.
 */

#include "drawing.h"
#include "functions.h"
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

void pellet::draw() {

    mob::draw();
    
    frame* f_ptr = anim.get_frame();
    if(!f_ptr) return;
    
    float dummy_w, dummy_h, scale;
    get_sprite_dimensions(this, f_ptr, &dummy_w, &dummy_h, &scale);
    
    float radius = type->radius * scale;
    
    draw_sprite(
        pel_type->bmp_number,
        x, y,
        radius * 1.36, -1,
        0, map_gray(get_sprite_lighting(this))
    );
    
}