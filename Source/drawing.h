/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the drawing-related functions.
 */

#ifndef DRAWING_INCLUDED
#define DRAWING_INCLUDED

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "controls.h"
#include "functions.h"

void do_game_drawing();

bool casts_shadow(sector* s1, sector* s2);
void draw_control(const ALLEGRO_FONT* const font, const control_info &c, const float x, const float y, const float max_w, const float max_h);
void draw_compressed_text(const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color, const float x, const float y, const int flags, const unsigned char valign, const float max_w, const float max_h, const string &text);
void draw_fraction(const float cx, const float cy, const unsigned int current, const unsigned int needed, const ALLEGRO_COLOR &color);
void draw_health(const float cx, const float cy, const unsigned int health, const unsigned int max_health, const float radius = 20, const bool just_chart = false);
void draw_loading_screen(const string &area_name, const string &subtitle, const float opacity);
void draw_sector(sector* s_ptr, const float x, const float y, const float scale, sector_texture_info* texture = NULL);
void draw_sector_texture(sector* s_ptr, const float x, const float y, const float scale, sector_texture_info* texture = NULL);
void draw_mob_shadow(const float cx, const float cy, const float size, const float delta_z, const float shadow_stretch);
void draw_scaled_text(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color, const float x, const float y,
    const float scale_x, const float scale_y, const int flags, const unsigned char valign, const string &text
);
void draw_sprite(ALLEGRO_BITMAP* bmp, const float cx, const float cy, const float w, const float h, const float angle = 0, const ALLEGRO_COLOR &tint = al_map_rgb(255, 255, 255));
void draw_text_lines(const ALLEGRO_FONT* const f, const ALLEGRO_COLOR &c, const float x, const float y, const int fl, const unsigned char va, const string &text);
float ease(const unsigned char method, float y);


enum EASING_METHODS {
    EASE_IN,
    EASE_OUT,
    EASE_UP_AND_DOWN,
};

const float WALL_SHADOW_LENGTH  = 32;  //The shadows of walls spread this much outwards.
const float WALL_SHADOW_OPACITY = 192; //The shadows of walls start with this opacity and fade to 0.

#endif //ifndef DRAWING_INCLUDED
