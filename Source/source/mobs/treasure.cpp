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

#include "../drawing.h"
#include "../functions.h"
#include "ship.h"
#include "treasure.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a treasure.
 */
treasure::treasure(
    const float x, const float y, treasure_type* type,
    const float angle, const string &vars
) :
    mob(x, y, type, angle, vars),
    tre_type(type),
    buried(s2f(get_var_value(vars, "buried", "0"))) {
    
    become_carriable(true);
    
    set_animation(ANIM_IDLING);
    
}


/* ----------------------------------------------------------------------------
 * Draws a treasure.
 */
void treasure::draw() {
    sprite* s_ptr = anim.get_cur_sprite();
    if(!s_ptr) return;
    
    float draw_x, draw_y;
    float draw_w, draw_h, scale;
    get_sprite_center(this, s_ptr, &draw_x, &draw_y);
    get_sprite_dimensions(this, s_ptr, &draw_w, &draw_h, &scale);
    
    float radius = type->radius * scale;
    bool being_delivered = false;
    ALLEGRO_COLOR extra_color;
    
    if(fsm.cur_state->id == TREASURE_STATE_BEING_DELIVERED) {
        //If it's being delivered, do some changes to the scale and coloring.
        being_delivered = true;
        
        if(script_timer.get_ratio_left() >= 0.5) {
            //First half of the sucking in process = interpolated coloring.
            extra_color = interpolate_color(
                              script_timer.get_ratio_left(),
                              0.5, 1.0,
                              carrying_color_move,
                              al_map_rgb(0, 0, 0)
                          );
        } else {
            //Second half of the sucking in process = interpolated scaling.
            extra_color = carrying_color_move;
            radius *= (script_timer.get_ratio_left() * 2.0);
        }
    }
    
    draw_sprite(
        s_ptr->bitmap,
        draw_x,
        draw_y,
        radius * 2.0, -1,
        angle,
        map_gray(get_sprite_brightness(this))
    );
    
    if(being_delivered) {
        int old_op, old_src, old_dst, old_aop, old_asrc, old_adst;
        al_get_separate_blender(
            &old_op, &old_src, &old_dst, &old_aop, &old_asrc, &old_adst
        );
        al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE);
        
        draw_sprite(
            s_ptr->bitmap,
            draw_x,
            draw_y,
            radius * 2.0, -1,
            angle,
            extra_color
        );
        
        al_set_separate_blender(
            old_op, old_src, old_dst, old_aop, old_asrc, old_adst
        );
    }
}
