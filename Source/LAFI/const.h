#ifndef LAFI_CONST_INCLUDED
#define LAFI_CONST_INCLUDED

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#define LAFI_COLOR_SHIFT_DELTA 0.2 //A color is lightened/darkened by this much.

#define LAFI_CHECKBOX_BOX_SIZE 12
#define LAFI_CHECKBOX_BOX_PADDING 4 //How many pixels after the box should the label come.
#define LAFI_RADIO_BUTTON_BUTTON_SIZE 12
#define LAFI_RADIO_BUTTON_BUTTON_PADDING 4

#define LAFI_DEF_STYLE_BG_R 192
#define LAFI_DEF_STYLE_BG_G 192
#define LAFI_DEF_STYLE_BG_B 192
#define LAFI_DEF_STYLE_FG_R 0
#define LAFI_DEF_STYLE_FG_G 0
#define LAFI_DEF_STYLE_FG_B 0
#define LAFI_DEF_STYLE_ALT_R 96
#define LAFI_DEF_STYLE_ALT_G 96
#define LAFI_DEF_STYLE_ALT_B 192

#define LAFI_STANDARD_SCROLLBAR_WIDTH 20

enum LAFI_WIDGET_FLAGS {
    LAFI_FLAG_DISABLED = 1,
    LAFI_FLAG_INVISIBLE = 2,
    LAFI_FLAG_NOT_SELECTABLE = 4,
    LAFI_FLAG_NO_CLIPPING_RECTANGLE = 8,
};

enum LAFI_DRAW_LINE_SIDES {
    LAFI_DRAW_LINE_RIGHT,
    LAFI_DRAW_LINE_TOP,
    LAFI_DRAW_LINE_LEFT,
    LAFI_DRAW_LINE_BOTTOM,
};

enum LAFI_EASY_FLAGS {
    LAFI_EASY_FLAG_WIDTH_PX = 1,  //Width is in pixels, not percentage.
};

#endif //ifndef LAFI_CONST_INCLUDED