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


const unsigned char ship::SHIP_CONTROL_POINT_RING_AMOUNT = 4;
const float ship::SHIP_CONTROL_POINT_ANIM_DUR = 10.0f;

const float ship::SHIP_TRACTOR_BEAM_EMIT_RATE = 0.15f;
const float ship::SHIP_TRACTOR_RING_ANIM_DUR = 0.8f;

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
    tractor_beam_enabled(false),
    next_tractor_ring_timer(SHIP_TRACTOR_BEAM_EMIT_RATE),
    shi_type(type),
    nest(nullptr),
    control_point_final_pos(rotate_point(type->control_point_offset, angle)),
    receptacle_final_pos(rotate_point(type->receptacle_offset, angle)),
    control_point_to_receptacle_dist(dist(control_point_final_pos, receptacle_final_pos).to_float()) {
    
    next_tractor_ring_timer.on_end = [this] () {
        next_tractor_ring_timer.start();
        tractor_beam_rings.push_back(0);
;
        float hue =
            fmod(
                game.states.gameplay->area_time_passed * 360,360
            );

        tractor_beam_ring_colors.push_back(hue);
    };
    next_tractor_ring_timer.start();

    nest = new pikmin_nest_struct(this, shi_type->nest);
    
    control_point_final_pos += pos;
    receptacle_final_pos += pos;
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
    
    //Drawing the rings on the control point
    for(unsigned char b = 0; b < SHIP_CONTROL_POINT_RING_AMOUNT; ++b) {
        float beam_idx_ratio = b / (float) SHIP_CONTROL_POINT_RING_AMOUNT;
        
        float beam_hue = 360 * beam_idx_ratio;
        ALLEGRO_COLOR beam_color = al_color_hsl(beam_hue, 1.0f, 0.8f);
        
        float beam_anim_ratio =
            fmod(
                game.states.gameplay->area_time_passed +
                SHIP_CONTROL_POINT_ANIM_DUR * beam_idx_ratio,
                SHIP_CONTROL_POINT_ANIM_DUR
            );
        beam_anim_ratio /= SHIP_CONTROL_POINT_ANIM_DUR;
        
        unsigned char beam_alpha = 120;
        
        if(beam_anim_ratio <= 0.3f) {
            //Fading into existence.
            beam_alpha =
                interpolate_number(
                    beam_anim_ratio,
                    0.0f, 0.3f,
                    0, beam_alpha
                );
        } else if(beam_anim_ratio >= 0.7f) {
            //Shrinking down.
            beam_alpha =
                interpolate_number(
                    beam_anim_ratio,
                    0.7f, 1.0f,
                    beam_alpha, 0
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
            control_point_final_pos, point(beam_diameter, beam_diameter),
            0.0f,
            change_alpha(beam_color, beam_alpha)
        );
    }

    //Drawing the tractor beam rings
    //Go in reverse to ensure the most recent rings are drawn underneath
    for(char r = tractor_beam_rings.size() - 1; r > 0; r--) {

        float ring_anim_ratio = 
            tractor_beam_rings[r] / SHIP_TRACTOR_RING_ANIM_DUR;

        unsigned char ring_alpha = 120;
        if(ring_anim_ratio <= 0.3f) {
            //Fading into existence.
            ring_alpha =
                interpolate_number(
                    ring_anim_ratio,
                    0.0f, 0.3f,
                    0, ring_alpha
                );
        } else if(ring_anim_ratio >= 0.5f) {
            //Shrinking down.
            ring_alpha =
                interpolate_number(
                    ring_anim_ratio,
                    0.5f, 1.0f,
                    ring_alpha, 0
                );
        }

        float ring_brightness = 
            interpolate_number(
                    ring_anim_ratio,
                    0.0f, 1.0f,
                    0.6f, 0.9f
                );

        ALLEGRO_COLOR ring_color = 
            change_alpha(
                al_color_hsl(tractor_beam_ring_colors[r], 1.0f, ring_brightness), 
                ring_alpha
            );

        float ring_scale =
            interpolate_number(
                ring_anim_ratio,
                0.0f, 1.0f,
                shi_type->beam_radius * 2.5f, 1.0f
            );

        float distance = control_point_to_receptacle_dist * ring_anim_ratio;

        float angle = get_angle(control_point_final_pos, receptacle_final_pos);
        point ring_pos(
            control_point_final_pos.x + cos(angle) * control_point_to_receptacle_dist * ring_anim_ratio,
            control_point_final_pos.y + sin(angle) * control_point_to_receptacle_dist * ring_anim_ratio
        );

        unsigned char n = 0;
        draw_bitmap(
            game.sys_assets.bmp_bright_ring,
            ring_pos,
            point(ring_scale, ring_scale),
            0.0f,
            ring_color
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
    return dist(l->pos, control_point_final_pos) <= shi_type->beam_radius;
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

    if(tractor_beam_enabled) {
        next_tractor_ring_timer.tick(delta_t);
    }
    
    for(size_t r = 0; r < tractor_beam_rings.size(); ) {
        //Erase rings that have reached the end of their animation.
        tractor_beam_rings[r] += delta_t;
        if(tractor_beam_rings[r] > SHIP_TRACTOR_RING_ANIM_DUR) {
            tractor_beam_rings.erase(tractor_beam_rings.begin() + r);
            tractor_beam_ring_colors.erase(tractor_beam_ring_colors.begin() + r);
        } else {
            r++;
        }
    }

}
