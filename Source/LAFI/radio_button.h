#ifndef LAFI_RADIO_BUTTON_INCLUDED
#define LAFI_RADIO_BUTTON_INCLUDED

#include "widget.h"

namespace lafi {

class radio_button : public widget {
public:
    bool selected;
    string text;
    int group;
    
    void init();
    
    radio_button(int x1 = 0, int y1 = 0, int x2 = 1, int y2 = 1, string text = "", int group = 0, bool selected = false, lafi::style* style = NULL, unsigned char flags = 0);
    ~radio_button();
    void widget_on_left_mouse_click(int x, int y);
    
    void select();
    void unselect();
    
    void draw_self();
};

class radio_button_button : public widget {
public:
    bool selected;
    
    radio_button_button(int x1 = 0, int y1 = 0, bool selected = false, lafi::style* style = NULL, unsigned char flags = 0);
    ~radio_button_button();
    
    void draw_self();
};

}

#endif //ifndef LAFI_RADIO_BUTTON_INCLUDED