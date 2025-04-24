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

#include "../content/other/liquid.h"
#include "controls_mediator.h"
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
    
    //Main menu help button.
    MENU_ICON_HELP,
    
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
    
    //Main menu particle editor button.
    MENU_ICON_PARTICLE_EDITOR,
    
    //Main menu GUI editor button.
    MENU_ICON_GUI_EDITOR,
    
    //Options menu controls button.
    MENU_ICON_CONTROLS,
    
    //Options menu graphics button.
    MENU_ICON_GRAPHICS,
    
    //Options menu audio button.
    MENU_ICON_AUDIO,
    
    //Options menu packs button.
    MENU_ICON_PACKS,
    
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


void drawBackgroundLogos(
    float timeSpent, size_t rows, size_t cols,
    const Point &logoSize, const ALLEGRO_COLOR &tint,
    const Point &speed, float rotationSpeed
);
void drawBitmapWithEffects(
    ALLEGRO_BITMAP* bmp, const BitmapEffect &effects
);
void drawButton(
    const Point &center, const Point &size, const string &text,
    const ALLEGRO_FONT* font, const ALLEGRO_COLOR &color,
    bool selected,
    float juicyGrowAmount = 0.0f
);
void drawFraction(
    const Point &bottom, size_t valueNr,
    size_t requirementNr, const ALLEGRO_COLOR &color, float scale
);
void drawHealth(
    const Point &center, float ratio,
    float alpha = 1.0f,
    float radius = DRAWING::DEF_HEALTH_WHEEL_RADIUS,
    bool justChart = false
);
void drawLiquid(
    Sector* sPtr, Liquid* lPtr, const Point &where, float scale,
    float time
);
void drawLoadingScreen(
    const string &areaName, const string &subtitle, float opacity
);
void drawMenuButtonIcon(
    MENU_ICON icon, const Point &buttonCenter, const Point &buttonSize,
    bool leftSide
);
void drawMouseCursor(const ALLEGRO_COLOR &color);
void drawPlayerInputSourceIcon(
    const ALLEGRO_FONT* const font, const PlayerInputSource &s,
    bool condensed, const Point &where, const Point &maxSize,
    unsigned char alpha = 228
);
void drawSectorTexture(
    Sector* sPtr, const Point &where, float scale, float opacity
);
void drawSectorEdgeOffsets(
    Sector* sPtr, ALLEGRO_BITMAP* buffer, float opacity
);
void drawMobShadow(
    const Mob* m,
    float deltaZ, float shadowStretch
);
void drawStatusEffectBmp(const Mob* m, BitmapEffect &effects);
void drawStringTokens(
    const vector<StringToken> &tokens, const ALLEGRO_FONT* const textFont,
    const ALLEGRO_FONT* const controlFont, bool controlsCondensed,
    const Point &where, int flags, const Point &maxSize,
    const Point &scale = Point(1.0f)
);
void getPlayerInputIconInfo(
    const PlayerInputSource &s, bool condensed,
    PLAYER_INPUT_ICON_SHAPE* shape,
    PLAYER_INPUT_ICON_SPRITE* bitmapSprite,
    string* text
);
float getPlayerInputIconWidth(
    const ALLEGRO_FONT* font, const PlayerInputSource &s, bool condensed,
    float maxBitmapHeight = 0
);
