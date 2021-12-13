/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Treasure class and treasure-related functions.
 */

#include "treasure.h"

#include "../drawing.h"
#include "../functions.h"
#include "../game.h"
#include "../utils/string_utils.h"
#include "ship.h"


/* ----------------------------------------------------------------------------
 * Creates a treasure.
 * pos:
 *   Starting coordinates.
 * type:
 *   Treasure type this mob belongs to.
 * angle:
 *   Starting angle.
 */
treasure::treasure(const point &pos, treasure_type* type, const float angle) :
    mob(pos, type, angle),
    tre_type(type) {
    
    become_carriable(CARRY_DESTINATION_SHIP);
    
    set_animation(ANIM_IDLING, true, START_ANIMATION_RANDOM_FRAME_ON_SPAWN);
    
}


/* ----------------------------------------------------------------------------
 * Draws a treasure.
 */
void treasure::draw_mob() {
    sprite* s_ptr = get_cur_sprite();
    if(!s_ptr) return;
    
    bitmap_effect_info eff;
    ALLEGRO_COLOR delivery_color = map_gray(0);
    float delivery_time_ratio_left = LARGE_FLOAT;
    
    if(fsm.cur_state->id == ENEMY_EXTRA_STATE_BEING_DELIVERED) {
        delivery_color = game.config.carrying_color_move;
        delivery_time_ratio_left = script_timer.get_ratio_left();
    }
    
    get_sprite_bitmap_effects(
        s_ptr, &eff, true, true,
        delivery_time_ratio_left, delivery_color
    );
    
    draw_bitmap_with_effects(s_ptr->bitmap, eff);
}
