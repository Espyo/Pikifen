/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Status effect classes and status effect-related functions.
 */

#include <algorithm>

#include "status.h"


/* ----------------------------------------------------------------------------
 * Creates a status effect instance.
 * type:
 *   Its type.
 */
status::status(status_type* type) :
    type(type),
    from_hazard(false),
    to_delete(false) {
    
    time_left = type->auto_remove_time;
}


/* ----------------------------------------------------------------------------
 * Ticks a status effect instance's time by one frame of logic, but does not
 * tick its effects logic.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 */
void status::tick(const float delta_t) {
    if(type->auto_remove_time > 0.0f) {
        time_left -= delta_t;
        if(time_left <= 0.0f) {
            to_delete = true;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Creates a status effect type.
 */
status_type::status_type() :
    affects(0),
    color(COLOR_EMPTY),
    tint(COLOR_WHITE),
    glow(COLOR_EMPTY),
    removable_with_whistle(false),
    remove_on_hazard_leave(false),
    auto_remove_time(0.0f),
    reapply_rule(STATUS_REAPPLY_KEEP_TIME),
    health_change(0.0f),
    health_change_ratio(0.0f),
    maturity_change_amount(0),
    state_change_type(STATUS_STATE_CHANGE_NONE),
    speed_multiplier(1.0f),
    attack_multiplier(1.0f),
    defense_multiplier(1.0f),
    anim_speed_multiplier(1.0f),
    disables_attack(false),
    turns_inedible(false),
    turns_invisible(false),
    freezes_animation(false),
    generates_particles(false),
    particle_gen(nullptr),
    particle_offset_z(0.0f),
    shaking_effect(0.0f),
    overlay_anim_mob_scale(1.0f),
    replacement_on_timeout(nullptr) {
    
}
