#ifndef LAFI_SCROLLBAR_INCLUDED
#define LAFI_SCROLLBAR_INCLUDED

#include "widget.h"

namespace lafi {

/* ----------------------------------------------------------------------------
 * A button widget that can be dragged up and down (or left and right)
 * to pan a document, amongst other things.
 */
class scrollbar : public widget {
public:
    widget* attached_widget;
    float min_value, max_value;
    float low_value, high_value;
    bool vertical;
    
    scrollbar(
        const int x1, const int y1, const int x2, const int y2,
        const float min_value = 0, const float max_value = 10,
        const float low_value = 0, const float high_value = 1,
        const bool vertical = true, lafi::style* style = NULL,
        const unsigned char flags = 0
    );
    scrollbar(
        const float min_value = 0, const float max_value = 10,
        const float low_value = 0, const float high_value = 1,
        const bool vertical = true
    );
    
    void init();
    void draw_self();
    
    function<void(widget* w)> change_handler;
    
    void widget_on_mouse_down(const int button, const int x, const int y);
    void widget_on_mouse_move(const int x, const int y);
    
    void register_change_handler(void(*handler)(widget* w));
    void make_widget_scroll(widget* widget);
    static void widget_scroller(widget* w);
    void move_button(const int x, const int y);
    void set_value(const float new_low, const bool call_handler);
    void create_button();
};

}

#endif //ifndef LAFI_SCROLLBAR_INCLUDED
