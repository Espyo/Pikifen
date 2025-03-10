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

#include "../../core/drawing.h"
#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/allegro_utils.h"
#include "../../util/general_utils.h"
#include "../../util/geometry_utils.h"
#include "leader.h"


namespace SHIP {

//Animate the control point's ring for this long.
const float CONTROL_POINT_ANIM_DUR = 10.0f;

//The amount of rings the ship's control point has.
const unsigned char CONTROL_POINT_RING_AMOUNT = 4;

//How often the tractor beam generates a ring.
const float TRACTOR_BEAM_EMIT_RATE = 0.15f;

//Animate each tractor beam ring for this long.
const float TRACTOR_BEAM_RING_ANIM_DUR = 0.8f;

}


/**
 * @brief Constructs a new ship object.
 *
 * @param pos Starting coordinates.
 * @param type Ship type this mob belongs to.
 * @param angle Starting angle.
 */
Ship::Ship(const Point &pos, ShipType* type, float angle) :
    Mob(pos, type, angle),
    shi_type(type),
    control_point_final_pos(
        rotate_point(type->control_point_offset, angle)
    ),
    receptacle_final_pos(
        rotate_point(type->receptacle_offset, angle)
    ),
    control_point_to_receptacle_dist(
        Distance(control_point_final_pos, receptacle_final_pos).to_float()
    ) {
    
    next_tractor_beam_ring_timer.on_end = [this] () {
        next_tractor_beam_ring_timer.start();
        tractor_beam_rings.push_back(0);
        float hue =
            fmod(
                game.states.gameplay->area_time_passed * 360, 360
            );
            
        tractor_beam_ring_colors.push_back(hue);
    };
    next_tractor_beam_ring_timer.start();
    
    nest = new PikminNest(this, shi_type->nest);
    
    control_point_final_pos += pos;
    receptacle_final_pos += pos;
}


/**
 * @brief Destroys the ship object.
 */
Ship::~Ship() {
    delete nest;
}


/**
 * @brief Draws a ship.
 */
void Ship::draw_mob() {

    //Draw the rings on the control point.
    for(unsigned char b = 0; b < SHIP::CONTROL_POINT_RING_AMOUNT; b++) {
        float ring_idx_ratio = b / (float) SHIP::CONTROL_POINT_RING_AMOUNT;
        
        float ring_hue = 360 * ring_idx_ratio;
        ALLEGRO_COLOR ring_color = al_color_hsl(ring_hue, 1.0f, 0.8f);
        
        float ring_anim_ratio =
            fmod(
                game.states.gameplay->area_time_passed +
                SHIP::CONTROL_POINT_ANIM_DUR * ring_idx_ratio,
                SHIP::CONTROL_POINT_ANIM_DUR
            );
        ring_anim_ratio /= SHIP::CONTROL_POINT_ANIM_DUR;
        
        unsigned char ring_alpha = 120;
        
        if(ring_anim_ratio <= 0.3f) {
            //Fading into existence.
            ring_alpha =
                interpolate_number(
                    ring_anim_ratio,
                    0.0f, 0.3f,
                    0, ring_alpha
                );
        } else if(ring_anim_ratio >= 0.7f) {
            //Shrinking down.
            ring_alpha =
                interpolate_number(
                    ring_anim_ratio,
                    0.7f, 1.0f,
                    ring_alpha, 0
                );
        }
        
        float ring_scale =
            interpolate_number(
                ease(EASE_METHOD_IN, ring_anim_ratio),
                0.0f, 1.0f,
                1.0f, 0.3f
            );
        float ring_diameter =
            shi_type->control_point_radius * 2.0f * ring_scale;
            
        draw_bitmap(
            game.sys_content.bmp_bright_ring,
            control_point_final_pos, Point(ring_diameter),
            0.0f,
            change_alpha(ring_color, ring_alpha)
        );
    }
    
    //Drawing the tractor beam rings.
    //Go in reverse to ensure the most recent rings are drawn underneath.
    for(char r = (char) tractor_beam_rings.size() - 1; r > 0; r--) {
    
        float ring_anim_ratio =
            tractor_beam_rings[r] / SHIP::TRACTOR_BEAM_RING_ANIM_DUR;
            
        unsigned char ring_alpha = 80;
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
                0.4f, 0.6f
            );
            
        ALLEGRO_COLOR ring_color =
            al_color_hsl(tractor_beam_ring_colors[r], 1.0f, ring_brightness);
        ring_color = change_alpha(ring_color, ring_alpha);
        
        float ring_scale =
            interpolate_number(
                ring_anim_ratio,
                0.0f, 1.0f,
                shi_type->control_point_radius * 2.5f, 1.0f
            );
            
        float distance = control_point_to_receptacle_dist * ring_anim_ratio;
        float angle = get_angle(control_point_final_pos, receptacle_final_pos);
        Point ring_pos(
            control_point_final_pos.x + cos(angle) * distance,
            control_point_final_pos.y + sin(angle) * distance
        );
        
        draw_bitmap(
            game.sys_content.bmp_bright_ring,
            ring_pos,
            Point(ring_scale),
            0.0f,
            ring_color
        );
    }
    
    Mob::draw_mob();
}


/**
* @brief Heals a leader, causes particle effects, etc.
*
* @param l Leader to heal.
*/
void Ship::heal_leader(Leader* l) const {
    l->set_health(false, true, 1.0);
    
    ParticleGenerator pg =
        standard_particle_gen_setup(
            game.sys_content_names.part_leader_heal, l
        );
    l->particle_generators.push_back(pg);
}


/**
 * @brief Checks whether the specified leader is currently on the ship's
 * control point or not.
 *
 * @param l Leader to check.
 * @return Whether the leader is on the control point.
 */
bool Ship::is_leader_on_cp(const Leader* l) const {
    return
        Distance(l->pos, control_point_final_pos) <=
        shi_type->control_point_radius;
}


/**
 * @brief Reads the provided script variables, if any, and does stuff with them.
 *
 * @param svr Script var reader to use.
 */
void Ship::read_script_vars(const ScriptVarReader &svr) {
    Mob::read_script_vars(svr);
    
    nest->read_script_vars(svr);
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void Ship::tick_class_specifics(float delta_t) {
    nest->tick(delta_t);
    
    if(mobs_being_beamed > 0) {
        next_tractor_beam_ring_timer.tick(delta_t);
    }
    
    for(size_t r = 0; r < tractor_beam_rings.size(); ) {
        //Erase rings that have reached the end of their animation.
        tractor_beam_rings[r] += delta_t;
        if(tractor_beam_rings[r] > SHIP::TRACTOR_BEAM_RING_ANIM_DUR) {
            tractor_beam_rings.erase(
                tractor_beam_rings.begin() + r
            );
            tractor_beam_ring_colors.erase(
                tractor_beam_ring_colors.begin() + r
            );
        } else {
            r++;
        }
    }
    
}
