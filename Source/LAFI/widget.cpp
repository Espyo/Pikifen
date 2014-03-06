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
    
    children_offset_x = children_offset_y = 0;
    focused_widget = NULL;
    mouse_over_widget = NULL;
    parent = NULL;
    mouse_in = false;
    mouse_clicking = false;
    
    needs_rerender = true;
    
    mouse_move_handler = NULL;
    left_mouse_click_handler = NULL;
    mouse_down_handler = NULL;
    mouse_up_handler = NULL;
    mouse_enter_handler = NULL;
    mouse_leave_handler = NULL;
    get_focus_handler = NULL;
    lose_focus_handler = NULL;
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
    
    children_offset_x = w2.children_offset_x;
    children_offset_y = w2.children_offset_y;
    focused_widget = w2.focused_widget;
    mouse_over_widget = w2.mouse_over_widget;
    parent = NULL;
    mouse_in = false;
    mouse_clicking = false;
    
    mouse_move_handler = NULL;
    left_mouse_click_handler = NULL;
    mouse_down_handler = NULL;
    mouse_up_handler = NULL;
    mouse_enter_handler = NULL;
    mouse_leave_handler = NULL;
    get_focus_handler = NULL;
    lose_focus_handler = NULL;
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

//Calls the function that handles the mouse entering.
void lafi_widget::call_mouse_enter_handler() { if(mouse_enter_handler) mouse_enter_handler(this); }

//Calls the function that handles the mouse leaving.
void lafi_widget::call_mouse_leave_handler() { if(mouse_leave_handler) mouse_leave_handler(this); }

//Calls the function that handles the focus being obtained.
void lafi_widget::call_get_focus_handler() { if(get_focus_handler) get_focus_handler(this); }

//Calls the function that handles the focus being lost.
void lafi_widget::call_lose_focus_handler() { if(lose_focus_handler) lose_focus_handler(this); }

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
    
    if(needs_rerender) {
        needs_rerender = false;
        render();
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
    if(flags & LAFI_FLAG_DISABLED) {
        mouse_over_widget = NULL;
        return;
    }
    
    if(ev.type == ALLEGRO_EVENT_MOUSE_AXES || ev.type == ALLEGRO_EVENT_MOUSE_WARPED) {
    
        for(auto w = widgets.begin(); w != widgets.end(); w++) {
        
            if(w->second->flags & LAFI_FLAG_DISABLED) continue;
            
            if(w->second->mouse_in) {
                if(!w->second->is_mouse_in(ev.mouse.x, ev.mouse.y)) {
                
                    //Mouse was in but left.
                    w->second->widget_on_mouse_leave();
                    w->second->mouse_in = false;
                    w->second->call_mouse_leave_handler();
                    
                    if(mouse_over_widget == w->second) {
                        //If the mouse was over this widget and left, set it to none.
                        mouse_over_widget = NULL;
                    }
                }
                
                w->second->widget_on_mouse_move(ev.mouse.x, ev.mouse.y);
                w->second->call_mouse_move_handler(ev.mouse.x, ev.mouse.y);
                
            } else {
            
                if(w->second->is_mouse_in(ev.mouse.x, ev.mouse.y)) {
                
                    //Mouse was out but is now in.
                    w->second->widget_on_mouse_enter();
                    w->second->mouse_in = true;
                    w->second->call_mouse_enter_handler();
                    
                    mouse_over_widget = w->second;
                }
            }
        }
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
    
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