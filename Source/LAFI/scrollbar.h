#ifndef LAFI_SCROLLBAR_INCLUDED
#define LAFI_SCROLLBAR_INCLUDED

#include "widget.h"

namespace lafi {

class scrollbar : public widget {
public:
    widget* attached_widget;
    float min_value, max_value;
    float low_value, high_value;
    bool vertical;
    
    scrollbar(int x1 = 0, int y1 = 0, int x2 = 1, int y2 = 1, float min_value = 0, float max_value = 10, float low_value = 0, float high_value = 1, bool vertical = true, lafi::style* style = NULL, unsigned char flags = 0);
    ~scrollbar();
    
    void init();
    void draw_self();
    
    function<void(widget* w)> change_handler;
    
    void widget_on_mouse_down(int button, int x, int y);
    void widget_on_mouse_move(int x, int y);
    
    void register_change_handler(void(*handler)(widget* w));
    void make_widget_scroll(widget* widget);
    static void widget_scroller(widget* w);
    void move_button(int x, int y);
    void set_value(float new_low);
    void create_button();
};

}

#endif //ifndef LAFI_SCROLLBAR_INCLUDED