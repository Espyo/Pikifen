#include <cfloat>

#include "button.h"
#include "scrollbar.h"

namespace lafi {

scrollbar::scrollbar(int x1, int y1, int x2, int y2, float min, float max, float low_value, float high_value, bool vertical, lafi::style* style, unsigned char flags)
    : widget(x1, y1, x2, y2, style, flags) {
    
    attached_widget = NULL;
    this->min = min;
    this->max = max;
    this->low_value = low_value;
    this->high_value = high_value;
    this->vertical = vertical;
    needs_init = true;
    change_handler = NULL;
}

void scrollbar::init() {
    create_button();
    flags |= FLAG_WUM_NO_CHILDREN;
}

void scrollbar::widget_on_mouse_down(int button, int x, int y) {
    if(button != 1) return;
    
    move_button(x, y);
}

void scrollbar::widget_on_mouse_move(int x, int y) {
    if(!mouse_clicking) return;
    
    move_button(x, y);
}

void scrollbar::create_button() {
    int bx1, by1, bx2, by2;
    
    remove("but_bar");
    
    if(low_value != high_value) {
    
        if(vertical) {
            int bh = (high_value - low_value) * ((float) (y2 - y1) / (max - min));  //Pixels per value.
            
            bx1 = x1; by1 = y1;
            bx2 = x2; by2 = y1 + bh;
        } else {
            int bw = (high_value - low_value) * ((float) (x2 - x1) / (max - min));  //Pixels per value.
            
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

void scrollbar::move_button(int x, int y) {
    if(low_value == high_value) return;
    
    button* but = (button*) widgets["but_bar"];
    
    if(vertical) {
        int bh = but->y2 - but->y1;
        int h = y2 - y1;
        
        but->y1 = y - (but->y2 - but->y1) / 2;
        if(but->y1 < y1) but->y1 = y1;
        if(but->y1 + bh > y2) but->y1 = y2 - bh;
        
        but->y2 = but->y1 + bh;
        
        low_value = min + ((but->y1 - y1) / (float) h) * (max - min);
        high_value = min + ((but->y2 - y1) / (float) h) * (max - min);
    } else {
        int bw = but->x2 - but->x1;
        int w = x2 - x1;
        
        but->x1 = x - (but->x2 - but->x1) / 2;
        if(but->x1 < x1) but->x1 = x1;
        if(but->x1 + bw > x2) but->x1 = x2 - bw;
        
        but->x2 = but->x1 + bw;
        
        low_value = min + ((but->x1 - x1) / (float) w) * (max - min);
        high_value = min + ((but->x2 - x1) / (float) w) * (max - min);
    }
    
    if(change_handler) change_handler(this);
}

void scrollbar::set_value(float new_low) {
    float dif = high_value - low_value;
    if(new_low < min || new_low + dif > max) return;
    
    button* but = (button*) widgets["but_bar"];
    if(!but) return;
    
    low_value = new_low;
    high_value = new_low + dif;
    
    float ratio = (low_value - min) / (max - min - dif);
    
    if(vertical) {
        int but_h = but->y2 - but->y1;
        but->y1 = y1 + ratio * ((y2 - y1) - but_h);
        but->y2 = but->y1 + but_h;
    } else {
        int but_w = but->x2 - but->x1;
        but->x1 = x1 + ratio * ((x2 - x1) - but_w);
        but->x2 = but->x1 + but_w;
    }
    
    if(change_handler) change_handler(this);
}

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
        al_draw_line(x1 + w / 2 - 4, y1 + 0.5, x1 + w / 2 + 4, y1 + 0.5, get_lighter_bg_color(), 1);
        al_draw_line(x1 + w / 2 - 4, y1 + 1.5, x1 + w / 2 + 4, y1 + 1.5, get_darker_bg_color(),  1);
        al_draw_line(x1 + w / 2 - 4, y2 - 0.5, x1 + w / 2 + 4, y2 - 0.5, get_darker_bg_color(),  1);
        al_draw_line(x1 + w / 2 - 4, y2 - 1.5, x1 + w / 2 + 4, y2 - 1.5, get_lighter_bg_color(), 1);
        
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
        al_draw_line(x1 + 0.5, y1 + h / 2 - 4, x1 + 0.5, y1 + h / 2 + 4, get_lighter_bg_color(), 1);
        al_draw_line(x1 + 1.5, y1 + h / 2 - 4, x1 + 1.5, y1 + h / 2 + 4, get_darker_bg_color(),  1);
        al_draw_line(x2 - 0.5, y1 + h / 2 - 4, x2 - 0.5, y1 + h / 2 + 4, get_darker_bg_color(),  1);
        al_draw_line(x2 - 1.5, y1 + h / 2 - 4, x2 - 1.5, y1 + h / 2 + 4, get_lighter_bg_color(), 1);
        
    }
}

void scrollbar::register_change_handler(void(*handler)(widget* w)) {
    change_handler = handler;
}

void scrollbar::make_widget_scroll(widget* widget) {
    attached_widget = widget;
    this->min = this->low_value = 0;
    if(widget) {
        widget->children_offset_x = widget->children_offset_y = 0;
        float largest_y2 = FLT_MIN, largest_x2 = FLT_MIN;
        
        for(auto w = widget->widgets.begin(); w != widget->widgets.end(); w++) {
            if(!w->second) continue;
            
            if(vertical) {
                if(w->second->y2 > largest_y2) largest_y2 = w->second->y2;
            } else {
                if(w->second->x2 > largest_x2) largest_x2 = w->second->x2;
            }
        }
        
        if(vertical) {
            largest_y2 += 8; //Spacing.
            largest_y2 -= widget->y1;
            if(largest_y2 < widget->y2 - widget->y1) {
                this->high_value = this->max = 0;
            } else {
                this->high_value = widget->y2 - widget->y1;
                this->max = largest_y2;
            }
        } else {
            largest_x2 += 8; //Spacing.
            largest_x2 -= widget->x1;
            if(largest_x2 < widget->x2 - widget->x1) {
                this->high_value = this->max = 0;
            } else {
                this->high_value = widget->x2 - widget->x1;
                this->max = largest_x2;
            }
        }
        
        register_change_handler(widget_scroller);
        
    } else {
    
        this->max = 10;
        this->high_value = 1;
        
        register_change_handler(NULL);
    }
    
    create_button();
}

void scrollbar::widget_scroller(widget* w) {
    scrollbar* scrollbar_ptr = (scrollbar*) w;
    
    if(scrollbar_ptr->vertical) {
        scrollbar_ptr->attached_widget->children_offset_y = -scrollbar_ptr->low_value;
    } else {
        scrollbar_ptr->attached_widget->children_offset_x = -scrollbar_ptr->low_value;
    }
}

scrollbar::~scrollbar() { }

}