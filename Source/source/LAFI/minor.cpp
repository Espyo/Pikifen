#include "minor.h"

namespace lafi {

/* ----------------------------------------------------------------------------
 * Creates a line.
 */
line::line(
    const int x1, const int y1, const int x2, const int y2,
    const bool horizontal, const int thickness,
    lafi::style* style, const unsigned char flags
) :
    widget(x1, y1, x2, y2, style, flags),
    horizontal(horizontal),
    thickness(thickness) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a line.
 */
line::line(const bool horizontal, const int thickness) :
    widget(),
    horizontal(horizontal),
    thickness(thickness) {
    
}


/* ----------------------------------------------------------------------------
 * Destroys a line.
 */
line::~line() { }


/* ----------------------------------------------------------------------------
 * Draws the line.
 */
void line::draw_self() {
    if(horizontal) {
        int y = (y1 + y2) / 2;
        al_draw_line(x1, y, x2, y, get_fg_color(), thickness);
    } else {
        int x = (x1 + x2) / 2;
        al_draw_line(x, y1, x, y2, get_fg_color(), thickness);
    }
}





/* ----------------------------------------------------------------------------
 * Creates a dummy widget.
 */
dummy::dummy(
    const int x1, const int y1, const int x2, const int y2,
    lafi::style* style, const unsigned char flags
) :
    widget(x1, y1, x2, y2, style, flags) {
}


/* ----------------------------------------------------------------------------
 * Creates a dummy widget.
 */
dummy::dummy() : widget() {

}


/* ----------------------------------------------------------------------------
 * Destroys a dummy widget.
 */
void dummy::draw_self() { }

}
