#ifndef LAFI_FRAME_INCLUDED
#define LAFI_FRAME_INCLUDED

#include <string>

#include "widget.h"

using namespace std;

class lafi_frame : public lafi_widget {
public:
    ALLEGRO_BITMAP* normal_bitmap;
    
    lafi_frame(int x1 = 0, int y1 = 0, int x2 = 1, int y2 = 1, lafi_style* style = NULL, unsigned char flags = 0);
    lafi_frame(lafi_frame &f2);
    ~lafi_frame();
    
    void draw_self();
};

#endif //ifndef LAFI_FRAME_INCLUDED