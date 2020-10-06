/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Ship class and ship-related functions.
 */

#include "ship.h"

#include "../drawing.h"
#include "../game.h"
#include "../utils/geometry_utils.h"
#include "leader.h"


const unsigned int ship::SHIP_BEAM_RING_COLOR_SPEED = 255;

/* ----------------------------------------------------------------------------
 * Creates a ship mob.
 * pos:
 *   Starting coordinates.
 * type:
 *   Ship type this mob belongs to.
 * angle:
 *   Starting angle.
 */
ship::ship(const point &pos, ship_type* type, float angle) :
    mob(pos, type, angle),
    shi_type(type),
    nest(nullptr),
    beam_final_pos(rotate_point(type->beam_offset, angle)) {
    
    nest = new pikmin_nest_struct(this, shi_type->nest);
    
    beam_final_pos += pos;
    beam_ring_color[0] = 0;
    beam_ring_color[1] = 0;
    beam_ring_color[2] = 0;
    beam_ring_color_up[0] = true;
    beam_ring_color_up[1] = true;
    beam_ring_color_up[2] = true;
}


/* ----------------------------------------------------------------------------
 * Destroys a ship mob.
 */
ship::~ship() {
    delete nest;
}


/* ----------------------------------------------------------------------------
 * Draws a ship.
 */
void ship::draw_mob() {

    mob::draw_mob();
    
    al_draw_circle(
        beam_final_pos.x,
        beam_final_pos.y,
        shi_type->beam_radius,
        al_map_rgb(
            beam_ring_color[0],
            beam_ring_color[1],
            beam_ring_color[2]
        ),
        2
    );
}


/* ----------------------------------------------------------------------------
 * Heals a leader, causes particle effects, etc.
 * l:
 *   Leader to heal.
 */
void ship::heal_leader(leader* l) const {
    l->set_health(false, true, 1.0);
    
    particle p(
        PARTICLE_TYPE_BITMAP,
        l->pos, l->z + l->height, 16, 3,
        PARTICLE_PRIORITY_LOW
    );
    p.bitmap = game.sys_assets.bmp_sparkle;
    p.color = al_map_rgba(192, 255, 192, 255);
    p.speed = point(0, -24);
    
    particle_generator g(0, p, 12);
    g.duration_deviation = 0.5;
    g.pos_deviation = point(l->type->radius, l->type->radius);
    g.emit(game.states.gameplay->particles);
}


/* ----------------------------------------------------------------------------
 * Checks whether the specified leader is currently under the ship's
 * beam or not.
 * l:
 *   Leader to check.
 */
bool ship::is_leader_under_beam(leader* l) const {
    return dist(l->pos, beam_final_pos) <= shi_type->beam_radius;
}


/* ----------------------------------------------------------------------------
 * Reads the provided script variables, if any, and does stuff with them.
 * svr:
 *   Script var reader to use.
 */
void ship::read_script_vars(const script_var_reader &svr) {
    mob::read_script_vars(svr);
    
    nest->read_script_vars(svr);
}


/* ----------------------------------------------------------------------------
 * Ticks class-specific logic.
 * delta_t:
 *   How many seconds to tick by.
 */
void ship::tick_class_specifics(const float delta_t) {
    //The way the beam ring works is that the three color components are saved.
    //Each frame, we increase them or decrease them
    //(if it reaches 255, set it to decrease, if 0, set it to increase).
    //Each component increases/decreases at a different speed,
    //with red being the slowest and blue the fastest.
    for(unsigned char i = 0; i < 3; ++i) {
        float dir_mult = (beam_ring_color_up[i]) ? 1.0 : -1.0;
        signed char addition =
            dir_mult * SHIP_BEAM_RING_COLOR_SPEED * (i + 1) * delta_t;
        if(beam_ring_color[i] + addition >= 255) {
            beam_ring_color[i] = 255;
            beam_ring_color_up[i] = false;
        } else if(beam_ring_color[i] + addition <= 0) {
            beam_ring_color[i] = 0;
            beam_ring_color_up[i] = true;
        } else {
            beam_ring_color[i] += addition;
        }
    }
    
    nest->tick(delta_t);
}
