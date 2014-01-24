#ifndef LAFI_RADIO_BUTTON_BUTTON_INCLUDED
#define LAFI_RADIO_BUTTON_BUTTON_INCLUDED

#include "widget.h"

class lafi_radio_button_button : public lafi_widget {
public:
    ALLEGRO_BITMAP* unselected_bitmap;
    ALLEGRO_BITMAP* selected_bitmap;
    
    bool selected;
    
    lafi_radio_button_button(int x1 = 0, int y1 = 0, bool selected = false, lafi_style* style = NULL, unsigned char flags = 0);
    ~lafi_radio_button_button();
    
    void render();
    void draw_self();
};

#endif //ifndef LAFI_RADIO_BUTTON_BUTTON_INCLUDED