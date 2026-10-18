/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Maker tool structures and functions.
 */

#include "display.hpp"

#include "../game.hpp"


/**
 * @brief Clears the data of the display.
 */
void MakerDisplay::clear() {
    text.clear();
    visibilityState = OPEN_CLOSE_STATE_CLOSED;
    visibility = 0.0f;
    timeLeft = 0.0f;
}


/**
 * @brief Closes the display,
 * gradually hiding it unless specified otherwise.
 *
 * @param instant Whether to close it instantaneously.
 */
void MakerDisplay::close(bool instant) {
    switch(visibilityState) {
    case OPEN_CLOSE_STATE_CLOSED:
    case OPEN_CLOSE_STATE_CLOSING: {
        break;
    } case OPEN_CLOSE_STATE_OPEN:
    case OPEN_CLOSE_STATE_OPENING: {
        if(instant) {
            visibilityState = OPEN_CLOSE_STATE_CLOSED;
            visibility = 0.0f;
        } else {
            visibilityState = OPEN_CLOSE_STATE_CLOSING;
        }
        break;
    }
    }
}


/**
 * @brief Draws the maker display on-screen.
 *
 * @param y Y coordinate to draw at.
 * @return Its height.
 */
int MakerDisplay::draw(int y) const {
    if(visibilityState == OPEN_CLOSE_STATE_CLOSED) return 0;
    
    //Setup.
    const ALLEGRO_FONT* FONT = game.sysContent.fntBuiltin;
    const vector<string> lines = split(text, "\n", true);
    const int fontHeight = al_get_font_line_height(FONT);
    
    int textW = game.winW - CONSOLE::PADDING * 4;
    int textH =
        lines.size() * fontHeight +
        (lines.size() - 1) * CONSOLE::LINE_PADDING;
    int boxW = textW + CONSOLE::PADDING * 2;
    int boxH = textH + CONSOLE::PADDING * 2;
    int boxX = CONSOLE::PADDING;
    int boxY = y;
    int textX = boxX + CONSOLE::PADDING;
    int textY = boxY + CONSOLE::PADDING;
    
    float alpha = visibility;
    
    const auto getLineY =
    [textY, fontHeight] (size_t idx) {
        return textY + (idx * (fontHeight + CONSOLE::LINE_PADDING));
    };
    
    //Background.
    al_draw_filled_rectangle(
        boxX, boxY, boxX + boxW, boxY + boxH,
        multAlpha(CONSOLE::COLOR_BG, alpha)
    );
    
    //Text lines.
    forIdx(l, lines) {
        int lineY = getLineY(l);
        
        drawText(
            lines[l], FONT,
            Point(textX, lineY),
            Point(textW, fontHeight),
            multAlpha(CONSOLE::COLOR_MAIN, alpha),
            ALLEGRO_ALIGN_LEFT, V_ALIGN_MODE_TOP,
            TEXT_SETTING_FLAG_CANT_GROW | TEXT_SETTING_FLAG_CANT_SHRINK
        );
    }
    
    //Decor.
    float decorWidth = std::min(40.0f, boxW / 2.0f);
    float decorHeight = std::min(40.0f, boxH / 2.0f);
    al_draw_line(
        boxX + 0.0f, boxY - 1.0f,
        boxX + decorWidth, boxY - 1.0f,
        multAlpha(CONSOLE::COLOR_DECOR_LIGHT, alpha), 2.0f
    );
    al_draw_line(
        boxX + 0.0f, boxY + 1.0f,
        boxX + decorWidth, boxY + 1.0f,
        multAlpha(CONSOLE::COLOR_DECOR_DARK, alpha), 2.0f
    );
    al_draw_line(
        boxX + 0.0f - 1.0f, boxY + 0.0f,
        boxX + 0.0f - 1.0f, boxY + decorHeight,
        multAlpha(CONSOLE::COLOR_DECOR_LIGHT, alpha), 2.0f
    );
    al_draw_line(
        boxX + 0.0f + 1.0f, boxY + 0.0f,
        boxX + 0.0f + 1.0f, boxY + decorHeight,
        multAlpha(CONSOLE::COLOR_DECOR_DARK, alpha), 2.0f
    );
    
    al_draw_line(
        boxX + boxW - 0.0f, boxY - 1.0f,
        boxX + boxW - decorWidth, boxY - 1.0f,
        multAlpha(CONSOLE::COLOR_DECOR_LIGHT, alpha), 2.0f
    );
    al_draw_line(
        boxX + boxW - 0.0f, boxY + 1.0f,
        boxX + boxW - decorWidth, boxY + 1.0f,
        multAlpha(CONSOLE::COLOR_DECOR_DARK, alpha), 2.0f
    );
    al_draw_line(
        boxX + boxW - 1.0f, boxY + 0.0f,
        boxX + boxW - 1.0f, boxY + decorHeight,
        multAlpha(CONSOLE::COLOR_DECOR_LIGHT, alpha), 2.0f
    );
    al_draw_line(
        boxX + boxW + 1.0f, boxY + 0.0f,
        boxX + boxW + 1.0f, boxY + decorHeight,
        multAlpha(CONSOLE::COLOR_DECOR_DARK, alpha), 2.0f
    );
    
    al_draw_line(
        boxX + 0.0f, boxY + boxH - 1.0f,
        boxX + decorWidth, boxY + boxH - 1.0f,
        multAlpha(CONSOLE::COLOR_DECOR_LIGHT, alpha), 2.0f
    );
    al_draw_line(
        boxX + 0.0f, boxY + boxH + 1.0f,
        boxX + decorWidth, boxY + boxH + 1.0f,
        multAlpha(CONSOLE::COLOR_DECOR_DARK, alpha), 2.0f
    );
    al_draw_line(
        boxX + 0.0f - 1.0f, boxY + boxH - 0.0f,
        boxX + 0.0f - 1.0f, boxY + boxH - decorHeight,
        multAlpha(CONSOLE::COLOR_DECOR_LIGHT, alpha), 2.0f
    );
    al_draw_line(
        boxX + 0.0f + 1.0f, boxY + boxH - 0.0f,
        boxX + 0.0f + 1.0f, boxY + boxH - decorHeight,
        multAlpha(CONSOLE::COLOR_DECOR_DARK, alpha), 2.0f
    );
    
    al_draw_line(
        boxX + boxW - 0.0f, boxY + boxH - 1.0f,
        boxX + boxW - decorWidth, boxY + boxH - 1.0f,
        multAlpha(CONSOLE::COLOR_DECOR_LIGHT, alpha), 2.0f
    );
    al_draw_line(
        boxX + boxW - 0.0f, boxY + boxH + 1.0f,
        boxX + boxW - decorWidth, boxY + boxH + 1.0f,
        multAlpha(CONSOLE::COLOR_DECOR_DARK, alpha), 2.0f
    );
    al_draw_line(
        boxX + boxW - 1.0f, boxY + boxH - 0.0f,
        boxX + boxW - 1.0f, boxY + boxH - decorHeight,
        multAlpha(CONSOLE::COLOR_DECOR_LIGHT, alpha), 2.0f
    );
    al_draw_line(
        boxX + boxW + 1.0f, boxY + boxH - 0.0f,
        boxX + boxW + 1.0f, boxY + boxH - decorHeight,
        multAlpha(CONSOLE::COLOR_DECOR_DARK, alpha), 2.0f
    );
    
    return boxH;
}


/**
 * @brief Opens the display,
 * gradually showing it unless specified otherwise.
 *
 * @param instant Whether to open it instantaneously.
 */
void MakerDisplay::open(bool instant) {
    switch(visibilityState) {
    case OPEN_CLOSE_STATE_OPEN:
    case OPEN_CLOSE_STATE_OPENING: {
        break;
    } case OPEN_CLOSE_STATE_CLOSED:
    case OPEN_CLOSE_STATE_CLOSING: {
        if(instant) {
            visibilityState = OPEN_CLOSE_STATE_OPEN;
            visibility = 1.0f;
        } else {
            visibilityState = OPEN_CLOSE_STATE_OPENING;
        }
        break;
    }
    }
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param deltaT How long the frame's tick is, in seconds.
 */
void MakerDisplay::tick(float deltaT) {
    if(timeLeft > 0.0f) {
        timeLeft -= deltaT;
        if(timeLeft <= 0.0f) {
            timeLeft = 0.0f;
            close();
        }
    }
    
    switch(visibilityState) {
    case OPEN_CLOSE_STATE_CLOSED:
    case OPEN_CLOSE_STATE_OPEN: {
        break;
    } case OPEN_CLOSE_STATE_CLOSING:
    case OPEN_CLOSE_STATE_OPENING: {
        visibility =
            inchTowards(
                visibility,
                visibilityState == OPEN_CLOSE_STATE_CLOSING ? 0.0f : 1.0f,
                CONSOLE::VISIBILITY_CHANGE_SPEED * deltaT
            );
        if(visibility >= 1.0f) {
            visibility = 1.0f;
            visibilityState = OPEN_CLOSE_STATE_OPEN;
        } else if(visibility <= 0.0f) {
            visibility = 0.0f;
            visibilityState = OPEN_CLOSE_STATE_CLOSED;
        }
        break;
    }
    }
}


/**
 * @brief Sets the text to show on the display.
 *
 * @param text Text to show.
 * @param duration How long to keep the display on after this.
 */
void MakerDisplay::write(
    const string& text, float duration
) {
    this->text = text;
    timeLeft = duration;
    open();
}
