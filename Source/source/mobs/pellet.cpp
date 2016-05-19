/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pellet class and pellet-related functions.
 */

#include "../drawing.h"
#include "../functions.h"
#include "pellet.h"

/* ----------------------------------------------------------------------------
 * Creates a pellet mob.
 */
pellet::pellet(
    float x, float y, pellet_type* type, const float angle, const string &vars
) :
    mob(x, y, type, angle, vars),
    pel_type(type) {

    become_carriable(false);

    set_animation(ANIM_IDLE);
}


/* ----------------------------------------------------------------------------
 * Draws a pellet, with the number and all.
 */
void pellet::draw() {

    frame* f_ptr = anim.get_frame();
    if(!f_ptr) return;

    float draw_x, draw_y;
    float draw_w, draw_h, scale;
    get_sprite_center(this, f_ptr, &draw_x, &draw_y);
    get_sprite_dimensions(this, f_ptr, &draw_w, &draw_h, &scale);

    float radius = type->radius * scale;
    bool being_delivered = false;
    ALLEGRO_COLOR extra_color;

    if(fsm.cur_state->id == PELLET_STATE_BEING_DELIVERED) {
        //If it's being delivered, do some changes to the scale and coloring.
        being_delivered = true;

        if(script_timer.get_ratio_left() >= 0.5) {
            //First half of the sucking in process = interpolated coloring.
            extra_color =
                interpolate_color(
                    script_timer.get_ratio_left(),
                    0.5, 1.0,
                    ((onion*) carrying_target)->oni_type->pik_type->main_color,
                    al_map_rgb(0, 0, 0)
                );
        } else {
            //Second half of the sucking in process = interpolated scaling.
            extra_color =
                ((onion*) carrying_target)->oni_type->pik_type->main_color;
            radius *=
                (script_timer.get_ratio_left() * 2.0);
        }
    }

    draw_sprite(
        f_ptr->bitmap,
        draw_x, draw_y,
        radius * 2.0, -1,
        angle,
        map_gray(get_sprite_brightness(this))
    );

    draw_sprite(
        pel_type->bmp_number,
        draw_x, draw_y,
        radius * 1.36, -1,
        0, map_gray(get_sprite_brightness(this))
    );

    if(being_delivered) {
        int old_op, old_src, old_dst, old_aop, old_asrc, old_adst;
        al_get_separate_blender(
            &old_op, &old_src, &old_dst, &old_aop, &old_asrc, &old_adst
        );
        al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ONE);

        draw_sprite(
            f_ptr->bitmap,
            draw_x, draw_y,
            radius * 2.0, -1,
            angle,
            extra_color
        );

        al_set_separate_blender(
            old_op, old_src, old_dst, old_aop, old_asrc, old_adst
        );
    }

}
