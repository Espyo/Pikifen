#include "minor.h"

lafi_line::lafi_line(int x1, int y1, int x2, int y2, bool horizontal, int thickness, lafi_style* style, unsigned char flags)
    : lafi_widget(x1, y1, x2, y2, style, flags) {
    
    this->horizontal = horizontal;
    this->thickness = thickness;
}

lafi_line::~lafi_line() { }

void lafi_line::draw_self() {
    if(horizontal) {
        int y = (y1 + y2) / 2;
        al_draw_line(x1, y, x2, y, get_fg_color(), thickness);
    } else {
        int x = (x1 + x2) / 2;
        al_draw_line(x, y1, x, y2, get_fg_color(), thickness);
    }
}





lafi_dummy::lafi_dummy(int x1, int y1, int x2, int y2, lafi_style* style, unsigned char flags)
    : lafi_widget(x1, y1, x2, y2, style, flags) {
}

void lafi_dummy::draw_self() { }