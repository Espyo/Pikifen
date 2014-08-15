#ifndef LAFI_CONST_INCLUDED
#define LAFI_CONST_INCLUDED

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

namespace lafi {

#define COLOR_SHIFT_DELTA 0.2 //A color is lightened/darkened by this much.

#define CHECKBOX_BOX_SIZE 12
#define CHECKBOX_BOX_PADDING 4 //How many pixels after the box should the label come.
#define RADIO_BUTTON_BUTTON_SIZE 12
#define RADIO_BUTTON_BUTTON_PADDING 4

#define DEF_STYLE_BG_R 192
#define DEF_STYLE_BG_G 192
#define DEF_STYLE_BG_B 192
#define DEF_STYLE_FG_R 0
#define DEF_STYLE_FG_G 0
#define DEF_STYLE_FG_B 0
#define DEF_STYLE_ALT_R 96
#define DEF_STYLE_ALT_G 96
#define DEF_STYLE_ALT_B 192

#define STANDARD_SCROLLBAR_WIDTH 20

enum WIDGET_FLAGS {
    FLAG_DISABLED = 1,
    FLAG_INVISIBLE = 2,
    FLAG_NOT_SELECTABLE = 4,
    FLAG_NO_CLIPPING_RECTANGLE = 8,
    FLAG_WUM_NO_CHILDREN = 16, //When checking the Widget Under Mouse, do not check this widget's children.
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

}

#endif //ifndef LAFI_CONST_INCLUDED