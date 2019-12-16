#ifndef LAFI_CONST_INCLUDED
#define LAFI_CONST_INCLUDED

#define _USE_MATH_DEFINES
#include <math.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

namespace lafi {

//A color is lightened/darkened by this much.
const float COLOR_SHIFT_DELTA = 0.2f;

const int CHECKBOX_BOX_SIZE = 12;
//How many pixels after the box should the label come.
const int CHECKBOX_BOX_PADDING = 4;
const int RADIO_BUTTON_BUTTON_SIZE = 12;
const int RADIO_BUTTON_BUTTON_PADDING = 4;

const unsigned char DEF_STYLE_BG_R  = 192;
const unsigned char DEF_STYLE_BG_G  = 192;
const unsigned char DEF_STYLE_BG_B  = 192;
const unsigned char DEF_STYLE_FG_R  = 0;
const unsigned char DEF_STYLE_FG_G  = 0;
const unsigned char DEF_STYLE_FG_B  = 0;
const unsigned char DEF_STYLE_ALT_R = 96;
const unsigned char DEF_STYLE_ALT_G = 96;
const unsigned char DEF_STYLE_ALT_B = 192;

const int STANDARD_SCROLLBAR_WIDTH = 20;


enum WIDGET_FLAGS {
    FLAG_DISABLED = 1,
    FLAG_INVISIBLE = 2,
    FLAG_NOT_SELECTABLE = 4,
    FLAG_NO_CLIPPING_RECTANGLE = 8,
    //When checking the Widget Under Mouse, do not check this widget's children.
    FLAG_WUM_NO_CHILDREN = 16,
};

enum DRAW_LINE_SIDES {
    DRAW_LINE_RIGHT,
    DRAW_LINE_TOP,
    DRAW_LINE_LEFT,
    DRAW_LINE_BOTTOM,
};

enum EASY_FLAGS {
    EASY_FLAG_WIDTH_PX = 1,  //Width is in pixels, not percentage.
};

const float TAU = 3.14159265358979323846 * 2;

}

#endif //ifndef LAFI_CONST_INCLUDED
