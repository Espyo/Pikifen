/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * In-world HUD class and in-world HUD-related functions.
 */

#include "in_world_hud.hpp"

#include "../../content/mob/mob.hpp"
#include "../../core/drawing.hpp"
#include "../../core/game.hpp"
#include "../../core/misc_functions.hpp"
#include "../../util/allegro_utils.hpp"
#include "../../util/string_utils.hpp"


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

//Standard alpha [0 - 1].
const float ALPHA = 0.85f;

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

//Standard alpha [0 - 1].
const float ALPHA = 0.85f;

//Corner radius of each bar, in size ratio.
const float CORNER_RADIUS = 0.20f;

//Height of each bar.
const float HEIGHT = 16.0f;

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
                    m->center.x,
                    m->center.y - m->radius - IN_WORLD_FRACTION::PADDING
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
                    m->center.x,
                    m->center.y - m->radius -
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
void InWorldFraction::startFadingOut() {
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
InWorldMobStatus::InWorldMobStatus(Mob* m) :
    InWorldHudItem(m) {
    
    if(m->maxHealth > 0.0f) {
        healthVisibleRatio = m->health / m->maxHealth;
    }
    transitionTimer = IN_WORLD_HEALTH_WHEEL::TRANSITION_IN_DURATION;
}


/**
 * @brief Aborts the fading away process.
 */
void InWorldMobStatus::abortFadeOut() {
    if(transition != IN_WORLD_HUD_TRANSITION_OUT) {
        return;
    }
    float remainingRatio =
        transitionTimer / IN_WORLD_HEALTH_WHEEL::TRANSITION_OUT_DURATION;
    transition = IN_WORLD_HUD_TRANSITION_IN;
    transitionTimer =
        remainingRatio * IN_WORLD_HEALTH_WHEEL::TRANSITION_IN_DURATION;
}


/**
 * @brief Draws an in-world health wheel, and any status buildup bars.
 */
void InWorldMobStatus::draw() {
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
    float curYOffset = m->radius + IN_WORLD_HEALTH_WHEEL::PADDING + wheelRadius;
    
    drawHealthWheel(wheelRadius, curYOffset, alphaMult);
    
    //Draw status bars.
    Point buildupBarSize =
        Point(
            IN_WORLD_STATUS_BUILDUP::WIDTH, IN_WORLD_STATUS_BUILDUP::HEIGHT
        ) * sizeMult;
    curYOffset += wheelRadius + IN_WORLD_STATUS_BUILDUP::PADDING;  
    drawStatuses(buildupBarSize, curYOffset, alphaMult);

}


/**
 * @brief Draws an in-world health wheel.
 * 
 * @param radius Radius to draw the wheel.
 * @param yOffset Offset to draw the wheel from the mob.
 * @param alpha Opacity to draw the wheel.
 */
void InWorldMobStatus::drawHealthWheel(float radius, float yOffset, float alpha) {
    Point center = Point(m->center.x, m->center.y - yOffset);
    const ALLEGRO_COLOR CHART_COLOR = al_map_rgb(10, 25, 35);

    ALLEGRO_SHADER* healthShader = game.shaders.getShader(SHADER_TYPE_SCANLINE);
    if(healthShader) {
        al_use_shader(healthShader);
        al_set_shader_float("image_height", 1.0); //Pieslice prims map UV to [-r, r], so we don't need to scale based on image size
        al_set_shader_float("area_time", game.timePassed * 2);
        al_set_shader_float("intensity", 0.2f);
        al_set_shader_float("frequency", 2.0f);
    }

    //Backing of the wheel
    al_draw_filled_circle(
        center.x, center.y, radius, multAlpha(CHART_COLOR, 0.5f * alpha)
    );

    //Draw pieslice
    drawHealthFill(
        Point(center.x, center.y),
        healthVisibleRatio,
        IN_WORLD_HEALTH_WHEEL::ALPHA * alpha,
        radius
    );

    //Additive glow
    AllegroBlenderState prevBlender;
    prevBlender.save();
    al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_ONE);
    drawBitmapInBox(
        game.sysContent.bmpHealthGlow,
        Point(center.x, center.y),
        Point(radius * 2, radius * 2),
        true, true, 0.0f,
        multAlpha(COLOR_WHITE, alpha)
    );
    prevBlender.loadIfHasData();

    //Don't use a shader for the border.
    al_use_shader(nullptr);
    al_draw_circle(
        center.x, center.y, radius + 1, multAlpha(CHART_COLOR, alpha), 2
    );
}

/**
 * @brief Draws in-world status buildup bars.
 * 
 * @param barSize Size of the bar, including the outline.
 * @param yOffset Offset to draw the bar from the mob.
 * @param alpha Opacity to draw the wheel.
 */
void InWorldMobStatus::drawStatuses(Point barSize, float yOffset, float alpha) {
    const auto drawNextBar =
        [this, &yOffset, &alpha, &barSize]
    (float fillRatio, const ALLEGRO_COLOR & color, bool drawCross) {
        if(fillRatio <= 0.0f) return;
        
        yOffset +=
            IN_WORLD_STATUS_BUILDUP::PADDING + IN_WORLD_STATUS_BUILDUP::HEIGHT;
        Point buildupBarCenter(m->center.x, m->center.y - yOffset);
        
        drawFilledRoundedRatioRectangle(
            buildupBarCenter,
            barSize,
            IN_WORLD_STATUS_BUILDUP::CORNER_RADIUS,
            changeAlpha(
                COLOR_BLACK, 255 * IN_WORLD_STATUS_BUILDUP::ALPHA * alpha
            )
        );
        
        Point filledBarSize =
            Point(
                barSize.x, barSize.y
            ) - IN_WORLD_STATUS_BUILDUP::OUTLINE_SIZE * 2.0f;
        filledBarSize.x *= fillRatio;
        filledBarSize.x = std::max(0.0f, filledBarSize.x);

        Point filledBarCenter(
            m->center.x - (barSize.x / 2.0f - IN_WORLD_STATUS_BUILDUP::OUTLINE_SIZE) +
            filledBarSize.x / 2.0f,
            m->center.y - yOffset
        );
        drawFilledRoundedRatioRectangle(
            filledBarCenter, filledBarSize,
            IN_WORLD_STATUS_BUILDUP::CORNER_RADIUS,
            changeAlpha(
                color, 255 * IN_WORLD_STATUS_BUILDUP::ALPHA * alpha
            )
        );
        
        if(drawCross) {
            al_draw_line(
                buildupBarCenter.x - barSize.x / 2.0f,
                buildupBarCenter.y - barSize.y / 2.0f,
                buildupBarCenter.x + barSize.x / 2.0f,
                buildupBarCenter.y + barSize.y / 2.0f,
                al_map_rgb(128, 64, 32), 2.0f
            );
            al_draw_line(
                buildupBarCenter.x + barSize.x / 2.0f,
                buildupBarCenter.y - barSize.y / 2.0f,
                buildupBarCenter.x - barSize.x / 2.0f,
                buildupBarCenter.y + barSize.y / 2.0f,
                al_map_rgb(128, 64, 32), 2.0f
            );
        }
    };
    
    forIdx(b, m->statuses.getBuildups()) {
        const StatusBuildup* bPtr = &m->statuses.getBuildups()[b];
        
        drawNextBar(
            bPtr->amount,
            bPtr->type->color, false
        );
    }
    
    forIdx(c, m->statuses.getCooldowns()) {
        const StatusCooldown* cPtr = &m->statuses.getCooldowns()[c];
        
        drawNextBar(
            cPtr->timeLeft / cPtr->type->cooldown,
            cPtr->type->color, true
        );
    }
}


/**
 * @brief Starts fading away.
 */
void InWorldMobStatus::startFadingOut() {
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
void InWorldMobStatus::tick(float deltaT) {
    InWorldHudItem::tick(deltaT);
    
    if(m->maxHealth == 0.0f) return;
    
    healthVisibleRatio +=
        ((m->health / m->maxHealth) - healthVisibleRatio) *
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
