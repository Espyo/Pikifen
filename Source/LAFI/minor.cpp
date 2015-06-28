#include "minor.h"

namespace lafi {

/* ----------------------------------------------------------------------------
 * Creates a line.
 */
line::line(int x1, int y1, int x2, int y2, bool horizontal, int thickness, lafi::style* style, unsigned char flags) :
    widget(x1, y1, x2, y2, style, flags),
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
dummy::dummy(int x1, int y1, int x2, int y2, lafi::style* style, unsigned char flags) :
    widget(x1, y1, x2, y2, style, flags) {
}


/* ----------------------------------------------------------------------------
 * Destroys a dummy widget.
 */
void dummy::draw_self() { }

}
