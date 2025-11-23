/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for drawing-related utility functions.
 * These don't contain logic specific to the Pikifen project.
 */

#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>

#include "general_utils.h"
#include "geometry_utils.h"


//Full-white opaque color.
constexpr ALLEGRO_COLOR COLOR_WHITE = { 1.0f, 1.0f, 1.0f, 1.0f };

//Full-black opaque color.
constexpr ALLEGRO_COLOR COLOR_BLACK = { 0.0f, 0.0f, 0.0f, 1.0f };

//Fully-transparent color, in black.
constexpr ALLEGRO_COLOR COLOR_EMPTY = { 0.0f, 0.0f, 0.0f, 0.0f };

//Fully-transparent color, in black.
constexpr ALLEGRO_COLOR COLOR_EMPTY_WHITE = { 1.0f, 1.0f, 1.0f, 0.0f };

//Transparent white color, usually for menu headers.
constexpr ALLEGRO_COLOR COLOR_TRANSPARENT_WHITE = { 1.0f, 1.0f, 1.0f, 0.5f };


//Flags for text drawing settings.
enum TEXT_SETTING_FLAG {

    //The text can never be grown horizontally.
    TEXT_SETTING_FLAG_CANT_GROW_X = 1 << 0,
    
    //The text can never be grown vertically.
    TEXT_SETTING_FLAG_CANT_GROW_Y = 1 << 1,
    
    //The text can never be shrunk horizontally.
    TEXT_SETTING_FLAG_CANT_SHRINK_X = 1 << 2,
    
    //The text can never be shrunk vertically.
    TEXT_SETTING_FLAG_CANT_SHRINK_Y = 1 << 3,
    
    //If necessary, the text's aspect ratio can be changed.
    TEXT_SETTING_FLAG_CAN_CHANGE_RATIO = 1 << 4,
    
    //Compensate for the Y offset given by the font, by removing it.
    TEXT_SETTING_FLAG_COMPENSATE_YO = 1 << 5,
    
    //Utility flag -- The text can never be grown in any way.
    TEXT_SETTING_FLAG_CANT_GROW =
        TEXT_SETTING_FLAG_CANT_GROW_X |
        TEXT_SETTING_FLAG_CANT_GROW_Y,
        
    //Utility flag -- The text can never be shrunk in any way.
    TEXT_SETTING_FLAG_CANT_SHRINK =
        TEXT_SETTING_FLAG_CANT_SHRINK_X |
        TEXT_SETTING_FLAG_CANT_SHRINK_Y,
        
    //Utility flag -- The text can never be scaled horizontally in any way.
    TEXT_SETTING_FLAG_FIXED_WIDTH =
        TEXT_SETTING_FLAG_CANT_GROW_X |
        TEXT_SETTING_FLAG_CANT_SHRINK_X,
        
    //Utility flag -- The text can never be scaled vertically in any way.
    TEXT_SETTING_FLAG_FIXED_HEIGHT =
        TEXT_SETTING_FLAG_CANT_GROW_Y |
        TEXT_SETTING_FLAG_CANT_SHRINK_Y,
        
    //Utility flag -- The text can never be grown or shrunk in any way.
    TEXT_SETTING_FLAG_FIXED_SIZE =
        TEXT_SETTING_FLAG_CANT_GROW |
        TEXT_SETTING_FLAG_CANT_SHRINK,
};


void drawBitmap(
    ALLEGRO_BITMAP* bmp, const Point& center,
    const Point& size, float angle = 0,
    const ALLEGRO_COLOR& tint = COLOR_WHITE
);
void drawBitmapInBox(
    ALLEGRO_BITMAP* bmp, const Point& center,
    const Point& boxSize, bool scaleUp,
    float angle = 0,
    const ALLEGRO_COLOR& tint = COLOR_WHITE
);
void drawEquilateralTriangle(
    const Point& center, float radius, float angle,
    const ALLEGRO_COLOR& color, float thickness
);
void drawFilledDiamond(
    const Point& center, float radius, const ALLEGRO_COLOR& color
);
void drawFilledEquilateralTriangle(
    const Point& center, float radius, float angle,
    const ALLEGRO_COLOR& color
);
void drawFilledRoundedRectangle(
    const Point& center, const Point& size, float radii,
    const ALLEGRO_COLOR& color
);
void drawPrimRect(
    const Point& tl, const Point& size, const ALLEGRO_COLOR color,
    ALLEGRO_BITMAP* texture = nullptr
);
void drawRotatedRectangle(
    const Point& center, const Point& dimensions,
    float angle, const ALLEGRO_COLOR& color, float thickness
);
void drawRoundedRectangle(
    const Point& center, const Point& size, float radii,
    const ALLEGRO_COLOR& color, float thickness
);
void drawText(
    const string& text, const ALLEGRO_FONT* const font,
    const Point& where, const Point& boxSize,
    const ALLEGRO_COLOR& color = COLOR_WHITE,
    int textFlags = ALLEGRO_ALIGN_CENTER,
    V_ALIGN_MODE vAlign = V_ALIGN_MODE_CENTER, Bitmask8 settings = 0,
    const Point& furtherScale = Point(1.0f)
);
void drawTextLines(
    const string& text, const ALLEGRO_FONT* const font,
    const Point& where, const Point& boxSize,
    const ALLEGRO_COLOR& color = COLOR_WHITE,
    int text_flags = ALLEGRO_ALIGN_CENTER,
    V_ALIGN_MODE vAlign = V_ALIGN_MODE_CENTER, Bitmask8 settings = 0,
    const Point& furtherScale = Point(1.0f)
);
void drawTexturedBox(
    const Point& center, const Point& size, ALLEGRO_BITMAP* texture,
    const ALLEGRO_COLOR& tint = COLOR_WHITE
);
void getMultilineTextDimensions(
    const vector<string>& lines, const ALLEGRO_FONT* const font,
    int* outWidth, int* outHeight, int* outLineHeight
);
void getTextDrawingTransforms(
    const Point& where, const Point& scale,
    float textOrigOy, float vAlignOffset,
    ALLEGRO_TRANSFORM* outTextTransform, ALLEGRO_TRANSFORM* outOldTransform
);
