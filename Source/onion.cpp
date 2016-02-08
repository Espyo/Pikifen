/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Onion class and Onion-related functions.
 */

#include "drawing.h"
#include "functions.h"
#include "onion.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Creates an onion.
 */
onion::onion(float x, float y, onion_type* type, const float angle, const string &vars) :
    mob(x, y, type, angle, vars),
    oni_type(type),
    full_spew_timer(ONION_FULL_SPEW_DELAY),
    next_spew_timer(ONION_NEXT_SPEW_DELAY),
    spew_queue(0),
    next_spew_angle(0),
    activated(true),
    seethrough(255) {
    
    //Increase its Z by one so that mobs that walk at
    //ground level next to it will appear under it.
    affected_by_gravity = false;
    z++;
    
    full_spew_timer.on_end = [this] () { next_spew_timer.start(); };
    next_spew_timer.on_end = [this] () { if(spew_queue == 0) return; next_spew_timer.start(); spew(); };
    
    set_animation(ANIM_IDLE);
}

/* ----------------------------------------------------------------------------
 * Receive a mob, carried by a Pikmin.
 */
void onion::receive_mob(mob* m, void* info1, void* info2) {
    size_t seeds = (size_t) info1;
    onion* o_ptr = (onion*) m;
    
    if(o_ptr->spew_queue == 0) {
        o_ptr->full_spew_timer.start();
        o_ptr->next_spew_timer.time_left = 0.0f;
    }
    o_ptr->spew_queue += seeds;
    
    random_particle_explosion(
        PARTICLE_TYPE_BITMAP, bmp_smoke,
        o_ptr->x, o_ptr->y,
        60, 80, 10, 20,
        1, 2, 24, 24, al_map_rgb(255, 255, 255)
    );
    
}


/* ----------------------------------------------------------------------------
 * Spew a Pikmin seed in the queue or add it to the Onion's storage.
 */
void onion::spew() {
    if(spew_queue == 0) return;
    spew_queue--;
    
    unsigned total_after = pikmin_list.size() + 1;
    
    if(total_after > max_pikmin_in_field) {
        pikmin_in_onions[oni_type->pik_type]++;
        return;
    }
    
    pikmin* new_pikmin = new pikmin(x, y, oni_type->pik_type, 0, "");
    //TODO the shooting strength shouldn't be a magic number.
    new_pikmin->z = 320;
    new_pikmin->speed_x = cos(next_spew_angle) * 60;
    new_pikmin->speed_y = sin(next_spew_angle) * 60;
    new_pikmin->speed_z = 200;
    new_pikmin->fsm.set_state(PIKMIN_STATE_BURIED);
    new_pikmin->first_state_set = true;
    create_mob(new_pikmin);
    
    next_spew_angle += ONION_SPEW_ANGLE_SHIFT;
    
}


void onion::draw() {
    frame* f_ptr = anim.get_frame();
    
    if(!f_ptr) return;
    
    float draw_x, draw_y;
    float draw_w, draw_h;
    get_sprite_center(this, f_ptr, &draw_x, &draw_y);
    get_sprite_dimensions(this, f_ptr, &draw_w, &draw_h);
    
    draw_sprite(
        f_ptr->bitmap,
        draw_x, draw_y,
        draw_w, draw_h,
        angle,
        al_map_rgba(
            get_sprite_lighting(this),
            get_sprite_lighting(this),
            get_sprite_lighting(this),
            seethrough
        )
    );
}
