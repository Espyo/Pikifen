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

#include "../../content/mob/mob.h"
#include "../../core/drawing.h"
#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/allegro_utils.h"
#include "../../util/string_utils.h"


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
InWorldFraction::InWorldFraction(Mob* m) :
    InWorldHudItem(m) {
    
    transitionTimer = IN_WORLD_FRACTION::TRANSITION_IN_DURATION;
}


/**
 * @brief Draws an in-world fraction.
 */
void InWorldFraction::draw() {
    float alpha_mult = 1.0f;
    float size_mult = 1.0f;
    
    switch(transition) {
    case IN_WORLD_HUD_TRANSITION_IN: {
        float timer_ratio =
            1 - (transitionTimer / IN_WORLD_FRACTION::TRANSITION_IN_DURATION);
        alpha_mult = timer_ratio;
        size_mult = ease(EASE_METHOD_OUT, timer_ratio) * 0.5 + 0.5;
        break;
    }
    case IN_WORLD_HUD_TRANSITION_OUT: {
        alpha_mult =
            transitionTimer / IN_WORLD_FRACTION::TRANSITION_OUT_DURATION;
        break;
    }
    default: {
        break;
    }
    }
    
    if(growJuiceTimer > 0.0f) {
        float anim_ratio =
            1 - (growJuiceTimer / IN_WORLD_FRACTION::GROW_JUICE_DURATION);
        anim_ratio = ease(EASE_METHOD_UP_AND_DOWN, anim_ratio);
        size_mult += IN_WORLD_FRACTION::GROW_JUICE_AMOUNT * anim_ratio;
    }
    
    ALLEGRO_COLOR final_color;
    if(reqMetJuiceTimer > 0.0f) {
        final_color =
            interpolateColor(
                reqMetJuiceTimer, 0.0f,
                IN_WORLD_FRACTION::REQ_MET_JUICE_DURATION,
                color, COLOR_WHITE
            );
            
        float anim_ratio =
            1.0f -
            (reqMetJuiceTimer / IN_WORLD_FRACTION::REQ_MET_JUICE_DURATION);
        anim_ratio = ease(EASE_METHOD_UP_AND_DOWN, anim_ratio);
        size_mult += IN_WORLD_FRACTION::REQ_MET_GROW_JUICE_AMOUNT * anim_ratio;
    } else {
        final_color = color;
    }
    final_color.a *= alpha_mult;
    
    if(requirementNumber > 0) {
        Point pos(m->pos.x, m->pos.y - m->radius - IN_WORLD_FRACTION::PADDING);
        drawFraction(
            pos,
            valueNumber, requirementNumber, final_color, size_mult
        );
    } else {
        Point pos(
            m->pos.x,
            m->pos.y - m->radius -
            al_get_font_line_height(game.sysContent.fntStandard) -
            IN_WORLD_FRACTION::PADDING
        );
        drawText(
            i2s(valueNumber), game.sysContent.fntStandard, pos,
            Point(LARGE_FLOAT, IN_WORLD_FRACTION::ROW_HEIGHT * size_mult),
            final_color
        );
    }
}


/**
 * @brief Sets the color.
 *
 * @param new_color Color to set to.
 */
void InWorldFraction::setColor(const ALLEGRO_COLOR &new_color) {
    if(color == new_color) return;
    
    color = new_color;
    growJuiceTimer = IN_WORLD_FRACTION::GROW_JUICE_DURATION;
}


/**
 * @brief Sets the requirement number.
 *
 * @param new_req_nr Requirement number to set to.
 */
void InWorldFraction::setRequirementNumber(float new_req_nr) {
    if(requirementNumber == new_req_nr) return;
    
    bool req_was_met = valueNumber >= requirementNumber;
    requirementNumber = new_req_nr;
    
    if(
        requirementNumber > 0.0f &&
        !req_was_met &&
        valueNumber >= requirementNumber
    ) {
        reqMetJuiceTimer = IN_WORLD_FRACTION::REQ_MET_JUICE_DURATION;
    } else {
        growJuiceTimer = IN_WORLD_FRACTION::GROW_JUICE_DURATION;
    }
}


/**
 * @brief Sets the value number.
 *
 * @param new_value_nr Value number to set to.
 */
void InWorldFraction::setValueNumber(float new_value_nr) {
    if(valueNumber == new_value_nr) return;
    
    bool req_was_met = valueNumber >= requirementNumber;
    
    valueNumber = new_value_nr;
    
    if(
        requirementNumber > 0.0f &&
        !req_was_met &&
        valueNumber >= requirementNumber
    ) {
        reqMetJuiceTimer = IN_WORLD_FRACTION::REQ_MET_JUICE_DURATION;
    } else {
        growJuiceTimer = IN_WORLD_FRACTION::GROW_JUICE_DURATION;
    }
}


/**
 * @brief Starts fading away.
 */
void InWorldFraction::startFading() {
    if(transition == IN_WORLD_HUD_TRANSITION_OUT) {
        return;
    }
    transition = IN_WORLD_HUD_TRANSITION_OUT;
    transitionTimer = IN_WORLD_FRACTION::TRANSITION_OUT_DURATION;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void InWorldFraction::tick(float delta_t) {
    InWorldHudItem::tick(delta_t);
    if(growJuiceTimer > 0.0f) {
        growJuiceTimer -= delta_t;
    }
    if(reqMetJuiceTimer > 0.0f) {
        reqMetJuiceTimer -= delta_t;
    }
}


/**
 * @brief Constructs a new in-world health wheel object.
 *
 * @param m Mob it belongs to.
 */
InWorldHealthWheel::InWorldHealthWheel(Mob* m) :
    InWorldHudItem(m) {
    
    if(m->maxHealth > 0.0f) {
        visibleRatio = m->health / m->maxHealth;
    }
    transitionTimer = IN_WORLD_HEALTH_WHEEL::TRANSITION_IN_DURATION;
}


/**
 * @brief Draws an in-world health wheel.
 */
void InWorldHealthWheel::draw() {
    float alpha_mult = 1.0f;
    float size_mult = 1.0f;
    switch(transition) {
    case IN_WORLD_HUD_TRANSITION_IN: {
        float timer_ratio =
            1.0f -
            (transitionTimer / IN_WORLD_HEALTH_WHEEL::TRANSITION_IN_DURATION);
        alpha_mult = timer_ratio;
        size_mult = ease(EASE_METHOD_OUT, timer_ratio) * 0.5 + 0.5;
        break;
    }
    case IN_WORLD_HUD_TRANSITION_OUT: {
        alpha_mult =
            transitionTimer / IN_WORLD_HEALTH_WHEEL::TRANSITION_OUT_DURATION;
        break;
    }
    default: {
        break;
    }
    }
    
    float radius = DRAWING::DEF_HEALTH_WHEEL_RADIUS * size_mult;
    drawHealth(
        Point(
            m->pos.x,
            m->pos.y - m->radius -
            radius - IN_WORLD_HEALTH_WHEEL::PADDING
        ),
        visibleRatio,
        IN_WORLD_HEALTH_WHEEL::OPACITY * alpha_mult,
        radius
    );
}


/**
 * @brief Starts fading away.
 */
void InWorldHealthWheel::startFading() {
    if(transition == IN_WORLD_HUD_TRANSITION_OUT) {
        return;
    }
    transition = IN_WORLD_HUD_TRANSITION_OUT;
    transitionTimer = IN_WORLD_HEALTH_WHEEL::TRANSITION_OUT_DURATION;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void InWorldHealthWheel::tick(float delta_t) {
    InWorldHudItem::tick(delta_t);
    
    if(m->maxHealth == 0.0f) return;
    
    visibleRatio +=
        ((m->health / m->maxHealth) - visibleRatio) *
        (IN_WORLD_HEALTH_WHEEL::SMOOTHNESS_MULT * delta_t);
}


/**
 * @brief Constructs a new in-world HUD item object.
 *
 * @param m Mob it belongs to.
 */
InWorldHudItem::InWorldHudItem(Mob* m) :
    m(m) {
    
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void InWorldHudItem::tick(float delta_t) {
    switch(transition) {
    case IN_WORLD_HUD_TRANSITION_IN: {
        transitionTimer -= delta_t;
        if(transitionTimer <= 0.0f) {
            transition = IN_WORLD_HUD_TRANSITION_NONE;
        }
        break;
    }
    case IN_WORLD_HUD_TRANSITION_OUT: {
        transitionTimer -= delta_t;
        if(transitionTimer <= 0.0f) {
            toDelete = true;
        }
        break;
    }
    default: {
        break;
    }
    }
}
