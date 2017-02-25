#include "frame.h"

namespace lafi {

/* ----------------------------------------------------------------------------
 * Creates a frame.
 */
frame::frame(
    const int x1, const int y1, const int x2, const int y2, lafi::style* style,
    const unsigned char flags
) :
    widget(x1, y1, x2, y2, style, flags) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a frame.
 */
frame::frame() : widget() {

}


/* ----------------------------------------------------------------------------
 * Destroys a frame.
 */
frame::~frame() {}


/* ----------------------------------------------------------------------------
 * Draws the actual frame. Like the frame of a painting, this
 * is basically a rectangle and a border.
 */
void frame::draw_self() {
    al_draw_filled_rectangle(x1, y1, x2, y2, get_bg_color());
    //Top line, outermost.
    draw_line(this, DRAW_LINE_TOP,    0, 1, 0, get_lighter_bg_color());
    //Top line, innermost.
    draw_line(this, DRAW_LINE_TOP,    1, 2, 1, get_darker_bg_color());
    //Left line, outermost.
    draw_line(this, DRAW_LINE_LEFT,   0, 1, 0, get_lighter_bg_color());
    //Left line, innermost.
    draw_line(this, DRAW_LINE_LEFT,   1, 2, 1, get_darker_bg_color());
    //Bottom line, outermost.
    draw_line(this, DRAW_LINE_BOTTOM, 1, 0, 0, get_darker_bg_color());
    //Bottom line, innermost.
    draw_line(this, DRAW_LINE_BOTTOM, 2, 1, 1, get_lighter_bg_color());
    //Right line, outermost.
    draw_line(this, DRAW_LINE_RIGHT,  1, 0, 0, get_darker_bg_color());
    //Right line, innermost.
    draw_line(this, DRAW_LINE_RIGHT,  2, 1, 1, get_lighter_bg_color());
}

}
