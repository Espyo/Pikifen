#include "frame.h"

/*
 * Creates a frame given some parameters.
 */
lafi_frame::lafi_frame(int x1, int y1, int x2, int y2, lafi_style* style, unsigned char flags)
    : lafi_widget(x1, y1, x2, y2, style, flags) {
    
}

/*
 * Destroys a frame.
 */
lafi_frame::~lafi_frame() {}

void lafi_frame::draw_self() {
    al_draw_filled_rectangle(x1, y1, x2, y2, get_bg_color());
    lafi_draw_line(this, LAFI_DRAW_LINE_TOP,    0, 1, 0, get_lighter_bg_color()); //Top line,    outermost.
    lafi_draw_line(this, LAFI_DRAW_LINE_TOP,    1, 2, 1, get_darker_bg_color());  //Top line,    innermost.
    lafi_draw_line(this, LAFI_DRAW_LINE_LEFT,   0, 1, 0, get_lighter_bg_color()); //Left line,   outermost.
    lafi_draw_line(this, LAFI_DRAW_LINE_LEFT,   1, 2, 1, get_darker_bg_color());  //Left line,   innermost.
    lafi_draw_line(this, LAFI_DRAW_LINE_BOTTOM, 1, 0, 0, get_darker_bg_color());  //Bottom line, outermost.
    lafi_draw_line(this, LAFI_DRAW_LINE_BOTTOM, 2, 1, 1, get_lighter_bg_color()); //Bottom line, innermost.
    lafi_draw_line(this, LAFI_DRAW_LINE_RIGHT,  1, 0, 0, get_darker_bg_color());  //Right line,  outermost.
    lafi_draw_line(this, LAFI_DRAW_LINE_RIGHT,  2, 1, 1, get_lighter_bg_color()); //Right line,  innermost.
}