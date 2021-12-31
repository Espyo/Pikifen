/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Ship class and ship-related functions.
 */

#include <allegro5/allegro_color.h>

#include "ship.h"

#include "../functions.h"
#include "../drawing.h"
#include "../game.h"
#include "../utils/geometry_utils.h"
#include "leader.h"


const float ship::SHIP_BEAM_RING_HUE_SPEED = 200;
const float ship::SHIP_BEAM_RING_ANIM_DUR = 2.0f;


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
    
    float beam_hue =
        fmod(
            game.states.gameplay->area_time_passed *
            SHIP_BEAM_RING_HUE_SPEED,
            360.0f
        );
    ALLEGRO_COLOR beam_color = al_color_hsl(beam_hue, 0.90f, 0.90f);
    
    for(unsigned char b = 0; b < 2; ++b) {
        float beam_anim_ratio =
            fmod(
                game.states.gameplay->area_time_passed +
                SHIP_BEAM_RING_ANIM_DUR * 0.4f * b,
                SHIP_BEAM_RING_ANIM_DUR
            );
        beam_anim_ratio /= SHIP_BEAM_RING_ANIM_DUR;
        unsigned char beam_alpha = 255;
        
        if(beam_anim_ratio <= 0.4f) {
            //Fading into existence.
            beam_alpha =
                interpolate_number(
                    beam_anim_ratio,
                    0.0f, 0.4f,
                    0, 255
                );
        } else {
            //Shrinking down.
            beam_alpha =
                interpolate_number(
                    beam_anim_ratio,
                    0.4f, 1.0f,
                    255, 0
                );
        }
        
        float beam_scale =
            interpolate_number(
                ease(EASE_IN, beam_anim_ratio),
                0.0f, 1.0f,
                1.0f, 0.3f
            );
        float beam_diameter = shi_type->beam_radius * 2.0f * beam_scale;
        
        draw_bitmap(
            game.sys_assets.bmp_bright_ring,
            beam_final_pos, point(beam_diameter, beam_diameter),
            0.0f,
            change_alpha(beam_color, beam_alpha)
        );
    }
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
    g.pos_deviation = point(l->radius, l->radius);
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
    nest->tick(delta_t);
}
