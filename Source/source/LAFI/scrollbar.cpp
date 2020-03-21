#include <cfloat>

#include "scrollbar.h"

#include "button.h"

namespace lafi {

/* ----------------------------------------------------------------------------
 * Creates a scrollbar.
 */
scrollbar::scrollbar(
    const int x1, const int y1, const int x2, const int y2,
    const float min_value, const float max_value,
    const float low_value, const float high_value,
    const bool vertical, lafi::style* style,
    const unsigned char flags
) :
    widget(x1, y1, x2, y2, style, flags),
    attached_widget(NULL),
    min_value(min_value),
    max_value(max_value),
    low_value(low_value),
    high_value(high_value),
    vertical(vertical),
    change_handler(nullptr) {
    
    needs_init = true;
}


/* ----------------------------------------------------------------------------
 * Creates a scrollbar.
 */
scrollbar::scrollbar(
    const float min_value, const float max_value,
    const float low_value, const float high_value, const bool vertical
) :
    widget(),
    attached_widget(NULL),
    min_value(min_value),
    max_value(max_value),
    low_value(low_value),
    high_value(high_value),
    vertical(vertical),
    change_handler(nullptr) {
    
    needs_init = true;
}


/* ----------------------------------------------------------------------------
 * Initializes the scrollbar.
 */
void scrollbar::init() {
    create_button();
    flags |= FLAG_WUM_NO_CHILDREN;
}


/* ----------------------------------------------------------------------------
 * When the mouse is held down, the scrollbar button
 * is moved to that location.
 */
void scrollbar::widget_on_mouse_down(
    const int button, const int x, const int y
) {
    if(button != 1) return;
    
    move_button(x, y);
}


/* ----------------------------------------------------------------------------
 * When the mouse is moved, if the mouse button is held
 * down, the scrollbar button is moved with the mouse.
 */
void scrollbar::widget_on_mouse_move(const int x, const int y) {
    if(!mouse_clicking) return;
    
    move_button(x, y);
}


/* ----------------------------------------------------------------------------
 * (Re)creates the button used on the scrollbar.
 * If the values do not allow for there to be
 * a button, it is not created (or deleted, if
 * it already exists).
 */
void scrollbar::create_button() {
    remove("but_bar");
    
    if(low_value != high_value) {
    
        int bx1, by1, bx2, by2;
        
        if(vertical) {
            //Pixels per value.
            int bh =
                (high_value - low_value) *
                ((float) (y2 - y1) / (max_value - min_value));
                
            bx1 = x1; by1 = y1;
            bx2 = x2; by2 = y1 + bh;
        } else {
            //Pixels per value.
            int bw =
                (high_value - low_value) *
                ((float) (x2 - x1) / (max_value - min_value));
                
            by1 = y1; bx1 = x1;
            by2 = y2; bx2 = x1 + bw;
        }
        
        add("but_bar", new button(
                bx1,
                by1,
                bx2,
                by2,
                "",
                "",
                NULL,
                style
            ));
    }
}


/* ----------------------------------------------------------------------------
 * Moves the scrollbar's button to the desired coordinates.
 * The coordinates specify the center.
 */
void scrollbar::move_button(const int x, const int y) {
    if(low_value == high_value) return;
    
    button* but = (button*) widgets["but_bar"];
    
    if(vertical) {
        int bh = but->y2 - but->y1;
        int h = y2 - y1;
        
        but->y1 = y - (but->y2 - but->y1) / 2;
        if(but->y1 < y1) but->y1 = y1;
        if(but->y1 + bh > y2) but->y1 = y2 - bh;
        
        but->y2 = but->y1 + bh;
        
        low_value =
            min_value + ((but->y1 - y1) / (float) h) *
            (max_value - min_value);
        high_value =
            min_value + ((but->y2 - y1) / (float) h) *
            (max_value - min_value);
            
    } else {
        int bw = but->x2 - but->x1;
        int w = x2 - x1;
        
        but->x1 = x - (but->x2 - but->x1) / 2;
        if(but->x1 < x1) but->x1 = x1;
        if(but->x1 + bw > x2) but->x1 = x2 - bw;
        
        but->x2 = but->x1 + bw;
        
        low_value =
            min_value + ((but->x1 - x1) / (float) w) *
            (max_value - min_value);
        high_value =
            min_value + ((but->x2 - x1) / (float) w) *
            (max_value - min_value);
    }
    
    if(change_handler) change_handler(this);
}


/* ----------------------------------------------------------------------------
 * Sets the scrollbar's current value.
 */
void scrollbar::set_value(const float new_low, const bool call_handler) {
    float dif = high_value - low_value;
    if(new_low < min_value || new_low + dif > max_value) return;
    
    button* but = (button*) widgets["but_bar"];
    if(!but) return;
    
    low_value = new_low;
    high_value = new_low + dif;
    
    float ratio = (low_value - min_value) / (max_value - min_value - dif);
    
    if(vertical) {
        int but_h = but->y2 - but->y1;
        but->y1 = y1 + ratio * ((y2 - y1) - but_h);
        but->y2 = but->y1 + but_h;
    } else {
        int but_w = but->x2 - but->x1;
        but->x1 = x1 + ratio * ((x2 - x1) - but_w);
        but->x2 = but->x1 + but_w;
    }
    
    if(change_handler && call_handler) change_handler(this);
}


/* ----------------------------------------------------------------------------
 * Draws the scrollbar. The button can draw itself, but this widget
 * draws the harness.
 */
void scrollbar::draw_self() {
    int w = x2 - x1;
    int h = y2 - y1;
    
    if(vertical) {
        al_draw_filled_rectangle(
            x1 + w / 2 - 2,
            y1 + 0.5,
            x1 + w / 2 + 2,
            y2 - 0.5,
            get_bg_color()
        );
        al_draw_line(
            x1 + w / 2 - 0.5,
            y1 + 0.5,
            x1 + w / 2 - 0.5,
            y2 - 0.5,
            get_lighter_bg_color(),
            1
        );
        al_draw_line(
            x1 + w / 2 + 0.5,
            y1 + 0.5,
            x1 + w / 2 + 0.5,
            y2 - 0.5,
            get_darker_bg_color(),
            1
        );
        al_draw_line(
            x1 + w / 2 - 4, y1 + 0.5, x1 + w / 2 + 4, y1 + 0.5,
            get_lighter_bg_color(), 1
        );
        al_draw_line(
            x1 + w / 2 - 4, y1 + 1.5, x1 + w / 2 + 4, y1 + 1.5,
            get_darker_bg_color(), 1
        );
        al_draw_line(
            x1 + w / 2 - 4, y2 - 0.5, x1 + w / 2 + 4, y2 - 0.5,
            get_darker_bg_color(), 1
        );
        al_draw_line(
            x1 + w / 2 - 4, y2 - 1.5, x1 + w / 2 + 4, y2 - 1.5,
            get_lighter_bg_color(), 1
        );
        
    } else { //Horizontal bar.
    
        al_draw_filled_rectangle(
            x1 + 0.5,
            y1 + h / 2 - 2,
            x2 - 0.5,
            y1 + h / 2 + 2,
            get_bg_color()
        );
        al_draw_line(
            x1 + 0.5,
            y1 + h / 2 - 0.5,
            x2 - 0.5,
            y1 + h / 2 - 0.5,
            get_lighter_bg_color(),
            1
        );
        al_draw_line(
            x1 + 0.5,
            y1 + h / 2 + 0.5,
            x2 - 0.5,
            y1 + h / 2 + 0.5,
            get_darker_bg_color(),
            1
        );
        al_draw_line(
            x1 + 0.5, y1 + h / 2 - 4, x1 + 0.5, y1 + h / 2 + 4,
            get_lighter_bg_color(), 1
        );
        al_draw_line(
            x1 + 1.5, y1 + h / 2 - 4, x1 + 1.5, y1 + h / 2 + 4,
            get_darker_bg_color(), 1
        );
        al_draw_line(
            x2 - 0.5, y1 + h / 2 - 4, x2 - 0.5, y1 + h / 2 + 4,
            get_darker_bg_color(), 1
        );
        al_draw_line(
            x2 - 1.5, y1 + h / 2 - 4, x2 - 1.5, y1 + h / 2 + 4,
            get_lighter_bg_color(), 1
        );
        
    }
}


/* ----------------------------------------------------------------------------
 * Registers an external handler for when the scrollbar's
 * value is changed.
 */
void scrollbar::register_change_handler(void(*handler)(widget* w)) {
    change_handler = handler;
}


/* ----------------------------------------------------------------------------
 * Makes it so that this scrollbar is bound to a widget.
 * Whenever the scrollbar is changed, the widget scrolls
 * accordingly.
 */
void scrollbar::make_widget_scroll(widget* widget) {
    attached_widget = widget;
    this->min_value = this->low_value = 0;
    if(widget) {
        widget->children_offset_x = widget->children_offset_y = 0;
        float largest_y2 = FLT_MIN, largest_x2 = FLT_MIN;
        
        for(auto &w : widget->widgets) {
            if(!w.second) continue;
            
            if(vertical) {
                if(w.second->y2 > largest_y2) largest_y2 = w.second->y2;
            } else {
                if(w.second->x2 > largest_x2) largest_x2 = w.second->x2;
            }
        }
        
        if(vertical) {
            largest_y2 += 8; //Spacing.
            largest_y2 -= widget->y1;
            if(largest_y2 < widget->y2 - widget->y1) {
                this->high_value = this->max_value = 0;
            } else {
                this->high_value = widget->y2 - widget->y1;
                this->max_value = largest_y2;
            }
        } else {
            largest_x2 += 8; //Spacing.
            largest_x2 -= widget->x1;
            if(largest_x2 < widget->x2 - widget->x1) {
                this->high_value = this->max_value = 0;
            } else {
                this->high_value = widget->x2 - widget->x1;
                this->max_value = largest_x2;
            }
        }
        
        register_change_handler(widget_scroller);
        
    } else {
    
        this->max_value = 10;
        this->high_value = 1;
        
        register_change_handler(NULL);
    }
    
    create_button();
}


/* ----------------------------------------------------------------------------
 * Callback for when a scrollbar is meant to scroll the contents of a widget.
 */
void scrollbar::widget_scroller(widget* w) {
    scrollbar* scrollbar_ptr = (scrollbar*) w;
    
    if(scrollbar_ptr->vertical) {
        scrollbar_ptr->attached_widget->children_offset_y =
            -scrollbar_ptr->low_value;
    } else {
        scrollbar_ptr->attached_widget->children_offset_x =
            -scrollbar_ptr->low_value;
    }
}


/* ----------------------------------------------------------------------------
 * Destroys a scrollbar.
 */
scrollbar::~scrollbar() { }

}
