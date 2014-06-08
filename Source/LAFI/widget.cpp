#include "widget.h"

/*
 * Creates a widget given some parameters.
 */
lafi_widget::lafi_widget(int x1, int y1, int x2, int y2, lafi_style* style, unsigned char flags) {
    this->x1 = x1;
    this->y1 = y1;
    this->x2 = x2;
    this->y2 = y2;
    this->flags = flags;
    this->style = style;
    
    easy_reset();
    
    children_offset_x = children_offset_y = 0;
    focused_widget = NULL;
    parent = NULL;
    mouse_in = false;
    mouse_clicking = false;
    
    mouse_move_handler = NULL;
    left_mouse_click_handler = NULL;
    mouse_down_handler = NULL;
    mouse_up_handler = NULL;
    mouse_wheel_handler = NULL;
    mouse_enter_handler = NULL;
    mouse_leave_handler = NULL;
    get_focus_handler = NULL;
    lose_focus_handler = NULL;
    
    needs_init = false;
}

/*
 * Creates a widget by copying the info from another widget.
 */
lafi_widget::lafi_widget(lafi_widget &w2) {
    x1 = w2.x1;
    x2 = w2.x2;
    y1 = w2.y1;
    y2 = w2.y2;
    flags = w2.flags;
    style = w2.style;
    
    easy_reset();
    
    children_offset_x = w2.children_offset_x;
    children_offset_y = w2.children_offset_y;
    focused_widget = w2.focused_widget;
    parent = NULL;
    mouse_in = false;
    mouse_clicking = false;
    
    mouse_move_handler = NULL;
    left_mouse_click_handler = NULL;
    mouse_down_handler = NULL;
    mouse_up_handler = NULL;
    mouse_wheel_handler = NULL;
    mouse_enter_handler = NULL;
    mouse_leave_handler = NULL;
    get_focus_handler = NULL;
    lose_focus_handler = NULL;
    
    needs_init = false;
}

/*
 * Destroys a widget.
 */
lafi_widget::~lafi_widget() {
}

//Calls the function that handles a mouse move.
void lafi_widget::call_mouse_move_handler(int x, int y) { if(mouse_move_handler) mouse_move_handler(this, x, y); }

//Calls the function that handles a left mouse click.
void lafi_widget::call_left_mouse_click_handler(int x, int y) { if(left_mouse_click_handler) left_mouse_click_handler(this, x, y); }

//Calls the function that handles a mouse button down.
void lafi_widget::call_mouse_down_handler(int button, int x, int y) { if(mouse_down_handler) mouse_down_handler(this, button, x, y); }

//Calls the function that handles a mouse button up.
void lafi_widget::call_mouse_up_handler(int button, int x, int y) { if(mouse_up_handler) mouse_up_handler(this, button, x, y); }

//Calls the function that handles a mouse wheel move.
void lafi_widget::call_mouse_wheel_handler(int dy, int dx) { if(mouse_wheel_handler) mouse_wheel_handler(this, dy, dx); }

//Calls the function that handles the mouse entering.
void lafi_widget::call_mouse_enter_handler() { if(mouse_enter_handler) mouse_enter_handler(this); }

//Calls the function that handles the mouse leaving.
void lafi_widget::call_mouse_leave_handler() { if(mouse_leave_handler) mouse_leave_handler(this); }

//Calls the function that handles the focus being obtained.
void lafi_widget::call_get_focus_handler() { if(get_focus_handler) get_focus_handler(this); }

//Calls the function that handles the focus being lost.
void lafi_widget::call_lose_focus_handler() { if(lose_focus_handler) lose_focus_handler(this); }


//Returns the appropriate background color, taking into account whether the widget is enabled or not.
ALLEGRO_COLOR lafi_widget::get_bg_color() {
    if(is_disabled()) return style->disabled_bg_color;
    return style->bg_color;
}

//Returns the appropriate lighter background color, taking into account whether the widget is enabled or not.
ALLEGRO_COLOR lafi_widget::get_lighter_bg_color() {
    if(is_disabled()) return style->lighter_disabled_bg_color;
    return style->lighter_bg_color;
}


//Returns the appropriate darker background color, taking into account whether the widget is enabled or not.
ALLEGRO_COLOR lafi_widget::get_darker_bg_color() {
    if(is_disabled()) return style->darker_disabled_bg_color;
    return style->darker_bg_color;
}


//Returns the appropriate foreground color, taking into account whether the widget is enabled or not.
ALLEGRO_COLOR lafi_widget::get_fg_color() {
    if(is_disabled()) return style->disabled_fg_color;
    return style->fg_color;
}

//Returns the appropriate alternate color, taking into account whether the widget is enabled or not.
ALLEGRO_COLOR lafi_widget::get_alt_color() {
    if(is_disabled()) return style->disabled_alt_color;
    return style->alt_color;
}

//Returns which widget the mouse is under.
//It searches for the deepmost child widget, and if none has it,
//it returns either itself, or none.
//Disabled widgets are ignored.
lafi_widget* lafi_widget::get_widget_under_mouse(int mx, int my) {
    if(!(flags & LAFI_FLAG_DISABLED)) {
        if(!(flags & LAFI_FLAG_WUM_NO_CHILDREN)) {
            for(auto c = widgets.begin(); c != widgets.end(); c++) {
                lafi_widget* w = c->second->get_widget_under_mouse(mx, my);
                if(w) return w;
            }
        }
        if(is_mouse_in(mx, my)) return this;
    }
    return NULL;
}

/*
 * Checks if the widget is disabled, either because of its flags,
 * or because of one of its parents' flags.
 */
bool lafi_widget::is_disabled() {
    lafi_widget* p = this;
    while(p->parent) {
        if((p->flags & LAFI_FLAG_DISABLED) != 0) return true;
        p = p->parent;
    }
    return false;
}

/*
 * Checks if the mouse cursor is inside the widget, given its coordinates.
 */
bool lafi_widget::is_mouse_in(int mx, int my) {
    int ox, oy;
    get_offset(&ox, &oy);
    bool in_current_widget = mx >= x1 + ox && mx <= x2 + ox && my >= y1 + oy && my <= y2 + oy;
    bool in_parent_widget = true;
    if(parent) in_parent_widget = parent->is_mouse_in(mx, my);
    
    return(in_current_widget && in_parent_widget);
}

void lafi_widget::get_offset(int* ox, int* oy) {
    if(!parent) { *ox = 0; *oy = 0; return; }
    
    int parent_parent_offset_x, parent_parent_offset_y;
    parent->get_offset(&parent_parent_offset_x, &parent_parent_offset_y);
    *ox = parent->children_offset_x + parent_parent_offset_x;
    *oy = parent->children_offset_y + parent_parent_offset_y;
}

void lafi_widget::add(string name, lafi_widget* widget) {
    widgets[name] = widget;
    widget->parent = this;
    if(!widget->style) widget->style = style;
    if(widget->needs_init) widget->init();
}

void lafi_widget::register_accelerator(int key, unsigned int modifiers, lafi_widget* widget) {
    accelerators.push_back(lafi_accelerator(key, modifiers, widget));
}

/*
 * Draws the widget on the specified bitmap/display.
 */
void lafi_widget::draw() {
    if(flags & LAFI_FLAG_INVISIBLE) return;
    
    if(!style) {
        //Last attempt at making things right: if there's no style, try using the parent's style now. If not even the parent has it, try to use the default style.
        if(parent) {
            if(parent->style) {
                style = parent->style;
            } else {
                style = new lafi_style();
            }
        } else {
            style = new lafi_style();
        }
    }
    
    draw_self();
    
    int ocr_x, ocr_y, ocr_w, ocr_h;  //Original clipping rectangle.
    al_get_clipping_rectangle(&ocr_x, &ocr_y, &ocr_w, &ocr_h);
    
    if(!(flags & LAFI_FLAG_NO_CLIPPING_RECTANGLE)) al_set_clipping_rectangle(x1, y1, x2 - x1, y2 - y1); {
        ALLEGRO_TRANSFORM t;
        int ox, oy;
        get_offset(&ox, &oy);
        al_build_transform(&t, ox + children_offset_x, oy + children_offset_y, 1, 1, 0);
        
        ALLEGRO_TRANSFORM old;
        al_copy_transform(&old, al_get_current_transform());
        
        al_use_transform(&t); {
        
            for(auto w = widgets.begin(); w != widgets.end(); w++) {
                w->second->draw();
            }
            
        } al_use_transform(&old);
        
    } if(!(flags & LAFI_FLAG_NO_CLIPPING_RECTANGLE)) al_set_clipping_rectangle(ocr_x, ocr_y, ocr_w, ocr_h);
    
}

void lafi_widget::handle_event(ALLEGRO_EVENT ev) {
    if(flags & LAFI_FLAG_DISABLED) return;
    
    if(ev.type == ALLEGRO_EVENT_MOUSE_AXES || ev.type == ALLEGRO_EVENT_MOUSE_WARPED || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
    
        for(auto w = widgets.begin(); w != widgets.end(); w++) {
        
            if(w->second->flags & LAFI_FLAG_DISABLED) continue;
            
            if(w->second->mouse_in) {
                if(!w->second->is_mouse_in(ev.mouse.x, ev.mouse.y)) {
                
                    //Mouse was in but left.
                    w->second->widget_on_mouse_leave();
                    w->second->mouse_in = false;
                    w->second->call_mouse_leave_handler();
                }
                
                if(ev.mouse.dx != 0 || ev.mouse.dy != 0) {
                    w->second->widget_on_mouse_move(ev.mouse.x, ev.mouse.y);
                    w->second->call_mouse_move_handler(ev.mouse.x, ev.mouse.y);
                }
                if(ev.mouse.dz != 0 || ev.mouse.dw != 0) {
                    w->second->widget_on_mouse_wheel(ev.mouse.dz, ev.mouse.dw);
                    w->second->call_mouse_wheel_handler(ev.mouse.dz, ev.mouse.dw);
                }
                
            } else {
            
                if(w->second->is_mouse_in(ev.mouse.x, ev.mouse.y)) {
                
                    //Mouse was out but is now in.
                    w->second->widget_on_mouse_enter();
                    w->second->mouse_in = true;
                    w->second->call_mouse_enter_handler();
                }
            }
        }
        
    }
    
    if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
    
        for(auto w = widgets.begin(); w != widgets.end(); w++) {
        
            if(w->second->flags & LAFI_FLAG_DISABLED) continue;
            
            if(w->second->mouse_in) {
            
                w->second->widget_on_mouse_down(ev.mouse.button, ev.mouse.x, ev.mouse.y);
                if(ev.mouse.button == 1) w->second->mouse_clicking = true;
                
                //Mark focus lost. First go up to the topmost parent, and let it tell everybody to lose their focuses.
                lafi_widget* p = this;
                while(p->parent) p = p->parent;
                p->lose_focus();
                
                focused_widget = w->second;
                w->second->call_get_focus_handler();
                w->second->call_mouse_down_handler(ev.mouse.button, ev.mouse.x, ev.mouse.y);
                
            } else {
                if(ev.mouse.button == 1) w->second->mouse_clicking = false;
            }
        }
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
    
        for(auto w = widgets.begin(); w != widgets.end(); w++) {
        
            if(w->second->flags & LAFI_FLAG_DISABLED) continue;
            
            if(w->second->mouse_in) {
            
                w->second->widget_on_mouse_up(ev.mouse.button, ev.mouse.x, ev.mouse.y);
                w->second->call_mouse_up_handler(ev.mouse.button, ev.mouse.x, ev.mouse.y);
                
                if(ev.mouse.button == 1) {
                    if(w->second->mouse_clicking) {
                    
                        //Mouse was clicking, and a button up just happened. So a full click just happened.
                        w->second->widget_on_left_mouse_click(ev.mouse.x, ev.mouse.y);
                        w->second->call_left_mouse_click_handler(ev.mouse.x, ev.mouse.y);
                    }
                }
            }
            if(ev.mouse.button == 1) w->second->mouse_clicking = false;
        }
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_CHAR) {
        if(focused_widget) {
            if(!(focused_widget->flags & LAFI_FLAG_DISABLED)) {
                focused_widget->widget_on_key_char(ev.keyboard.keycode, ev.keyboard.unichar, ev.keyboard.modifiers);
            }
        }
        
        for(size_t a = 0; a < accelerators.size(); a++) {
            lafi_accelerator* a_ptr = &accelerators[a];
            if(ev.keyboard.keycode == a_ptr->key && ev.keyboard.modifiers == a_ptr->modifiers) {
                a_ptr->widget->call_left_mouse_click_handler(0, 0);
            }
        }
    }
    
    //Now let children widgets handle events.
    for(auto w = widgets.begin(); w != widgets.end(); w++) {
        w->second->handle_event(ev);
    }
}

void lafi_widget::remove(string child_name) {
    if(widgets.find(child_name) == widgets.end()) return;
    
    if(focused_widget == widgets[child_name]) focused_widget = NULL;
    delete widgets[child_name];
    widgets.erase(widgets.find(child_name));
}

void lafi_widget::widget_on_mouse_move(int, int) { }
void lafi_widget::widget_on_left_mouse_click(int, int) { }
void lafi_widget::widget_on_mouse_down(int, int, int) { }
void lafi_widget::widget_on_mouse_up(int, int, int) { }
void lafi_widget::widget_on_mouse_wheel(int, int) { }
void lafi_widget::widget_on_mouse_enter() { }
void lafi_widget::widget_on_mouse_leave() { }
void lafi_widget::widget_on_key_char(int, int, unsigned int) { }
void lafi_widget::init() { }

void lafi_widget::lose_focus() {
    if(focused_widget) {
        focused_widget->call_lose_focus_handler();
        focused_widget = NULL;
    }
    
    for(auto cw = widgets.begin(); cw != widgets.end(); cw++) {
        cw->second->lose_focus();
    }
}

/*
 * Creates a row and commits the previous one.
 * vertical_padding:   Padding between this new row and the previous one, in pixels.
 * horizontal_padding: Padding between the left and right sides, in pixels.
 * widget_padding:     Padding between added widgets, in pixels.
 * Returns the y of the next row.
 */
int lafi_widget::easy_row(float vertical_padding, float horizontal_padding, float widget_padding) {
    if(easy_row_widgets.size()) {
        //Find the tallest widget.
        float tallest_height = easy_row_widgets[0].height;
        float available_width = (x2 - x1) - ((easy_row_widgets.size() - 1) * easy_row_widget_padding) - (easy_row_horizontal_padding * 2);
        float prev_x = x1 + easy_row_horizontal_padding;
        
        for(size_t w = 1; w < easy_row_widgets.size(); w++) {
            if(easy_row_widgets[w].height > tallest_height) tallest_height = easy_row_widgets[w].height;
        }
        
        easy_row_y2 = easy_row_y1 + tallest_height;
        float y_center = (easy_row_y2 + easy_row_y1) / 2 + y1;
        
        for(size_t w = 0; w < easy_row_widgets.size(); w++) {
            lafi_easy_widget_info* i_ptr = &easy_row_widgets[w];
            
            float width =
                (
                    (i_ptr->flags & LAFI_EASY_FLAG_WIDTH_PX) != 0 ?
                    i_ptr->width :
                    available_width * (i_ptr->width / 100)
                );
            i_ptr->widget->x1 = prev_x;
            i_ptr->widget->x2 = prev_x + width;
            prev_x = i_ptr->widget->x2 + easy_row_widget_padding;
            
            i_ptr->widget->y1 = y_center - i_ptr->height / 2;
            i_ptr->widget->y2 = y_center + i_ptr->height / 2;
            
            add(i_ptr->name, i_ptr->widget);
        }
    }
    easy_row_widgets.clear();
    easy_row_y1 = easy_row_y2 + vertical_padding;
    easy_row_y2 = easy_row_y1;
    easy_row_vertical_padding = vertical_padding;
    easy_row_horizontal_padding = horizontal_padding;
    easy_row_widget_padding = widget_padding;
    
    return easy_row_y1;
}

/*
 * Adds a widget to the current row.
 * name:   Name.
 * widget: The widget.
 * width:  Width it takes up on the row, in percentage.
 * height: Height of the widget, in pixels.
 * flags:  Use LAFI_EASY_FLAG_*.
 */
void lafi_widget::easy_add(string name, lafi_widget* widget, float width, float height, unsigned char flags) {
    easy_row_widgets.push_back(lafi_easy_widget_info(name, widget, width, height, flags));
}

/*
 * Resets the rows.
 */
void lafi_widget::easy_reset() {
    easy_row_vertical_padding = 0;
    easy_row_horizontal_padding = 0;
    easy_row_widget_padding = 0;
    easy_row_y1 = 0;
    easy_row_y2 = 0;
}




lafi_easy_widget_info::lafi_easy_widget_info(string name, lafi_widget* widget, float width, float height, unsigned char flags) {
    this->name = name;
    this->widget = widget;
    this->width = width;
    this->height = height;
    this->flags = flags;
}

lafi_accelerator::lafi_accelerator(int key, unsigned int modifiers, lafi_widget* widget) {
    this->key = key;
    this->modifiers = modifiers;
    this->widget = widget;
}




/*
 * Draws a line in one side of a rectangle. This is used because al_draw_line gets too confusing after a bit.
 * Each line is drawn from a side: right, top, left or bottom. It starts on the top-left corner for top and
 * left-side lines, top-right corner for right-side, and bottom-left corner for bottom-side.
 * widget:          Widget, used to get the coordinates.
 * side:            Side of the rectangle to draw. Use LAFI_DRAW_LINE_*.
 * start_offset:    Start these many points to the right (horizontal lines) or down (vertical). 0 means the exact corner.
 * end_offset:      End these many pixels to the left (horizontal lines) or above (vertical). 0 means the exact corner.
 * location_offset: Place the lines these many pixels closer to the center.
 ** e.g. if the line is BOTTOM-side, setting this to 1 makes it draw on the 2nd to last row of pixels.
 * color:           Color.
 */
void lafi_draw_line(lafi_widget* widget, unsigned char side, int start_offset, int end_offset, int location_offset, ALLEGRO_COLOR color) {
    int x1 = widget->x1,  x2 = widget->x2;
    int y1 = widget->y1,  y2 = widget->y2;
    
    if(side == LAFI_DRAW_LINE_RIGHT || side == LAFI_DRAW_LINE_LEFT) {
        int line_x = (side == LAFI_DRAW_LINE_RIGHT) ? (x2 - location_offset + 0.5) : (x1 + location_offset + 1.5);
        al_draw_line(
            line_x, y1 + start_offset,
            line_x, y2 - end_offset,
            color, 1
        );
    } else {
        int line_y = (side == LAFI_DRAW_LINE_TOP) ? (y1 + location_offset + 1.5) : (y2 - location_offset + 0.5);
        al_draw_line(
            x1 + start_offset, line_y,
            x2 - end_offset, line_y,
            color, 1
        );
    }
}