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

//Golden-like color, usually for area names.
constexpr ALLEGRO_COLOR COLOR_GOLD = { 1.0f, 0.95f, 0.0f, 1.0f };

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


void draw_bitmap(
    ALLEGRO_BITMAP* bmp, const point &center,
    const point &size, const float angle = 0,
    const ALLEGRO_COLOR &tint = COLOR_WHITE
);
void draw_bitmap_in_box(
    ALLEGRO_BITMAP* bmp, const point &center,
    const point &box_size, const bool scale_up,
    const float angle = 0,
    const ALLEGRO_COLOR &tint = COLOR_WHITE
);
void draw_compressed_scaled_text(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const point &scale,
    const int flags, const V_ALIGN_MODE v_align,
    const point &max_size, const bool scale_past_max, const string &text
);
void draw_compressed_text(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const int flags, const V_ALIGN_MODE v_align,
    const point &max_size, const string &text
);
void draw_equilateral_triangle(
    const point &center, float radius, float angle,
    const ALLEGRO_COLOR &color, float thickness
);
void draw_filled_diamond(
    const point &center, const float radius, const ALLEGRO_COLOR &color
);
void draw_filled_equilateral_triangle(
    const point &center, float radius, float angle,
    const ALLEGRO_COLOR &color
);
void draw_filled_rounded_rectangle(
    const point &center, const point &size, const float radii,
    const ALLEGRO_COLOR &color
);
void draw_rotated_rectangle(
    const point &center, const point &dimensions,
    const float angle, const ALLEGRO_COLOR &color, const float thickness
);
void draw_rounded_rectangle(
    const point &center, const point &size, const float radii,
    const ALLEGRO_COLOR &color, const float thickness
);
void draw_scaled_text(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const point &scale,
    const int flags, const V_ALIGN_MODE v_align, const string &text
);
void draw_text(
    const string &text, const ALLEGRO_FONT* const font,
    const point &where, const point &box_size,
    const ALLEGRO_COLOR &color = COLOR_WHITE,
    int text_flags = ALLEGRO_ALIGN_CENTER,
    V_ALIGN_MODE v_align = V_ALIGN_MODE_CENTER, bitmask_8_t settings = 0,
    const point &further_scale = point(1.0f, 1.0f)
);
void draw_text_lines(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const int flags, const V_ALIGN_MODE v_align,
    const string &text
);
void draw_textured_box(
    const point &center, const point &size, ALLEGRO_BITMAP* texture,
    const ALLEGRO_COLOR &tint = COLOR_WHITE
);
void get_text_drawing_transforms(
    const point &where, const point &scale,
    float text_orig_oy, float v_align_offset,
    ALLEGRO_TRANSFORM* out_text_transform, ALLEGRO_TRANSFORM* out_old_transform
);
