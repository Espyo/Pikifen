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
#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/allegro_utils.h"
#include "../../util/string_utils.h"


namespace IN_WORLD_FRACTION {

//How much to grow when performing a juicy grow animation.
const float GROW_JUICE_AMOUNT = 0.06f;

//How long it takes to animate the numbers growing.
const float GROW_JUICE_DURATION = 0.3f;

//Width and height of the icon bitmap.
const float ICON_SIZE = 32.0f;

//The icon bitmap has this X offset from the fraction center.
const float ICON_X_OFFSET = -40.0f;

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


namespace IN_WORLD_STATUS_BUILDUP {

//Corner radius of each bar, in size ratio.
const float CORNER_RADIUS = 0.20f;

//Height of each bar.
const float HEIGHT = 16.0f;

//Standard opacity [0 - 1].
const float OPACITY = 0.85f;

//Size of the dark outline between the total bar and the filled portion.
const float OUTLINE_SIZE = 2.0f;

//Padding between health wheel and bars, and also between each bar.
const float PADDING = 4.0f;

//Width of each bar.
const float WIDTH = 64.0f;

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
    float alphaMult = 1.0f;
    float sizeMult = 1.0f;
    
    switch(transition) {
    case IN_WORLD_HUD_TRANSITION_IN: {
        float timerRatio =
            1 - (transitionTimer / IN_WORLD_FRACTION::TRANSITION_IN_DURATION);
        alphaMult = timerRatio;
        sizeMult = ease(timerRatio, EASE_METHOD_OUT) * 0.5 + 0.5;
        break;
    }
    case IN_WORLD_HUD_TRANSITION_OUT: {
        alphaMult =
            transitionTimer / IN_WORLD_FRACTION::TRANSITION_OUT_DURATION;
        break;
    }
    default: {
        break;
    }
    }
    
    if(growJuiceTimer > 0.0f) {
        float animRatio =
            1 - (growJuiceTimer / IN_WORLD_FRACTION::GROW_JUICE_DURATION);
        animRatio = ease(animRatio, EASE_METHOD_UP_AND_DOWN);
        sizeMult += IN_WORLD_FRACTION::GROW_JUICE_AMOUNT * animRatio;
    }
    
    ALLEGRO_COLOR finalColor;
    if(reqMetJuiceTimer > 0.0f) {
        finalColor =
            interpolateColor(
                reqMetJuiceTimer, 0.0f,
                IN_WORLD_FRACTION::REQ_MET_JUICE_DURATION,
                color, COLOR_WHITE
            );
            
        float animRatio =
            1.0f -
            (reqMetJuiceTimer / IN_WORLD_FRACTION::REQ_MET_JUICE_DURATION);
        animRatio = ease(animRatio, EASE_METHOD_UP_AND_DOWN);
        sizeMult += IN_WORLD_FRACTION::REQ_MET_GROW_JUICE_AMOUNT * animRatio;
    } else {
        finalColor = color;
    }
    finalColor.a *= alphaMult;
    
    Point pos = noMobPos;
    Point bmpPos = noMobPos;
    if(requirementNumber > 0) {
        if(m) {
            pos =
                Point(
                    m->pos.x,
                    m->pos.y - m->radius - IN_WORLD_FRACTION::PADDING
                );
        }
        bmpPos =
            Point(
                pos.x,
                pos.y - IN_WORLD_FRACTION::ROW_HEIGHT * 1.5f
            );
        drawFraction(
            pos,
            valueNumber, requirementNumber, finalColor, sizeMult
        );
    } else {
        if(m) {
            pos =
                Point(
                    m->pos.x,
                    m->pos.y - m->radius -
                    al_get_font_line_height(game.sysContent.fntStandard) -
                    IN_WORLD_FRACTION::PADDING
                );
        }
        bmpPos = pos;
        drawText(
            i2s(valueNumber), game.sysContent.fntStandard, pos,
            Point(LARGE_FLOAT, IN_WORLD_FRACTION::ROW_HEIGHT * sizeMult),
            finalColor
        );
    }
    
    if(bmpIcon) {
        bmpPos.x += IN_WORLD_FRACTION::ICON_X_OFFSET;
        drawBitmap(
            bmpIcon, bmpPos, Point(IN_WORLD_FRACTION::ICON_SIZE),
            0.0f, mapAlpha(finalColor.a * 255)
        );
    }
}


/**
 * @brief Sets the color.
 *
 * @param newColor Color to set to.
 */
void InWorldFraction::setColor(const ALLEGRO_COLOR& newColor) {
    if(color == newColor) return;
    
    color = newColor;
    growJuiceTimer = IN_WORLD_FRACTION::GROW_JUICE_DURATION;
}


/**
 * @brief Sets the position for the fraction, in the case where there is no
 * associated mob.
 *
 * @param pos Position to use.
 */
void InWorldFraction::setNoMobPos(const Point& pos) {
    noMobPos = pos;
}


/**
 * @brief Sets the requirement number.
 *
 * @param newReqNr Requirement number to set to.
 */
void InWorldFraction::setRequirementNumber(float newReqNr) {
    if(requirementNumber == newReqNr) return;
    
    bool reqWasMet = valueNumber >= requirementNumber;
    requirementNumber = newReqNr;
    
    if(
        requirementNumber > 0.0f &&
        !reqWasMet &&
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
 * @param newValueNr Value number to set to.
 */
void InWorldFraction::setValueNumber(float newValueNr) {
    if(valueNumber == newValueNr) return;
    
    bool reqWasMet = valueNumber >= requirementNumber;
    
    valueNumber = newValueNr;
    
    if(
        requirementNumber > 0.0f &&
        !reqWasMet &&
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
 * @param deltaT How long the frame's tick is, in seconds.
 */
void InWorldFraction::tick(float deltaT) {
    InWorldHudItem::tick(deltaT);
    if(growJuiceTimer > 0.0f) {
        growJuiceTimer -= deltaT;
    }
    if(reqMetJuiceTimer > 0.0f) {
        reqMetJuiceTimer -= deltaT;
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
 * @brief Draws an in-world health wheel, and any status buildup bars.
 */
void InWorldHealthWheel::draw() {
    //Setup.
    float alphaMult = 1.0f;
    float sizeMult = 1.0f;
    
    switch(transition) {
    case IN_WORLD_HUD_TRANSITION_IN: {
        float timerRatio =
            1.0f -
            (transitionTimer / IN_WORLD_HEALTH_WHEEL::TRANSITION_IN_DURATION);
        alphaMult = timerRatio;
        sizeMult = ease(timerRatio, EASE_METHOD_OUT) * 0.5 + 0.5;
        break;
    }
    case IN_WORLD_HUD_TRANSITION_OUT: {
        alphaMult =
            transitionTimer / IN_WORLD_HEALTH_WHEEL::TRANSITION_OUT_DURATION;
        break;
    }
    default: {
        break;
    }
    }
    
    float wheelRadius = DRAWING::DEF_HEALTH_WHEEL_RADIUS * sizeMult;
    Point buildupBarSize =
        Point(
            IN_WORLD_STATUS_BUILDUP::WIDTH, IN_WORLD_STATUS_BUILDUP::HEIGHT
        ) * sizeMult;
    float curYOffset = m->radius + IN_WORLD_HEALTH_WHEEL::PADDING + wheelRadius;
    
    //Draw the health wheel.
    drawHealth(
        Point(m->pos.x, m->pos.y - curYOffset),
        visibleRatio,
        IN_WORLD_HEALTH_WHEEL::OPACITY * alphaMult,
        wheelRadius
    );
    
    curYOffset += wheelRadius + IN_WORLD_STATUS_BUILDUP::PADDING;
    
    //Draw any status effect buildup bars.
    for(size_t s = 0; s < m->statuses.size(); s++) {
        Status* sPtr = &m->statuses[s];
        if(sPtr->type->buildup == 0.0f) continue;
        
        curYOffset +=
            IN_WORLD_STATUS_BUILDUP::PADDING + IN_WORLD_STATUS_BUILDUP::HEIGHT;
            
        drawFilledRoundedRatioRectangle(
            Point(m->pos.x, m->pos.y - curYOffset),
            buildupBarSize,
            IN_WORLD_STATUS_BUILDUP::CORNER_RADIUS,
            changeAlpha(
                COLOR_BLACK, 255 * IN_WORLD_STATUS_BUILDUP::OPACITY * alphaMult
            )
        );
        
        float filledWidth = IN_WORLD_STATUS_BUILDUP::WIDTH * sPtr->buildup;
        drawFilledRoundedRatioRectangle(
            Point(
                m->pos.x - IN_WORLD_STATUS_BUILDUP::WIDTH / 2.0f +
                filledWidth / 2.0f,
                m->pos.y - curYOffset
            ),
            Point(
                filledWidth, IN_WORLD_STATUS_BUILDUP::HEIGHT
            ) - IN_WORLD_STATUS_BUILDUP::OUTLINE_SIZE * 2.0f
            * sizeMult,
            IN_WORLD_STATUS_BUILDUP::CORNER_RADIUS,
            changeAlpha(
                sPtr->type->color,
                255 * IN_WORLD_STATUS_BUILDUP::OPACITY * alphaMult
            )
        );
    }
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
 * @param deltaT How long the frame's tick is, in seconds.
 */
void InWorldHealthWheel::tick(float deltaT) {
    InWorldHudItem::tick(deltaT);
    
    if(m->maxHealth == 0.0f) return;
    
    visibleRatio +=
        ((m->health / m->maxHealth) - visibleRatio) *
        (IN_WORLD_HEALTH_WHEEL::SMOOTHNESS_MULT * deltaT);
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
 * @param deltaT How long the frame's tick is, in seconds.
 */
void InWorldHudItem::tick(float deltaT) {
    switch(transition) {
    case IN_WORLD_HUD_TRANSITION_IN: {
        transitionTimer -= deltaT;
        if(transitionTimer <= 0.0f) {
            transition = IN_WORLD_HUD_TRANSITION_NONE;
        }
        break;
    }
    case IN_WORLD_HUD_TRANSITION_OUT: {
        transitionTimer -= deltaT;
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
