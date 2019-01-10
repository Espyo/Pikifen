/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Treasure class and treasure-related functions.
 */

#include "../drawing.h"
#include "../functions.h"
#include "ship.h"
#include "treasure.h"
#include "../utils/string_utils.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates a treasure.
 */
treasure::treasure(const point &pos, treasure_type* type, const float angle) :
    mob(pos, type, angle),
    tre_type(type),
    buried(false) {
    
    become_carriable(CARRY_DESTINATION_SHIP);
    
    set_animation(ANIM_IDLING);
    
}


/* ----------------------------------------------------------------------------
 * Draws a treasure.
 */
void treasure::draw_mob(bitmap_effect_manager* effect_manager) {
    sprite* s_ptr = anim.get_cur_sprite();
    if(!s_ptr) return;
    
    point draw_pos = get_sprite_center(s_ptr);
    
    bitmap_effect_manager effects;
    add_sector_brightness_bitmap_effect(&effects);
    
    if(fsm.cur_state->id == TREASURE_STATE_BEING_DELIVERED) {
        add_delivery_bitmap_effect(
            &effects, script_timer.get_ratio_left(),
            carrying_color_move
        );
    }
    
    draw_bitmap_with_effects(
        s_ptr->bitmap,
        draw_pos,
        point(type->radius * 2.0, -1),
        angle + s_ptr->angle, &effects
    );
}


/* ----------------------------------------------------------------------------
 * Reads the provided script variables, if any, and does stuff with them.
 */
void treasure::read_script_vars(const string &vars) {
    mob::read_script_vars(vars);
    buried = s2f(get_var_value(vars, "buried", "0"));
}
