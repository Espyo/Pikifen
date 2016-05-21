#include <algorithm>

#include "widget.h"

namespace lafi {

/* ----------------------------------------------------------------------------
 * Creates a widget given some parameters.
 */
widget::widget(
    int x1, int y1, int x2, int y2, lafi::style* style, unsigned char flags
) :
    x1(x1),
    y1(y1),
    x2(x2),
    y2(y2),
    flags(flags),
    style(style),
    children_offset_x(0),
    children_offset_y(0),
    focused_widget(nullptr),
    parent(nullptr),
    mouse_in(false),
    mouse_clicking(false),
    mouse_move_handler(nullptr),
    left_mouse_click_handler(nullptr),
    mouse_down_handler(nullptr),
    mouse_up_handler(nullptr),
    mouse_wheel_handler(nullptr),
    mouse_enter_handler(nullptr),
    mouse_leave_handler(nullptr),
    get_focus_handler(nullptr),
    lose_focus_handler(nullptr),
    needs_init(false) {
    
    easy_reset();
}


/* ----------------------------------------------------------------------------
 * Creates a widget by copying the info from another widget.
 */
widget::widget(widget &w2) :
    x1(w2.x1),
    x2(w2.x2),
    y1(w2.y1),
    y2(w2.y2),
    flags(w2.flags),
    style(w2.style),
    children_offset_x(w2.children_offset_x),
    children_offset_y(w2.children_offset_y),
    focused_widget(w2.focused_widget),
    parent(nullptr),
    mouse_in(false),
    mouse_clicking(false),
    mouse_move_handler(nullptr),
    left_mouse_click_handler(nullptr),
    mouse_down_handler(nullptr),
    mouse_up_handler(nullptr),
    mouse_wheel_handler(nullptr),
    mouse_enter_handler(nullptr),
    mouse_leave_handler(nullptr),
    get_focus_handler(nullptr),
    lose_focus_handler(nullptr),
    needs_init(false) {
    
    easy_reset();
    
}


/* ----------------------------------------------------------------------------
 * Destroys a widget.
 */
widget::~widget() {
}


//Calls the function that handles a mouse move.
void widget::call_mouse_move_handler(int x, int y) {
    if(mouse_move_handler) mouse_move_handler(this, x, y);
}

//Calls the function that handles a left mouse click.
void widget::call_left_mouse_click_handler(int x, int y) {
    if(left_mouse_click_handler) left_mouse_click_handler(this, x, y);
}

//Calls the function that handles a mouse button down.
void widget::call_mouse_down_handler(int button, int x, int y) {
    if(mouse_down_handler) mouse_down_handler(this, button, x, y);
}

//Calls the function that handles a mouse button up.
void widget::call_mouse_up_handler(int button, int x, int y) {
    if(mouse_up_handler) mouse_up_handler(this, button, x, y);
}

//Calls the function that handles a mouse wheel move.
void widget::call_mouse_wheel_handler(int dy, int dx) {
    if(mouse_wheel_handler) mouse_wheel_handler(this, dy, dx);
}

//Calls the function that handles the mouse entering.
void widget::call_mouse_enter_handler() {
    if(mouse_enter_handler) mouse_enter_handler(this);
}

//Calls the function that handles the mouse leaving.
void widget::call_mouse_leave_handler() {
    if(mouse_leave_handler) mouse_leave_handler(this);
}

//Calls the function that handles the focus being obtained.
void widget::call_get_focus_handler() {
    if(get_focus_handler) get_focus_handler(this);
}

//Calls the function that handles the focus being lost.
void widget::call_lose_focus_handler() {
    if(lose_focus_handler) lose_focus_handler(this);
}


//Returns the appropriate background color,
//taking into account whether the widget is enabled or not.
ALLEGRO_COLOR widget::get_bg_color() {
    if(is_disabled()) return style->disabled_bg_color;
    return style->bg_color;
}


//Returns the appropriate lighter background color,
//taking into account whether the widget is enabled or not.
ALLEGRO_COLOR widget::get_lighter_bg_color() {
    if(is_disabled()) return style->lighter_disabled_bg_color;
    return style->lighter_bg_color;
}


//Returns the appropriate darker background color,
//taking into account whether the widget is enabled or not.
ALLEGRO_COLOR widget::get_darker_bg_color() {
    if(is_disabled()) return style->darker_disabled_bg_color;
    return style->darker_bg_color;
}


//Returns the appropriate foreground color,
//taking into account whether the widget is enabled or not.
ALLEGRO_COLOR widget::get_fg_color() {
    if(is_disabled()) return style->disabled_fg_color;
    return style->fg_color;
}


//Returns the appropriate alternate color,
//taking into account whether the widget is enabled or not.
ALLEGRO_COLOR widget::get_alt_color() {
    if(is_disabled()) return style->disabled_alt_color;
    return style->alt_color;
}


//Returns which widget the mouse is under.
//It searches for the deepmost child widget, and if none has it,
//it returns either itself, or none.
//Disabled widgets are ignored.
widget* widget::get_widget_under_mouse(int mx, int my) {
    if(!(flags & FLAG_DISABLED)) {
        if(!(flags & FLAG_WUM_NO_CHILDREN)) {
            for(auto c = widgets.begin(); c != widgets.end(); ++c) {
                widget* w = c->second->get_widget_under_mouse(mx, my);
                if(w) return w;
            }
        }
        if(is_mouse_in(mx, my)) return this;
    }
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Checks if the widget is disabled, either because of its flags,
 * or because of one of its parents' flags.
 */
bool widget::is_disabled() {
    widget* p = this;
    while(p->parent) {
        if((p->flags & FLAG_DISABLED) != 0) return true;
        p = p->parent;
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * Checks if the mouse cursor is inside the widget, given its coordinates.
 */
bool widget::is_mouse_in(int mx, int my) {
    int ox, oy;
    get_offset(&ox, &oy);
    bool in_current_widget =
        mx >= x1 + ox && mx <= x2 + ox && my >= y1 + oy && my <= y2 + oy;
    bool in_parent_widget = true;
    if(parent) in_parent_widget = parent->is_mouse_in(mx, my);
    
    return(in_current_widget && in_parent_widget);
}


/* ----------------------------------------------------------------------------
 * Returns the total offset in pixels.
 * Takes into account all parent's offsets.
 */
void widget::get_offset(int* ox, int* oy) {
    if(!parent) { *ox = 0; *oy = 0; return; }
    
    int parent_parent_offset_x, parent_parent_offset_y;
    parent->get_offset(&parent_parent_offset_x, &parent_parent_offset_y);
    *ox = parent->children_offset_x + parent_parent_offset_x;
    *oy = parent->children_offset_y + parent_parent_offset_y;
}


/* ----------------------------------------------------------------------------
 * Adds a widget as a child to the current one.
 */
void widget::add(string name, widget* w) {
    widgets[name] = w;
    w->parent = this;
    if(!w->style) w->style = style;
    if(w->needs_init) w->init();
}


/* ----------------------------------------------------------------------------
 * Registers a key accelerator.
 */
void widget::register_accelerator(int key, unsigned int modifiers, widget* w) {
    accelerators.push_back(accelerator(key, modifiers, w));
}


/* ----------------------------------------------------------------------------
 * Draws the widget on the specified bitmap/display.
 */
void widget::draw() {
    if(flags & FLAG_INVISIBLE) return;
    
    if(!style) {
        //Last attempt at making things right: if there's no style,
        //try using the parent's style now. If not even the parent has it,
        //try to use the default style.
        if(parent) {
            if(parent->style) {
                style = parent->style;
            } else {
                style = new lafi::style();
            }
        } else {
            style = new lafi::style();
        }
    }
    
    int ox, oy;
    get_offset(&ox, &oy);
    
    int ocr_x, ocr_y, ocr_w, ocr_h;  //Original clipping rectangle.
    al_get_clipping_rectangle(&ocr_x, &ocr_y, &ocr_w, &ocr_h);
    
    if((flags & FLAG_NO_CLIPPING_RECTANGLE) == 0) {
        int rx1 = x1 + ox;
        int rx2 = x2 + ox;
        int ry1 = y1 + oy;
        int ry2 = y2 + oy;
        int ocr_x2 = ocr_x + ocr_w;
        int ocr_y2 = ocr_y + ocr_h;
        al_set_clipping_rectangle(
            max(rx1, ocr_x),
            max(ry1, ocr_y),
            min(ocr_x2, rx2) - rx1,
            min(ocr_y2, ry2) - ry1
        );
    }
    {
        draw_self();
        
        ALLEGRO_TRANSFORM t;
        al_build_transform(
            &t, ox + children_offset_x, oy + children_offset_y, 1, 1, 0
        );
        
        ALLEGRO_TRANSFORM old;
        al_copy_transform(&old, al_get_current_transform());
        
        al_use_transform(&t); {
        
            for(auto w = widgets.begin(); w != widgets.end(); ++w) {
                if(w->second) w->second->draw();
            }
            
        } al_use_transform(&old);
        
    }
    if((flags & FLAG_NO_CLIPPING_RECTANGLE) == 0) {
        al_set_clipping_rectangle(ocr_x, ocr_y, ocr_w, ocr_h);
    }
    
}


/* ----------------------------------------------------------------------------
 * Handles an Allegro event. The most important part of this
 * is handling the mouse and keyboard events, so that each
 * sub-class of widget can do whatever with them.
 */
void widget::handle_event(ALLEGRO_EVENT ev) {
    if(flags & FLAG_DISABLED) return;
    
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_WARPED ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
    
        for(auto w = widgets.begin(); w != widgets.end(); ++w) {
        
            if(!w->second) continue;
            
            if(w->second->flags & FLAG_DISABLED) continue;
            
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
                    w->second->call_mouse_wheel_handler(
                        ev.mouse.dz, ev.mouse.dw
                    );
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
    
        for(auto w = widgets.begin(); w != widgets.end(); ++w) {
        
            if(!w->second) continue;
            
            if(w->second->flags & FLAG_DISABLED) continue;
            
            if(w->second->mouse_in) {
            
                w->second->widget_on_mouse_down(
                    ev.mouse.button, ev.mouse.x, ev.mouse.y
                );
                if(ev.mouse.button == 1) w->second->mouse_clicking = true;
                
                give_focus(w->second);
                
                w->second->call_mouse_down_handler(
                    ev.mouse.button, ev.mouse.x, ev.mouse.y
                );
                
            } else {
                if(ev.mouse.button == 1) w->second->mouse_clicking = false;
            }
        }
        
    } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
    
        for(auto w = widgets.begin(); w != widgets.end(); ++w) {
        
            if(!w->second) continue;
            
            if(w->second->flags & FLAG_DISABLED) continue;
            
            if(w->second->mouse_in) {
            
                w->second->widget_on_mouse_up(
                    ev.mouse.button, ev.mouse.x, ev.mouse.y
                );
                w->second->call_mouse_up_handler(
                    ev.mouse.button, ev.mouse.x, ev.mouse.y
                );
                
                if(ev.mouse.button == 1) {
                    if(w->second->mouse_clicking) {
                    
                        //Mouse was clicking, and a button up just happened.
                        //So a full click just happened.
                        w->second->widget_on_left_mouse_click(
                            ev.mouse.x, ev.mouse.y
                        );
                        w->second->call_left_mouse_click_handler(
                            ev.mouse.x, ev.mouse.y
                        );
                    }
                }
            }
            if(ev.mouse.button == 1) w->second->mouse_clicking = false;
        }
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_CHAR) {
        if(focused_widget) {
            if(!(focused_widget->flags & FLAG_DISABLED)) {
                focused_widget->widget_on_key_char(
                    ev.keyboard.keycode, ev.keyboard.unichar,
                    ev.keyboard.modifiers
                );
            }
        }
        
        for(size_t a = 0; a < accelerators.size(); ++a) {
            accelerator* a_ptr = &accelerators[a];
            if(
                ev.keyboard.keycode == a_ptr->key &&
                ev.keyboard.modifiers == a_ptr->modifiers
            ) {
                a_ptr->w->call_left_mouse_click_handler(0, 0);
            }
        }
    }
    
    //Now let children widgets handle events.
    for(auto w = widgets.begin(); w != widgets.end(); ++w) {
        if(w->second) w->second->handle_event(ev);
    }
}


/* ----------------------------------------------------------------------------
 * Removes a child widget from the list.
 */
void widget::remove(string child_name) {
    if(widgets.find(child_name) == widgets.end()) return;
    
    if(focused_widget == widgets[child_name]) focused_widget = NULL;
    delete widgets[child_name];
    widgets.erase(widgets.find(child_name));
}


void widget::widget_on_mouse_move(int, int) { }
void widget::widget_on_left_mouse_click(int, int) { }
void widget::widget_on_mouse_down(int, int, int) { }
void widget::widget_on_mouse_up(int, int, int) { }
void widget::widget_on_mouse_wheel(int, int) { }
void widget::widget_on_mouse_enter() { }
void widget::widget_on_mouse_leave() { }
void widget::widget_on_key_char(int, int, unsigned int) { }
void widget::init() { }


/* ----------------------------------------------------------------------------
 * Makes this widget and all of its children lose focus.
 */
void widget::lose_focus() {
    if(focused_widget) {
        focused_widget->call_lose_focus_handler();
        focused_widget = NULL;
    }
    
    for(auto cw = widgets.begin(); cw != widgets.end(); ++cw) {
        if(cw->second) cw->second->lose_focus();
    }
}


/* ----------------------------------------------------------------------------
 * Gives focus to this widget, making all other widgets
 * lose focus in the process.
 */
void widget::give_focus(widget* w) {
    if(!w) return;
    //Mark focus lost. First go up to the topmost parent,
    //and let it tell everybody to lose their focuses.
    widget* p = this;
    while(p->parent) p = p->parent;
    p->lose_focus();
    
    focused_widget = w;
    w->call_get_focus_handler();
}


/* ----------------------------------------------------------------------------
 * Creates an "easy add" row and commits the previous one.
 * vertical_padding:   Padding between this new row and the previous one,
   * in pixels.
 * horizontal_padding: Padding between the left and right sides, in pixels.
 * widget_padding:     Padding between added widgets, in pixels.
 * Returns the y of the next row.
 */
int widget::easy_row(
    float vertical_padding, float horizontal_padding, float widget_padding
) {
    if(easy_row_widgets.size()) {
        //Find the tallest widget.
        float tallest_height = easy_row_widgets[0].height;
        float available_width =
            (x2 - x1) - (
                (
                    easy_row_widgets.size() - 1) *
                easy_row_widget_padding
            ) - (easy_row_horizontal_padding * 2);
        float prev_x = x1 + easy_row_horizontal_padding;
        
        for(size_t w = 1; w < easy_row_widgets.size(); ++w) {
            if(easy_row_widgets[w].height > tallest_height) {
                tallest_height = easy_row_widgets[w].height;
            }
        }
        
        easy_row_y2 = easy_row_y1 + tallest_height;
        float y_center = (easy_row_y2 + easy_row_y1) / 2 + y1;
        
        for(size_t w = 0; w < easy_row_widgets.size(); ++w) {
            easy_widget_info* i_ptr = &easy_row_widgets[w];
            
            float width =
                (
                    (i_ptr->flags & EASY_FLAG_WIDTH_PX) != 0 ?
                    i_ptr->width :
                    available_width * (i_ptr->width / 100)
                );
            i_ptr->w->x1 = prev_x;
            i_ptr->w->x2 = prev_x + width;
            prev_x = i_ptr->w->x2 + easy_row_widget_padding;
            
            i_ptr->w->y1 = y_center - i_ptr->height / 2;
            i_ptr->w->y2 = y_center + i_ptr->height / 2;
            
            add(i_ptr->name, i_ptr->w);
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


/* ----------------------------------------------------------------------------
 * "Easy add"s a widget to the current row.
 * name:   Name.
 * widget: The widget.
 * width:  Width it takes up on the row, in percentage.
 * height: Height of the widget, in pixels.
 * flags:  Use EASY_FLAG_*.
 */
void widget::easy_add(
    string name, widget* w, float width, float height, unsigned char flags
) {
    easy_row_widgets.push_back(easy_widget_info(name, w, width, height, flags));
}


/* ----------------------------------------------------------------------------
 * Resets the "easy add" rows.
 */
void widget::easy_reset() {
    easy_row_vertical_padding = 0;
    easy_row_horizontal_padding = 0;
    easy_row_widget_padding = 0;
    easy_row_y1 = 0;
    easy_row_y2 = 0;
}




/* ----------------------------------------------------------------------------
 * Creates an "easy add" widget info structure.
 */
easy_widget_info::easy_widget_info(
    string name, lafi::widget* w, float width, float height,
    unsigned char flags
) :
    name(name),
    w(w),
    width(width),
    height(height),
    flags(flags) {
    
}


/* ----------------------------------------------------------------------------
 * Creates an accelerator.
 */
accelerator::accelerator(int key, unsigned int modifiers, lafi::widget* w) {
    this->key = key;
    this->modifiers = modifiers;
    this->w = w;
}




/* ----------------------------------------------------------------------------
 * Draws a line in one side of a rectangle.
 * This is used because al_draw_line gets too confusing after a bit.
 * Each line is drawn from a side: right, top, left or bottom.
 * It starts on the top-left corner for top and
 * left-side lines, top-right corner for right-side,
 * and bottom-left corner for bottom-side.
 * widget:          Widget, used to get the coordinates.
 * side:            Side of the rectangle to draw. Use DRAW_LINE_*.
 * start_offset:    Start these many points to the right (horizontal lines)
   * or down (vertical). 0 means the exact corner.
 * end_offset:      End these many pixels to the left (horizontal lines)
   * or above (vertical). 0 means the exact corner.
 * location_offset: Place the lines these many pixels closer to the center.
   * e.g. if the line is BOTTOM-side, setting this to 1 makes it draw on the
   * 2nd to last row of pixels.
 * color:           Color.
 */
void draw_line(
    widget* w, unsigned char side, int start_offset, int end_offset,
    int location_offset, ALLEGRO_COLOR color
) {
    float x1 = w->x1, x2 = w->x2;
    float y1 = w->y1, y2 = w->y2;
    
    if(side == DRAW_LINE_RIGHT || side == DRAW_LINE_LEFT) {
        float line_x =
            (side == DRAW_LINE_RIGHT) ?
            (x2 - location_offset - 0.5) :
            (x1 + location_offset + 0.5);
        al_draw_line(
            line_x, y1 + start_offset,
            line_x, y2 - end_offset,
            color, 1
        );
    } else {
        float line_y =
            (side == DRAW_LINE_TOP) ?
            (y1 + location_offset + 0.5) :
            (y2 - location_offset - 0.5);
        al_draw_line(
            x1 + start_offset, line_y,
            x2 - end_offset, line_y,
            color, 1
        );
    }
}


/* ----------------------------------------------------------------------------
 * Draws text, but if there are line breaks,
 * it'll draw every line one under the other.
 * It basically calls Allegro's text drawing functions, but for each line.
 * f:    Font to use.
 * c:    Color.
 * x/y:  Coordinates of the text.
 * fl:   Flags, just like the ones you'd pass to al_draw_text.
 * va:   Vertical align: 0 for top, 1 for center, 2 for bottom.
 * text: Text to write, line breaks included ('\n').
 */
void draw_text_lines(
    const ALLEGRO_FONT* const f, const ALLEGRO_COLOR &c,
    const float x, const float y, const int fl,
    const unsigned char va, const string &text
) {
    vector<string> lines = split(text, "\n", true);
    int fh = al_get_font_line_height(f);
    size_t n_lines = lines.size();
    float top;
    
    if(va == 0) {
        top = y;
    } else {
        //We add n_lines - 1 because there is a 1px gap between each line.
        int total_height = n_lines * fh + (n_lines - 1);
        if(va == 1) {
            top = y - total_height / 2;
        } else {
            top = y - total_height;
        }
    }
    
    for(size_t l = 0; l < n_lines; ++l) {
        float line_y = (fh + 1) * l + top;
        al_draw_text(f, c, x, line_y, fl, lines[l].c_str());
    }
}


/* ----------------------------------------------------------------------------
 * Splits a string into several substrings, by the specified delimiter.
 * text:        The string to split.
 * del:         The delimiter. Default is space.
 * inc_empty:   If true, include empty substrings on the vector.
   * i.e. if two delimiters come together in a row,
   * keep an empty substring between.
 * inc_del:     If true, include the delimiters on the vector as a substring.
 */
vector<string> split(
    string text, const string &del, const bool inc_empty, const bool inc_del
) {
    vector<string> v;
    size_t pos;
    size_t del_size = del.size();
    
    do {
        pos = text.find(del);
        if (pos != string::npos) {  //If it DID find the delimiter.
            //Get the text between the start and the delimiter.
            string sub = text.substr(0, pos);
            
            //Add the text before the delimiter to the vector.
            if(sub != "" || inc_empty)
                v.push_back(sub);
                
            //Add the delimiter to the vector, but only if requested.
            if(inc_del)
                v.push_back(del);
                
            //Delete everything before the delimiter, including
            //the delimiter itself, and search again.
            text.erase(text.begin(), text.begin() + pos + del_size);
        }
    } while (pos != string::npos);
    
    //Text after the final delimiter.
    //(If there is one. If not, it's just the whole string.)
    //If it's a blank string, only add it if we want empty strings.
    if (text != "" || inc_empty)
        v.push_back(text);
        
    return v;
}

}
