/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Ship class and ship-related functions.
 */

#include "../drawing.h"
#include "ship.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a ship mob.
 */
ship::ship(float x, float y, ship_type* type, float angle, const string &vars) :
    mob(x, y, type, angle, vars),
    shi_type(type),
    beam_final_x(0),
    beam_final_y(0) {
    
    rotate_point(
        type->beam_offset_x, type->beam_offset_y,
        angle, &beam_final_x, &beam_final_y
    );
    beam_final_x += x;
    beam_final_y += y;
}


/* ----------------------------------------------------------------------------
 * Draws a ship.
 */
void ship::draw() {
    mob::draw();
    
    unsigned char brightness = get_sprite_brightness(this);
    al_draw_circle(
        beam_final_x,
        beam_final_y,
        shi_type->beam_radius,
        al_map_rgb(
            ship_beam_ring_color[0] * 255 / brightness,
            ship_beam_ring_color[1] * 255 / brightness,
            ship_beam_ring_color[2] * 255 / brightness
        ),
        2
    );
}
