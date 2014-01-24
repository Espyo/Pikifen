#ifndef LAFI_CHECKBOX_BOX_INCLUDED
#define LAFI_CHECKBOX_BOX_INCLUDED

#include "widget.h"

class lafi_checkbox_box : public lafi_widget {
private:

public:
    ALLEGRO_BITMAP* unchecked_bitmap;
    ALLEGRO_BITMAP* checked_bitmap;
    
    bool checked;
    
    lafi_checkbox_box(int x1 = 0, int y1 = 0, bool checked = false, lafi_style* style = NULL, unsigned char flags = 0);
    ~lafi_checkbox_box();
    
    void render();
    void draw_self();
};

#endif //ifndef LAFI_CHECKBOX_BOX_INCLUDED