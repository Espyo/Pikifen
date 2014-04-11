#ifndef LAFI_LINE_INCLUDED
#define LAFI_LINE_INCLUDED

#include "widget.h"

class lafi_line : public lafi_widget {
public:
    bool horizontal;
    int thickness;
    
    lafi_line(int x1 = 0, int y1 = 0, int x2 = 1, int y2 = 1, bool horizontal = true, int thickness = 1, lafi_style* style = NULL, unsigned char flags = 0);
    ~lafi_line();
    
    void draw_self();
};

//A dummy widget, mostly used for spacing.
class lafi_dummy : public lafi_widget {
public:
    lafi_dummy(int x1 = 0, int y1 = 0, int x2 = 1, int y2 = 1, lafi_style* style = NULL, unsigned char flags = 0);
    ~lafi_dummy();
    
    void draw_self();
};

#endif //ifndef LAFI_LINE_INCLUDED