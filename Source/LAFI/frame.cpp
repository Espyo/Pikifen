#include "frame.h"

namespace lafi {

/*
 * Creates a frame given some parameters.
 */
frame::frame(int x1, int y1, int x2, int y2, lafi::style* style, unsigned char flags)
    : widget(x1, y1, x2, y2, style, flags) {
    
}


/*
 * Destroys a frame.
 */
frame::~frame() {}


void frame::draw_self() {
    al_draw_filled_rectangle(x1, y1, x2, y2, get_bg_color());
    draw_line(this, DRAW_LINE_TOP,    0, 1, 0, get_lighter_bg_color()); //Top line,    outermost.
    draw_line(this, DRAW_LINE_TOP,    1, 2, 1, get_darker_bg_color());  //Top line,    innermost.
    draw_line(this, DRAW_LINE_LEFT,   0, 1, 0, get_lighter_bg_color()); //Left line,   outermost.
    draw_line(this, DRAW_LINE_LEFT,   1, 2, 1, get_darker_bg_color());  //Left line,   innermost.
    draw_line(this, DRAW_LINE_BOTTOM, 1, 0, 0, get_darker_bg_color());  //Bottom line, outermost.
    draw_line(this, DRAW_LINE_BOTTOM, 2, 1, 1, get_lighter_bg_color()); //Bottom line, innermost.
    draw_line(this, DRAW_LINE_RIGHT,  1, 0, 0, get_darker_bg_color());  //Right line,  outermost.
    draw_line(this, DRAW_LINE_RIGHT,  2, 1, 1, get_lighter_bg_color()); //Right line,  innermost.
}

}
