/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
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
    const point &pos, treasure_type* type,
    const float angle, const string &vars
) :
    mob(pos, type, angle, vars),
    tre_type(type),
    buried(s2f(get_var_value(vars, "buried", "0"))) {
    
    become_carriable(true);
    
    set_animation(ANIM_IDLING);
    
}


/* ----------------------------------------------------------------------------
 * Draws a treasure.
 */
void treasure::draw(sprite_effect_manager* effect_manager) {
    sprite* s_ptr = anim.get_cur_sprite();
    if(!s_ptr) return;
    
    point draw_pos = get_sprite_center(s_ptr);
    
    sprite_effect_manager effects;
    add_sector_brightness_sprite_effect(&effects);
    
    if(fsm.cur_state->id == TREASURE_STATE_BEING_DELIVERED) {
        add_delivery_sprite_effect(
            &effects, script_timer.get_ratio_left(),
            carrying_color_move
        );
    }
    
    draw_sprite_with_effects(
        s_ptr->bitmap,
        draw_pos,
        point(type->radius * 2.0, -1),
        angle, &effects
    );
}
