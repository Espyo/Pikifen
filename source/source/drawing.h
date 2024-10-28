/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the drawing-related functions.
 */

#pragma once

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

#include "controls.h"
#include "liquid.h"
#include "misc_structs.h"


namespace CONTROL_BIND_ICON {
extern const ALLEGRO_COLOR BASE_OUTLINE_COLOR;
extern const ALLEGRO_COLOR BASE_RECT_COLOR;
extern const ALLEGRO_COLOR BASE_TEXT_COLOR;
extern const float PADDING;
extern const float OUTLINE_THICKNESS;
}


namespace DRAWING {
extern const float DEF_HEALTH_WHEEL_RADIUS;
extern const float LIQUID_WOBBLE_DELTA_X;
extern const float LIQUID_WOBBLE_TIME_SCALE;
extern const int LOADING_SCREEN_PADDING;
extern const float LOADING_SCREEN_SUBTEXT_SCALE;
extern const float LOADING_SCREEN_TEXT_HEIGHT;
extern const float LOADING_SCREEN_TEXT_WIDTH;
extern const unsigned char NOTIFICATION_ALPHA;
extern const float NOTIFICATION_CONTROL_SIZE;
extern const float NOTIFICATION_PADDING;
}


//Icons for the menu buttons.
enum MENU_ICON {
    //Main menu play button.
    MENU_ICON_PLAY,
    
    //Main menu make button.
    MENU_ICON_MAKE,
    
    //Main menu options button.
    MENU_ICON_OPTIONS,
    
    //Main menu statistics button.
    MENU_ICON_STATISTICS,
    
    //Main menu quit button.
    MENU_ICON_QUIT,
    
    //Main menu simple areas button.
    MENU_ICON_SIMPLE_AREAS,
    
    //Main menu missions button.
    MENU_ICON_MISSIONS,
    
    //Main menu animation editor button.
    MENU_ICON_ANIM_EDITOR,
    
    //Main menu area editor button.
    MENU_ICON_AREA_EDITOR,
    
    //Main menu GUI editor button.
    MENU_ICON_GUI_EDITOR,
    
    //Options menu controls button.
    MENU_ICON_CONTROLS,
    
    //Options menu graphics button.
    MENU_ICON_GRAPHICS,
    
    //Options menu audio button.
    MENU_ICON_AUDIO,
    
    //Options menu misc. button.
    MENU_ICON_OPTIONS_MISC,
};


//Possible shapes for a player input icon.
enum PLAYER_INPUT_ICON_SHAPE {

    //Doesn't really have a shape, but instead draws a bitmap.
    PLAYER_INPUT_ICON_SHAPE_BITMAP,
    
    //Rectangle shape, representing keyboard keys.
    PLAYER_INPUT_ICON_SHAPE_RECTANGLE,
    
    //Circle/ellipse shape, representing buttons.
    PLAYER_INPUT_ICON_SHAPE_ROUNDED,
    
};


//Player input icon spritesheet sprites.
//The order matches what's in the spritesheet.
enum PLAYER_INPUT_ICON_SPRITE {

    //Left mouse button.
    PLAYER_INPUT_ICON_SPRITE_LMB,
    
    //Right mouse button.
    PLAYER_INPUT_ICON_SPRITE_RMB,
    
    //Middle mouse button.
    PLAYER_INPUT_ICON_SPRITE_MMB,
    
    //Mouse wheel up.
    PLAYER_INPUT_ICON_SPRITE_MWU,
    
    //Mouse wheel down.
    PLAYER_INPUT_ICON_SPRITE_MWD,
    
    //Up key.
    PLAYER_INPUT_ICON_SPRITE_UP,
    
    //Left key.
    PLAYER_INPUT_ICON_SPRITE_LEFT,
    
    //Down key.
    PLAYER_INPUT_ICON_SPRITE_DOWN,
    
    //Right key.
    PLAYER_INPUT_ICON_SPRITE_RIGHT,
    
    //Backspace key.
    PLAYER_INPUT_ICON_SPRITE_BACKSPACE,
    
    //Shift key.
    PLAYER_INPUT_ICON_SPRITE_SHIFT,
    
    //Tab key.
    PLAYER_INPUT_ICON_SPRITE_TAB,
    
    //Enter key.
    PLAYER_INPUT_ICON_SPRITE_ENTER,
    
    //Game controller stick up.
    PLAYER_INPUT_ICON_SPRITE_STICK_UP,
    
    //Game controller stick left.
    PLAYER_INPUT_ICON_SPRITE_STICK_LEFT,
    
    //Game controller stick down.
    PLAYER_INPUT_ICON_SPRITE_STICK_DOWN,
    
    //Game controller stick right.
    PLAYER_INPUT_ICON_SPRITE_STICK_RIGHT,
    
};


void draw_background_logos(
    float time_spent, size_t rows, size_t cols,
    const point &logo_size, const ALLEGRO_COLOR &tint,
    const point &speed, float rotation_speed
);
void draw_bitmap_with_effects(
    ALLEGRO_BITMAP* bmp, const bitmap_effect_t &effects
);
void draw_button(
    const point &center, const point &size, const string &text,
    const ALLEGRO_FONT* font, const ALLEGRO_COLOR &color,
    bool selected,
    float juicy_grow_amount = 0.0f
);
void draw_fraction(
    const point &bottom, size_t value_nr,
    size_t requirement_nr, const ALLEGRO_COLOR &color, float scale
);
void draw_health(
    const point &center, float ratio,
    float alpha = 1.0f,
    float radius = DRAWING::DEF_HEALTH_WHEEL_RADIUS,
    bool just_chart = false
);
void draw_liquid(
    sector* s_ptr, liquid* l_ptr, const point &where, float scale,
    float time
);
void draw_loading_screen(
    const string &area_name, const string &subtitle, float opacity
);
void draw_menu_button_icon(
    MENU_ICON icon, const point &button_center, const point &button_size,
    bool left_side
);
void draw_mouse_cursor(const ALLEGRO_COLOR &color);
void draw_player_input_icon(
    const ALLEGRO_FONT* const font, const player_input &i,
    bool condensed, const point &where, const point &max_size,
    unsigned char alpha = 228
);
void draw_sector_texture(
    sector* s_ptr, const point &where, float scale, float opacity
);
void draw_sector_edge_offsets(
    sector* s_ptr, ALLEGRO_BITMAP* buffer, float opacity
);
void draw_mob_shadow(
    const mob* m,
    float delta_z, float shadow_stretch
);
void draw_status_effect_bmp(const mob* m, bitmap_effect_t &effects);
void draw_string_tokens(
    const vector<string_token> &tokens, const ALLEGRO_FONT* const text_font,
    const ALLEGRO_FONT* const control_font, bool controls_condensed,
    const point &where, int flags, const point &max_size,
    const point &scale = point(1.0f, 1.0f)
);
void get_player_input_icon_info(
    const player_input &i, bool condensed,
    PLAYER_INPUT_ICON_SHAPE* shape,
    PLAYER_INPUT_ICON_SPRITE* bitmap_sprite,
    string* text
);
float get_player_input_icon_width(
    const ALLEGRO_FONT* font, const player_input &i, bool condensed,
    float max_bitmap_height = 0
);
