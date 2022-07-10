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


namespace CONTROL_ICON {
extern const ALLEGRO_COLOR BASE_OUTLINE_COLOR;
extern const ALLEGRO_COLOR BASE_RECT_COLOR;
extern const ALLEGRO_COLOR BASE_TEXT_COLOR;
extern const float CONTROL_ICON_PADDING;
extern const float OUTLINE_THICKNESS;
}


namespace DRAWING {
extern const float DEF_HEALTH_WHEEL_RADIUS;
extern const float LIQUID_WOBBLE_DELTA_X;
extern const float LIQUID_WOBBLE_TIME_SCALE;
extern const float LOADING_SCREEN_SUBTITLE_SCALE;
extern const int LOADING_SCREEN_PADDING;
extern const unsigned char NOTIFICATION_ALPHA;
extern const float NOTIFICATION_CONTROL_SIZE;
extern const float NOTIFICATION_PADDING;
}


//Possible shapes for a control icon.
enum CONTROL_ICON_SHAPES {
    //Doesn't really have a shape, but instead draws a bitmap.
    CONTROL_ICON_SHAPE_BITMAP,
    //Rectangle shape, representing keyboard keys.
    CONTROL_ICON_SHAPE_RECTANGLE,
    //Circle/rounded rectangle shape, representing buttons.
    CONTROL_ICON_SHAPE_ROUNDED,
};


//Control icon spritesheet sprites. The order matches what's in the spritesheet.
enum CONTROL_ICON_SPRITES {
    //Left mouse button.
    CONTROL_ICON_SPRITE_LMB,
    //Right mouse button.
    CONTROL_ICON_SPRITE_RMB,
    //Middle mouse button.
    CONTROL_ICON_SPRITE_MMB,
    //Mouse wheel up.
    CONTROL_ICON_SPRITE_MWU,
    //Mouse wheel down.
    CONTROL_ICON_SPRITE_MWD,
    //Up key.
    CONTROL_ICON_SPRITE_UP,
    //Left key.
    CONTROL_ICON_SPRITE_LEFT,
    //Down key.
    CONTROL_ICON_SPRITE_DOWN,
    //Right key.
    CONTROL_ICON_SPRITE_RIGHT,
    //Backspace key.
    CONTROL_ICON_SPRITE_BACKSPACE,
    //Shift key.
    CONTROL_ICON_SPRITE_SHIFT,
    //Tab key.
    CONTROL_ICON_SPRITE_TAB,
    //Enter key.
    CONTROL_ICON_SPRITE_ENTER,
    //Gamepad analog stick up.
    CONTROL_ICON_SPRITE_STICK_UP,
    //Gamepad analog stick left.
    CONTROL_ICON_SPRITE_STICK_LEFT,
    //Gamepad analog stick down.
    CONTROL_ICON_SPRITE_STICK_DOWN,
    //Gamepad analog stick right.
    CONTROL_ICON_SPRITE_STICK_RIGHT,
};


//Methods for easing numbers.
enum EASING_METHODS {
    //Eased as it goes in, then gradually goes out normally.
    EASE_IN,
    //Gradually goes in normally, then eased as it goes out.
    EASE_OUT,
    //Springs backwards before going in.
    EASE_IN_ELASTIC,
    //Near the end, it overshoots and then goes back in.
    EASE_OUT_ELASTIC,
    //Goes up to 1, then back down to 0, in a sine-wave.
    EASE_UP_AND_DOWN,
    //Goes up to 1, then down to 0, and wobbles around 0 for a bit.
    EASE_UP_AND_DOWN_ELASTIC,
};


//Ways to vertically align text when rendering it.
enum TEXT_VALIGN_MODES {
    //Align to the top.
    TEXT_VALIGN_TOP,
    //Align to the center.
    TEXT_VALIGN_CENTER,
    //Align to the bottom.
    TEXT_VALIGN_BOTTOM,
};


void draw_background_logos(
    const float time_spent, const size_t rows, const size_t cols,
    const point &logo_size, const ALLEGRO_COLOR &tint,
    const point &speed, const float rotation_speed
);
void draw_bitmap(
    ALLEGRO_BITMAP* bmp, const point &center,
    const point &size, const float angle = 0,
    const ALLEGRO_COLOR &tint = COLOR_WHITE
);
void draw_bitmap_in_box(
    ALLEGRO_BITMAP* bmp, const point &center,
    const point &box_size, const float angle = 0,
    const ALLEGRO_COLOR &tint = COLOR_WHITE
);
void draw_bitmap_with_effects(
    ALLEGRO_BITMAP* bmp, const bitmap_effect_info &effects
);
void draw_button(
    const point &center, const point &size, const string &text,
    ALLEGRO_FONT* font, const ALLEGRO_COLOR &color,
    const bool selected,
    const float juicy_grow_amount = 0.0f
);
void draw_control_icon(
    const ALLEGRO_FONT* const font, const control_info* c, const bool condensed,
    const point &where, const point &max_size, const unsigned char alpha = 228
);
void draw_compressed_scaled_text(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const point &scale,
    const int flags, const TEXT_VALIGN_MODES valign,
    const point &max_size, const bool scale_past_max, const string &text
);
void draw_compressed_text(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const int flags, const TEXT_VALIGN_MODES valign,
    const point &max_size, const string &text
);
void draw_filled_diamond(
    const point &center, const float radius, const ALLEGRO_COLOR &color
);
void draw_filled_rounded_rectangle(
    const point &center, const point &size, const float radii,
    const ALLEGRO_COLOR &color
);
void draw_fraction(
    const point &bottom, const size_t value_nr,
    const size_t requirement_nr, const ALLEGRO_COLOR &color,
    const float scale = 1.0f
);
void draw_health(
    const point &center, const float ratio,
    const float alpha = 1.0f,
    const float radius = DRAWING::DEF_HEALTH_WHEEL_RADIUS,
    const bool just_chart = false
);
void draw_liquid(
    sector* s_ptr, liquid* l_ptr, const point &where, const float scale
);
void draw_loading_screen(
    const string &area_name, const string &subtitle, const float opacity
);
void draw_rounded_rectangle(
    const point &center, const point &size, const float radii,
    const ALLEGRO_COLOR &color, const float thickness
);
void draw_rotated_rectangle(
    const point &center, const point &dimensions,
    const float angle, const ALLEGRO_COLOR &color, const float thickness
);
void draw_sector_texture(
    sector* s_ptr, const point &where, const float scale, const float opacity
);
void draw_sector_edge_offsets(
    sector* s_ptr, ALLEGRO_BITMAP* buffer, const float opacity
);
void draw_mob_shadow(
    const point &where, const float size,
    const float delta_z, const float shadow_stretch
);
void draw_scaled_text(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const point &scale,
    const int flags, const TEXT_VALIGN_MODES valign, const string &text
);
void draw_status_effect_bmp(mob* m, bitmap_effect_info &effects);
void draw_string_tokens(
    vector<string_token> &tokens, const ALLEGRO_FONT* const text_font,
    const ALLEGRO_FONT* const control_font, const point &where,
    const int flags, const point &max_size
);
void draw_text_lines(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const int flags, const TEXT_VALIGN_MODES valign,
    const string &text
);
void draw_textured_box(
    const point &center, const point &size, ALLEGRO_BITMAP* texture,
    const ALLEGRO_COLOR &tint = COLOR_WHITE
);
float ease(
    const EASING_METHODS method, float y
);
void get_control_icon_info(
    const control_info* c, const bool condensed,
    CONTROL_ICON_SHAPES* shape, CONTROL_ICON_SPRITES* bitmap_sprite,
    string* text
);
float get_control_icon_width(
    const ALLEGRO_FONT* font, const control_info* c, const bool condensed,
    const float max_bitmap_height = 0
);


#endif //ifndef DRAWING_INCLUDED
