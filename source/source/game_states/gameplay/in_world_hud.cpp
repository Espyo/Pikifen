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
#include "../../functions.h"
#include "../../game.h"
#include "../../mobs/mob.h"
#include "../../utils/allegro_utils.h"
#include "../../utils/string_utils.h"


namespace IN_WORLD_FRACTION {

//How much to grow when performing a juicy grow animation.
const float GROW_JUICE_AMOUNT = 0.06f;

//How long it takes to animate the numbers growing.
const float GROW_JUICE_DURATION = 0.3f;

//Padding between mob and fraction.
const float PADDING = 8.0f;

//How much to grow when performing a requirement met juicy grow animation.
const float REQ_MET_GROW_JUICE_AMOUNT = 0.12f;

//How long it takes to animate the numbers flashing.
const float REQ_MET_JUICE_DURATION = 0.5f;

//Height of one of the fraction's rows.
const float ROW_HEIGHT = 18.0f;

//How long it takes to fade in.
const float TRANSITION_IN_DURATION = 0.4f;

//How long it takes to fade out.
const float TRANSITION_OUT_DURATION = 0.5f;

}


namespace IN_WORLD_HEALTH_WHEEL {

//Standard opacity.
const float OPACITY = 0.85f;

//Padding between mob and wheel.
const float PADDING = 4.0f;

//Multiply health wheel speed by this.
const float SMOOTHNESS_MULT = 6.0f;

//How long it takes to fade in.
const float TRANSITION_IN_DURATION = 0.2f;

//How long it takes to fade out.
const float TRANSITION_OUT_DURATION = 1.5f;

}


/**
 * @brief Constructs a new in-world fraction object.
 *
 * @param m Mob it belongs to.
 */
in_world_fraction::in_world_fraction(mob* m) :
    in_world_hud_item(m) {
    
    transition_timer = IN_WORLD_FRACTION::TRANSITION_IN_DURATION;
}


/**
 * @brief Draws an in-world fraction.
 */
void in_world_fraction::draw() {
    float alpha_mult = 1.0f;
    float size_mult = 1.0f;
    
    switch(transition) {
    case IN_WORLD_HUD_TRANSITION_IN: {
        float timer_ratio =
            1 - (transition_timer / IN_WORLD_FRACTION::TRANSITION_IN_DURATION);
        alpha_mult = timer_ratio;
        size_mult = ease(EASE_METHOD_OUT, timer_ratio) * 0.5 + 0.5;
        break;
    }
    case IN_WORLD_HUD_TRANSITION_OUT: {
        alpha_mult =
            transition_timer / IN_WORLD_FRACTION::TRANSITION_OUT_DURATION;
        break;
    }
    default: {
        break;
    }
    }
    
    if(grow_juice_timer > 0.0f) {
        float anim_ratio =
            1 - (grow_juice_timer / IN_WORLD_FRACTION::GROW_JUICE_DURATION);
        anim_ratio = ease(EASE_METHOD_UP_AND_DOWN, anim_ratio);
        size_mult += IN_WORLD_FRACTION::GROW_JUICE_AMOUNT * anim_ratio;
    }
    
    ALLEGRO_COLOR final_color;
    if(req_met_juice_timer > 0.0f) {
        final_color =
            interpolate_color(
                req_met_juice_timer, 0.0f,
                IN_WORLD_FRACTION::REQ_MET_JUICE_DURATION,
                color, COLOR_WHITE
            );
            
        float anim_ratio =
            1.0f -
            (req_met_juice_timer / IN_WORLD_FRACTION::REQ_MET_JUICE_DURATION);
        anim_ratio = ease(EASE_METHOD_UP_AND_DOWN, anim_ratio);
        size_mult += IN_WORLD_FRACTION::REQ_MET_GROW_JUICE_AMOUNT * anim_ratio;
    } else {
        final_color = color;
    }
    final_color.a *= alpha_mult;
    
    if(requirement_number > 0) {
        point pos(m->pos.x, m->pos.y - m->radius - IN_WORLD_FRACTION::PADDING);
        draw_fraction(
            pos,
            value_number, requirement_number, final_color, size_mult
        );
    } else {
        point pos(
            m->pos.x,
            m->pos.y - m->radius -
            al_get_font_line_height(game.sys_assets.fnt_standard) -
            IN_WORLD_FRACTION::PADDING
        );
        draw_text(
            i2s(value_number), game.sys_assets.fnt_standard, pos,
            point(LARGE_FLOAT, IN_WORLD_FRACTION::ROW_HEIGHT * size_mult),
            final_color
        );
    }
}


/**
 * @brief Sets the color.
 *
 * @param new_color Color to set to.
 */
void in_world_fraction::set_color(const ALLEGRO_COLOR &new_color) {
    if(color == new_color) return;
    
    color = new_color;
    grow_juice_timer = IN_WORLD_FRACTION::GROW_JUICE_DURATION;
}


/**
 * @brief Sets the requirement number.
 *
 * @param new_req_nr Requirement number to set to.
 */
void in_world_fraction::set_requirement_number(float new_req_nr) {
    if(requirement_number == new_req_nr) return;
    
    bool req_was_met = value_number >= requirement_number;
    requirement_number = new_req_nr;
    
    if(
        requirement_number > 0.0f &&
        !req_was_met &&
        value_number >= requirement_number
    ) {
        req_met_juice_timer = IN_WORLD_FRACTION::REQ_MET_JUICE_DURATION;
    } else {
        grow_juice_timer = IN_WORLD_FRACTION::GROW_JUICE_DURATION;
    }
}


/**
 * @brief Sets the value number.
 *
 * @param new_value_nr Value number to set to.
 */
void in_world_fraction::set_value_number(float new_value_nr) {
    if(value_number == new_value_nr) return;
    
    bool req_was_met = value_number >= requirement_number;
    
    value_number = new_value_nr;
    
    if(
        requirement_number > 0.0f &&
        !req_was_met &&
        value_number >= requirement_number
    ) {
        req_met_juice_timer = IN_WORLD_FRACTION::REQ_MET_JUICE_DURATION;
    } else {
        grow_juice_timer = IN_WORLD_FRACTION::GROW_JUICE_DURATION;
    }
}


/**
 * @brief Starts fading away.
 */
void in_world_fraction::start_fading() {
    if(transition == IN_WORLD_HUD_TRANSITION_OUT) {
        return;
    }
    transition = IN_WORLD_HUD_TRANSITION_OUT;
    transition_timer = IN_WORLD_FRACTION::TRANSITION_OUT_DURATION;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void in_world_fraction::tick(float delta_t) {
    in_world_hud_item::tick(delta_t);
    if(grow_juice_timer > 0.0f) {
        grow_juice_timer -= delta_t;
    }
    if(req_met_juice_timer > 0.0f) {
        req_met_juice_timer -= delta_t;
    }
}


/**
 * @brief Constructs a new in-world health wheel object.
 *
 * @param m Mob it belongs to.
 */
in_world_health_wheel::in_world_health_wheel(mob* m) :
    in_world_hud_item(m) {
    
    if(m->max_health > 0.0f) {
        visible_ratio = m->health / m->max_health;
    }
    transition_timer = IN_WORLD_HEALTH_WHEEL::TRANSITION_IN_DURATION;
}


/**
 * @brief Draws an in-world health wheel.
 */
void in_world_health_wheel::draw() {
    float alpha_mult = 1.0f;
    float size_mult = 1.0f;
    switch(transition) {
    case IN_WORLD_HUD_TRANSITION_IN: {
        float timer_ratio =
            1.0f -
            (transition_timer / IN_WORLD_HEALTH_WHEEL::TRANSITION_IN_DURATION);
        alpha_mult = timer_ratio;
        size_mult = ease(EASE_METHOD_OUT, timer_ratio) * 0.5 + 0.5;
        break;
    }
    case IN_WORLD_HUD_TRANSITION_OUT: {
        alpha_mult =
            transition_timer / IN_WORLD_HEALTH_WHEEL::TRANSITION_OUT_DURATION;
        break;
    }
    default: {
        break;
    }
    }
    
    float radius = DRAWING::DEF_HEALTH_WHEEL_RADIUS * size_mult;
    draw_health(
        point(
            m->pos.x,
            m->pos.y - m->radius -
            radius - IN_WORLD_HEALTH_WHEEL::PADDING
        ),
        visible_ratio,
        IN_WORLD_HEALTH_WHEEL::OPACITY * alpha_mult,
        radius
    );
}


/**
 * @brief Starts fading away.
 */
void in_world_health_wheel::start_fading() {
    if(transition == IN_WORLD_HUD_TRANSITION_OUT) {
        return;
    }
    transition = IN_WORLD_HUD_TRANSITION_OUT;
    transition_timer = IN_WORLD_HEALTH_WHEEL::TRANSITION_OUT_DURATION;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void in_world_health_wheel::tick(float delta_t) {
    in_world_hud_item::tick(delta_t);
    
    if(m->max_health == 0.0f) return;
    
    visible_ratio +=
        ((m->health / m->max_health) - visible_ratio) *
        (IN_WORLD_HEALTH_WHEEL::SMOOTHNESS_MULT * delta_t);
}


/**
 * @brief Constructs a new in-world HUD item object.
 *
 * @param m Mob it belongs to.
 */
in_world_hud_item::in_world_hud_item(mob* m) :
    m(m) {
    
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void in_world_hud_item::tick(float delta_t) {
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
