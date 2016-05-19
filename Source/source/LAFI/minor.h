#ifndef LAFI_LINE_INCLUDED
#define LAFI_LINE_INCLUDED

#include "widget.h"

namespace lafi {

/* ----------------------------------------------------------------------------
 * A simple straight line widget, vertical or horizontal.
 */
class line : public widget {
public:
    bool horizontal;
    int thickness;

    line(
        int x1 = 0, int y1 = 0, int x2 = 1, int y2 = 1,
        bool horizontal = true, int thickness = 1,
        lafi::style* style = NULL, unsigned char flags = 0
    );
    ~line();

    void draw_self();
};



/* ----------------------------------------------------------------------------
 * A dummy widget, mostly used for spacing.
 */
class dummy : public widget {
public:
    dummy(
        int x1 = 0, int y1 = 0, int x2 = 1, int y2 = 1,
        lafi::style* style = NULL, unsigned char flags = 0
    );
    ~dummy();

    void draw_self();
};

}

#endif //ifndef LAFI_LINE_INCLUDED
