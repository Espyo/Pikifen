#ifndef LAFI_SCROLLBAR_INCLUDED
#define LAFI_SCROLLBAR_INCLUDED

#include "widget.h"

class lafi_scrollbar : public lafi_widget {
private:
    ALLEGRO_BITMAP* normal_bitmap;
    void move_button(int x, int y);
    void create_button();
    
public:
    lafi_widget* attached_widget;
    float min, max;
    float low_value, high_value;
    bool vertical;
    
    lafi_scrollbar(int x1 = 0, int y1 = 0, int x2 = 1, int y2 = 1, float min = 0, float max = 10, float low_value = 0, float high_value = 1, bool vertical = true, lafi_style* style = NULL, unsigned char flags = 0);
    ~lafi_scrollbar();
    
    void init();
    void render();
    void draw_self();
    
    void (*change_handler)(lafi_widget* w);
    
    void widget_on_mouse_down(int button, int x, int y);
    void widget_on_mouse_move(int x, int y);
    
    void register_change_handler(void(*handler)(lafi_widget* w));
    void make_widget_scroll(lafi_widget* widget);
    static void widget_scroller(lafi_widget* w);
};

#endif //ifndef LAFI_SCROLLBAR_INCLUDED