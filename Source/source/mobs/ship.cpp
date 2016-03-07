/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
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
 * Creates a ship.
 */
ship::ship(float x, float y, ship_type* type, float angle, const string &vars) :
    mob(x, y, type, angle, vars),
    shi_type(type) {
    
}


void ship::draw() {
    draw_sprite(
        bmp_ship,
        x, y,
        138, 112,
        0, map_gray(get_sprite_lighting(this))
    );
    al_draw_circle(
        x + type->radius + SHIP_BEAM_RANGE,
        y, SHIP_BEAM_RANGE,
        al_map_rgb(
            ship_beam_ring_color[0] * 255 / get_sprite_lighting(this),
            ship_beam_ring_color[1] * 255 / get_sprite_lighting(this),
            ship_beam_ring_color[2] * 255 / get_sprite_lighting(this)
        ), 1
    );
}


void ship::fsm_receive_mob(mob* m, void* info1, void* info2) {
    float pokos = *((float*) info1);
    ship* s_ptr = (ship*) m;
    
    random_particle_explosion(
        PARTICLE_TYPE_BITMAP, bmp_smoke,
        s_ptr->x + s_ptr->type->radius,
        s_ptr->y,
        60, 80, 10, 20,
        1, 2, 24, 24, al_map_rgb(255, 255, 255)
    );
    
}
