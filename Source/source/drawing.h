/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the drawing-related functions.
 */

#ifndef DRAWING_INCLUDED
#define DRAWING_INCLUDED

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

#include "controls.h"
#include "liquid.h"
#include "misc_structs.h"


const float DEF_HEALTH_WHEEL_RADIUS = 20;
const float LIQUID_WOBBLE_TIME_SCALE = 2.0f;
const float LIQUID_WOBBLE_DELTA_X = 3.0f;
const float NOTIFICATION_PADDING = 8.0f;
const float NOTIFICATION_CONTROL_SIZE = 24.0f;
const unsigned char NOTIFICATION_ALPHA = 160;
//The shadows of walls start with this opacity and fade to 0.
const float WALL_SHADOW_OPACITY = 208;
//A water wave ring lasts this long.
const float WAVE_RING_DURATION = 1.0f;

enum EASING_METHODS {
    EASE_IN,
    EASE_OUT,
    EASE_UP_AND_DOWN,
};


void draw_background_logos(
    const float time_spent, const size_t rows, const size_t cols,
    const point &logo_size, const ALLEGRO_COLOR &tint,
    const point &speed, const float rotation_speed
);
void draw_bitmap(
    ALLEGRO_BITMAP* bmp, const point &center,
    const point &size, const float angle = 0,
    const ALLEGRO_COLOR &tint = al_map_rgb(255, 255, 255)
);
void draw_bitmap_in_box(
    ALLEGRO_BITMAP* bmp, const point &center,
    const point &box_size, const float angle = 0,
    const ALLEGRO_COLOR &tint = al_map_rgb(255, 255, 255)
);
void draw_bitmap_with_effects(
    ALLEGRO_BITMAP* bmp, const bitmap_effect_info &effects
);
void draw_button(
    const point &center, const point &size, const string &text,
    ALLEGRO_FONT* font, const ALLEGRO_COLOR &color,
    const bool selected
);
void draw_control(
    const ALLEGRO_FONT* const font, const control_info &c,
    const point &where, const point &max_size
);
void draw_compressed_text(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const int flags, const unsigned char valign,
    const point &max_size, const string &text
);
void draw_filled_diamond(
    const point &center, const float radius, const ALLEGRO_COLOR &color
);
void draw_fraction(
    const point &center, const size_t current,
    const size_t needed, const ALLEGRO_COLOR &color
);
void draw_health(
    const point &center, const float ratio, const float alpha = 1, 
    const float radius = DEF_HEALTH_WHEEL_RADIUS,
    const bool just_chart = false
);
void draw_liquid(
    sector* s_ptr, liquid* l_ptr, const point &where, const float scale
);
void draw_loading_screen(
    const string &area_name, const string &subtitle, const float opacity
);
void draw_notification(
    const point &where, const string &text,
    control_info* control = NULL
);
void draw_rotated_rectangle(
    const point &center, const point &dimensions,
    const float angle, const ALLEGRO_COLOR &color, const float thickness
);
void draw_sector_shadows(
    sector* s_ptr, const point &where, const float scale
);
void draw_sector_texture(
    sector* s_ptr, const point &where, const float scale, const float opacity
);
void draw_mob_shadow(
    const point &where, const float size,
    const float delta_z, const float shadow_stretch
);
void draw_scaled_text(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const point &scale,
    const int flags, const unsigned char valign, const string &text
);
void draw_status_effect_bmp(mob* m, bitmap_effect_info &effects);
void draw_text_lines(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const int flags, const unsigned char valign,
    const string &text
);
void draw_textured_box(
    const point &center, const point &size, ALLEGRO_BITMAP* texture,
    const ALLEGRO_COLOR &tint = al_map_rgb(255, 255, 255)
);
float ease(
    const unsigned char method, float y
);


#endif //ifndef DRAWING_INCLUDED
