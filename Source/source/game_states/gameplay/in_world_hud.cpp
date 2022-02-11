/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * In-world HUD class and in-world HUD-related functions.
 */

#include "in_world_hud.h"

#include "../../drawing.h"
#include "../../mobs/mob.h"


//Multiply health wheel speed by this.
const float in_world_health_wheel::SMOOTHNESS_MULT = 6.0f;
//How long it takes to fade in.
const float in_world_health_wheel::TRANSITION_IN_TIME = 0.2f;
//How long it takes to fade out.
const float in_world_health_wheel::TRANSITION_OUT_TIME = 1.5f;


/* ----------------------------------------------------------------------------
 * Creates an in-world health wheel.
 */
in_world_health_wheel::in_world_health_wheel(mob* m) :
    in_world_hud_item(m),
    visible_ratio(0.0f) {
    
    if(m->type->max_health > 0.0f) {
        visible_ratio = m->health / m->type->max_health;
    }
    transition_timer = TRANSITION_IN_TIME;
}


/* ----------------------------------------------------------------------------
 * Draws an in-world health wheel.
 */
void in_world_health_wheel::draw() {
    //Standard opacity.
    const float OPACITY = 0.85f;
    
    float alpha_mult = 1.0f;
    float size_mult = 1.0f;
    switch(transition) {
    case IN_WORLD_HUD_TRANSITION_IN: {
        float timer_ratio = 1 - (transition_timer / TRANSITION_IN_TIME);
        alpha_mult = timer_ratio;
        size_mult = ease(EASE_OUT, timer_ratio) * 0.5 + 0.5;
        break;
    }
    case IN_WORLD_HUD_TRANSITION_OUT: {
        alpha_mult = transition_timer / TRANSITION_OUT_TIME;
        break;
    }
    default: {
        break;
    }
    }
    
    float radius = DEF_HEALTH_WHEEL_RADIUS * size_mult;
    draw_health(
        point(
            m->pos.x,
            m->pos.y - m->radius -
            radius - 4
        ),
        visible_ratio,
        OPACITY * alpha_mult,
        radius
    );
}


/* ----------------------------------------------------------------------------
 * Starts fading away.
 */
void in_world_health_wheel::start_fading() {
    if(transition == IN_WORLD_HUD_TRANSITION_OUT) {
        return;
    }
    transition = IN_WORLD_HUD_TRANSITION_OUT;
    transition_timer = TRANSITION_OUT_TIME;
}


/* ----------------------------------------------------------------------------
 * Ticks one frame worth of time.
 * delta_t:
 *   How many seconds to tick by.
 */
void in_world_health_wheel::tick(const float delta_t) {
    in_world_hud_item::tick(delta_t);
    
    if(m->type->max_health == 0.0f) return;
    
    visible_ratio +=
        ((m->health / m->type->max_health) - visible_ratio) *
        (SMOOTHNESS_MULT * delta_t);
}


/* ----------------------------------------------------------------------------
 * Creates an in-world HUD item.
 * m:
 *   Mob it belongs to.
 */
in_world_hud_item::in_world_hud_item(mob* m) :
    m(m),
    transition(IN_WORLD_HUD_TRANSITION_IN),
    transition_timer(0.0f),
    to_delete(false) {
    
}


/* ----------------------------------------------------------------------------
 * Ticks one frame worth of time.
 * delta_t:
 *   How many seconds to tick by.
 */
void in_world_hud_item::tick(const float delta_t) {
    switch(transition) {
    case IN_WORLD_HUD_TRANSITION_IN: {
        transition_timer -= delta_t;
        if(transition_timer <= 0.0f) {
            transition = IN_WORLD_HUD_TRANSITION_NONE;
        }
        break;
    }
    case IN_WORLD_HUD_TRANSITION_OUT: {
        transition_timer -= delta_t;
        if(transition_timer <= 0.0f) {
            to_delete = true;
        }
        break;
    }
    default: {
        break;
    }
    }
}
