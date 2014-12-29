#ifndef LAFI_CHECKBOX_INCLUDED
#define LAFI_CHECKBOX_INCLUDED

#include "widget.h"

namespace lafi {

class checkbox : public widget {
public:
    bool checked;
    string text;
    
    void check();
    void uncheck();
    void set(bool value);
    
    void init();
    
    checkbox(int x1 = 0, int y1 = 0, int x2 = 1, int y2 = 1, string text = "", bool checked = false, lafi::style* style = NULL, unsigned char flags = 0);
    ~checkbox();
    void widget_on_left_mouse_click(int x, int y);
    
    void draw_self();
};



class checkbox_box : public widget {
private:

public:
    bool checked;
    
    checkbox_box(int x1 = 0, int y1 = 0, bool checked = false, lafi::style* style = NULL, unsigned char flags = 0);
    ~checkbox_box();
    
    void draw_self();
};

}

#endif //ifndef LAFI_CHECKBOX_INCLUDED
