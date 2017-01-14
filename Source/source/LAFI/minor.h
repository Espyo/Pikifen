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
        const int x1, const int y1, const int x2, const int y2,
        const bool horizontal = true, const int thickness = 1,
        lafi::style* style = NULL, const unsigned char flags = 0
    );
    line(const bool horizontal = true, const int thickness = 1);
    ~line();
    
    void draw_self();
};



/* ----------------------------------------------------------------------------
 * A dummy widget, mostly used for spacing.
 */
class dummy : public widget {
public:
    dummy(
        const int x1, const int y1, const int x2, const int y2,
        lafi::style* style = NULL, const unsigned char flags = 0
    );
    dummy();
    ~dummy();
    
    void draw_self();
};

}

#endif //ifndef LAFI_LINE_INCLUDED
