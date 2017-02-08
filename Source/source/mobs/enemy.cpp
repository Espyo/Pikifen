/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Enemy class and enemy-related functions.
 */

#include "../drawing.h"
#include "enemy.h"
#include "../functions.h"

/* ----------------------------------------------------------------------------
 * Creates an enemy mob.
 */
enemy::enemy(
    const float x, const float y, enemy_type* type, const float angle,
    const string &vars
) :
    mob(x, y, type, angle, vars),
    ene_type(type),
    spawn_delay(s2f(get_var_value(vars, "spawn_delay", "0"))),
    respawn_days_left(0),
    respawns_after_x_days(0),
    appears_after_day(0),
    appears_before_day(0),
    appears_every_x_days(0) {
    
    team = MOB_TEAM_ENEMY_1; //TODO removeish.
    
    //TODO day apperance interval
}


/* ----------------------------------------------------------------------------
 * Draws an enemy, tinting it if necessary (for Onion delivery).
 */
void enemy::draw(sprite_effect_manager* effect_manager) {
    sprite* s_ptr = anim.get_cur_sprite();
    if(!s_ptr) return;
    
    float draw_x, draw_y;
    float draw_w, draw_h;
    get_sprite_center(this, s_ptr, &draw_x, &draw_y);
    get_sprite_dimensions(this, s_ptr, &draw_w, &draw_h);
    
    sprite_effect_manager effects;
    
    if(fsm.cur_state->id == ENEMY_EXTRA_STATE_BEING_DELIVERED) {
        add_delivery_sprite_effect(
            &effects, script_timer.get_ratio_left(),
            ((onion*) carrying_target)->oni_type->pik_type->main_color
        );
    }
    
    add_status_sprite_effects(&effects);
    add_brightness_sprite_effect(&effects);
    
    draw_sprite_with_effects(
        s_ptr->bitmap,
        draw_x, draw_y,
        draw_w, draw_h,
        angle, &effects
    );
    
    draw_status_effect_bmp(this, &effects);
    
}


/* ----------------------------------------------------------------------------
 * Returns whether or not an enemy can receive a given status effect.
 */
bool enemy::can_receive_status(status_type* s) {
    return s->affects & STATUS_AFFECTS_ENEMIES;
}
