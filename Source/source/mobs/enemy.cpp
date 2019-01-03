/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Enemy class and enemy-related functions.
 */

#include "../drawing.h"
#include "../functions.h"
#include "enemy.h"
#include "../utils/math_utils.h"
#include "../utils/string_utils.h"

/* ----------------------------------------------------------------------------
 * Creates an enemy mob.
 */
enemy::enemy(const point &pos, enemy_type* type, const float angle) :
    mob(pos, type, angle),
    ene_type(type),
    spawn_delay(0),
    respawn_days_left(0),
    respawns_after_x_days(0),
    appears_after_day(0),
    appears_before_day(0),
    appears_every_x_days(0) {
    
    team = MOB_TEAM_ENEMY_1; //TODO removeish.
}


/* ----------------------------------------------------------------------------
 * Draws an enemy, tinting it if necessary (for Onion delivery).
 */
void enemy::draw_mob(bitmap_effect_manager* effect_manager) {
    sprite* s_ptr = anim.get_cur_sprite();
    if(!s_ptr) return;
    
    point draw_pos = get_sprite_center(s_ptr);
    point draw_size = get_sprite_dimensions(s_ptr);
    
    bitmap_effect_manager effects;
    
    if(fsm.cur_state->id == ENEMY_EXTRA_STATE_BEING_DELIVERED) {
        onion* o_ptr = ((onion*) focused_mob);
        add_delivery_bitmap_effect(
            &effects, script_timer.get_ratio_left(),
            o_ptr->oni_type->pik_type->main_color
        );
    }
    
    add_status_bitmap_effects(&effects);
    add_sector_brightness_bitmap_effect(&effects);
    
    draw_bitmap_with_effects(
        s_ptr->bitmap,
        draw_pos, draw_size,
        angle + s_ptr->angle, &effects
    );
    
    draw_status_effect_bmp(this, &effects);
    
}


/* ----------------------------------------------------------------------------
 * Returns whether or not an enemy can receive a given status effect.
 */
bool enemy::can_receive_status(status_type* s) {
    return s->affects & STATUS_AFFECTS_ENEMIES;
}


/* ----------------------------------------------------------------------------
 * Reads the provided script variables, if any, and does stuff with them.
 */
void enemy::read_script_vars(const string &vars) {
    mob::read_script_vars(vars);
    spawn_delay = s2f(get_var_value(vars, "spawn_delay", "0"));
}
