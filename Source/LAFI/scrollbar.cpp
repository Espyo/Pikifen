#include <cfloat>

#include "button.h"
#include "scrollbar.h"

lafi_scrollbar::lafi_scrollbar(int x1, int y1, int x2, int y2, float min, float max, float low_value, float high_value, bool vertical, lafi_style* style, unsigned char flags)
    : lafi_widget(x1, y1, x2, y2, style, flags) {
    
    this->min = min;
    this->max = max;
    this->low_value = low_value;
    this->high_value = high_value;
    this->vertical = vertical;
    needs_init = true;
    change_handler = NULL;
    
    normal_bitmap = NULL;
}

void lafi_scrollbar::init() {
    create_button();
}

void lafi_scrollbar::widget_on_mouse_down(int button, int x, int y) {
    if(button != 1) return;
    
    move_button(x, y);
    if(change_handler) change_handler(this);
}

void lafi_scrollbar::widget_on_mouse_move(int x, int y) {
    if(!mouse_clicking) return;
    
    move_button(x, y);
    if(change_handler) change_handler(this);
}

void lafi_scrollbar::create_button() {
    int bx1, by1, bx2, by2;
    
    remove("btn_bar");
    
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
        
        add("btn_bar", new lafi_button(
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

void lafi_scrollbar::move_button(int x, int y) {
    if(low_value == high_value) return;
    
    lafi_button* but = (lafi_button*) widgets["btn_bar"];
    
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
}

void lafi_scrollbar::render() {
    if(normal_bitmap) al_destroy_bitmap(normal_bitmap);
    
    int w = x2 - x1;
    int h = y2 - y1;
    
    normal_bitmap = al_create_bitmap(w, h);
    ALLEGRO_BITMAP* old_target_bitmap = al_get_target_bitmap();
    al_set_target_bitmap(normal_bitmap); {
    
        if(vertical) {
        
            al_draw_filled_rectangle(
                w / 2 - 2,
                0,
                w / 2 + 2,
                h,
                style->bg_color
            );
            al_draw_line(
                w / 2 - 0.5,
                0,
                w / 2 - 0.5,
                h,
                style->lighter_bg_color,
                1
            );
            al_draw_line(
                w / 2 + 0.5,
                0,
                w / 2 + 0.5,
                h,
                style->darker_bg_color,
                1
            );
            al_draw_line(w / 2 - 4, 0.5, w / 2 + 4, 0.5, style->lighter_bg_color, 1);
            al_draw_line(w / 2 - 4, 1.5, w / 2 + 4, 1.5, style->darker_bg_color, 1);
            al_draw_line(w / 2 - 4, h - 0.5, w / 2 + 4, h - 0.5, style->darker_bg_color, 1);
            al_draw_line(w / 2 - 4, h - 1.5, w / 2 + 4, h - 1.5, style->lighter_bg_color, 1);
            
        } else { //Horizontal bar.
        
            al_draw_filled_rectangle(
                0,
                h / 2 - 2,
                w,
                h / 2 + 2,
                style->bg_color
            );
            al_draw_line(
                0,
                h / 2 - 0.5,
                w,
                h / 2 - 0.5,
                style->lighter_bg_color,
                1
            );
            al_draw_line(
                0,
                h / 2 + 0.5,
                w,
                h / 2 + 0.5,
                style->darker_bg_color,
                1
            );
            al_draw_line(0.5, h / 2 - 4, 0.5, h / 2 + 4, style->lighter_bg_color, 1);
            al_draw_line(1.5, h / 2 - 4, 1.5, h / 2 + 4, style->darker_bg_color, 1);
            al_draw_line(w - 0.5, h / 2 - 4, w - 0.5, h / 2 + 4, style->darker_bg_color, 1);
            al_draw_line(w - 1.5, h / 2 - 4, w - 1.5, h / 2 + 4, style->lighter_bg_color, 1);
            
        }
        
    } al_set_target_bitmap(old_target_bitmap);
}

void lafi_scrollbar::draw_self() {
    al_draw_bitmap(normal_bitmap, x1, y1, 0);
}

void lafi_scrollbar::register_change_handler(void(*handler)(lafi_widget* w)) {
    change_handler = handler;
}

void lafi_scrollbar::make_widget_scroll(lafi_widget* widget) {
    attached_widget = widget;
    this->min = this->low_value = 0;
    if(widget) {
        float largest_y2 = -FLT_MIN, largest_x2 = FLT_MIN;
        
        for(auto w = widget->widgets.begin(); w != widget->widgets.end(); w++) {
            if(vertical) {
                if(w->second->y2 > largest_y2) largest_y2 = w->second->y2;
            } else {
                if(w->second->x2 > largest_x2) largest_x2 = w->second->x2;
            }
        }
        
        if(vertical) {
            largest_y2 += 8; //Spacing.
            if(largest_y2 < widget->y2 - widget->y1) {
                this->high_value = this->max = 0;
            } else {
                this->high_value = widget->y2 - widget->y1;
                this->max = largest_y2 - widget->y1;
            }
        } else {
            largest_x2 += 8; //Spacing.
            if(largest_x2 < widget->x2 - widget->x1) {
                this->high_value = this->max = 0;
            } else {
                this->high_value = widget->x2 - widget->x1;
                this->max = largest_x2 - widget->x1;
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

void lafi_scrollbar::widget_scroller(lafi_widget* w) {
    lafi_scrollbar* scrollbar_ptr = (lafi_scrollbar*) w;
    
    if(scrollbar_ptr->vertical) {
        scrollbar_ptr->attached_widget->children_offset_y = -scrollbar_ptr->low_value;
    } else {
        scrollbar_ptr->attached_widget->children_offset_x = -scrollbar_ptr->low_value;
    }
}

lafi_scrollbar::~lafi_scrollbar() { }
