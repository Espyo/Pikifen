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
#include "../geometry_utils.h"
#include "ship.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a ship mob.
 */
ship::ship(const point &pos, ship_type* type, float angle, const string &vars) :
    mob(pos, type, angle, vars),
    beam_final_pos(rotate_point(type->beam_offset, angle)),
    shi_type(type) {
    
    beam_final_pos += pos;
}


/* ----------------------------------------------------------------------------
 * Draws a ship.
 */
void ship::draw(sprite_effect_manager* effect_manager) {

    mob::draw();
    
    al_draw_circle(
        beam_final_pos.x,
        beam_final_pos.y,
        shi_type->beam_radius,
        al_map_rgb(
            ship_beam_ring_color[0],
            ship_beam_ring_color[1],
            ship_beam_ring_color[2]
        ),
        2
    );
}
