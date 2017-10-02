#include <algorithm>
#include <limits.h>

#include "textbox.h"

namespace lafi {

size_t textbox::cur_tab_index = 0;

const float textbox::CURSOR_CHANGE_INTERVAL = 0.5f;

/* ----------------------------------------------------------------------------
 * Creates a textbox given some parameters.
 */
textbox::textbox(
    const int x1, const int y1, const int x2, const int y2, const string &text,
    lafi::style* style, const unsigned char flags
) :
    widget(x1, y1, x2, y2, style, flags),
    cursor_visible(true),
    cursor_change_time_left(0),
    text(text),
    editable(true),
    cursor(0),
    sel_start(0),
    sel_end(0),
    multi_line(false),
    change_handler(nullptr),
    scroll_x(0),
    enter_key_widget(nullptr) {
    
    tab_index = cur_tab_index++;
}


/* ----------------------------------------------------------------------------
 * Creates a textbox given some parameters.
 */
textbox::textbox(const string &text) :
    widget(),
    cursor_visible(true),
    cursor_change_time_left(0),
    text(text),
    editable(true),
    cursor(0),
    sel_start(0),
    sel_end(0),
    multi_line(false),
    change_handler(nullptr),
    scroll_x(0),
    enter_key_widget(nullptr) {
    
    tab_index = cur_tab_index++;
}


/* ----------------------------------------------------------------------------
 * Creates a textbox by copying the info from another textbox.
 */
textbox::textbox(textbox &t2) :
    widget(t2),
    cursor_visible(true),
    cursor_change_time_left(0),
    text(t2.text),
    editable(t2.editable),
    cursor(0),
    sel_start(0),
    sel_end(0),
    multi_line(false),
    change_handler(nullptr),
    scroll_x(0),
    enter_key_widget(nullptr) {
    
    tab_index = cur_tab_index++;
}


/* ----------------------------------------------------------------------------
 * Destroys a textbox.
 */
textbox::~textbox() {}


/* ----------------------------------------------------------------------------
 * Draws the textbox. It's basically a rectangle with a border.
 * The border is drawn line by line. Finally, it draws the
 * text within.
 */
void textbox::draw_self() {
    al_draw_filled_rectangle(x1, y1, x2, y2, get_bg_color());
    //Top line.
    draw_line(this, DRAW_LINE_TOP,    0, 1, 0, get_darker_bg_color());
    //Left line.
    draw_line(this, DRAW_LINE_LEFT,   0, 1, 0, get_darker_bg_color());
    //Bottom line.
    draw_line(this, DRAW_LINE_BOTTOM, 1, 0, 0, get_lighter_bg_color());
    //Right line.
    draw_line(this, DRAW_LINE_RIGHT,  1, 0, 0, get_lighter_bg_color());
    
    if(style->text_font) {
        int text_start = x1 + 2 - scroll_x;
        
        if(parent->focused_widget == this) {
            al_draw_filled_rectangle(
                text_start + al_get_text_width(
                    style->text_font,
                    text.substr(0, min(sel_start, sel_end)).c_str()
                ),
                y1 + 2,
                text_start + al_get_text_width(
                    style->text_font,
                    text.substr(0, max(sel_start, sel_end)).c_str()
                ),
                y2 - 2,
                get_alt_color()
            );
        }
        
        al_draw_text(
            style->text_font, get_fg_color(), text_start,
            (y2 + y1) / 2  - al_get_font_line_height(style->text_font) / 2, 0,
            text.c_str()
        );
        
        if(parent->focused_widget == this && cursor_visible) {
            unsigned int cursor_x =
                al_get_text_width(
                    style->text_font, text.substr(0, cursor).c_str()
                );
                
            al_draw_line(
                x1 + cursor_x + 1.5 - scroll_x,
                y1 + 2,
                x1 + cursor_x + 1.5 - scroll_x,
                y2 - 2,
                get_alt_color(),
                1);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Calls the function that handles a change of the text.
 */
void textbox::call_change_handler() { if(change_handler) change_handler(this); }


/* ----------------------------------------------------------------------------
 * Handles a character (or key) being typed with the keyboard.
 * It updates the textbox accordingly.
 */
void textbox::widget_on_key_char(
    const int keycode, const int unichar, const unsigned int modifiers
) {
    bool ctrl =
        (modifiers & ALLEGRO_KEYMOD_CTRL) ||
        (modifiers & ALLEGRO_KEYMOD_COMMAND);
    bool shift = modifiers & ALLEGRO_KEYMOD_SHIFT;
    
    if(cursor > text.size()) cursor = 0; //If the text is somehow changed.
    
    int sel1 = min(sel_start, sel_end), sel2 = max(sel_start, sel_end);
    int sel_size = sel2 - sel1;
    
    if(keycode == ALLEGRO_KEY_LEFT && !ctrl && !shift) {
        //Left arrow - move cursor left.
        if(sel_size) {
            cursor = sel1;
            sel_start = sel_end = cursor;
        } else if(cursor && text.size()) {
            //Go one to the left anyway.
            //This'll stop it from getting stuck on spaces.
            cursor--;
            
            if(ctrl) { //Whole word.
                while(cursor && text[cursor - 1] != ' ') {
                    cursor--;
                }
            }
            sel_start = sel_end = cursor;
        }
        
    } else if(keycode == ALLEGRO_KEY_RIGHT && !ctrl && !shift) {
        //Right arrow - move cursor right.
        if(sel_size) {
            cursor = sel2;
            sel_start = sel_end = cursor;
        } else if(cursor < text.size() && text.size()) {
            cursor++; //Go one to the right anyway.
            
            if(ctrl) {
                while(cursor < text.size() && text[cursor - 1] != ' ') {
                    cursor++;
                }
            }
        }
        
    } else if(keycode == ALLEGRO_KEY_HOME && !ctrl && !shift) {
        //Home - place cursor at beginning.
        cursor = 0;
        sel_start = sel_end = cursor;
        
    } else if(keycode == ALLEGRO_KEY_END && !ctrl && !shift) {
        //End - place cursor at end.
        cursor = text.size();
        sel_start = sel_end = cursor;
        
    } else if(keycode == ALLEGRO_KEY_BACKSPACE) {
        //Backspace - delete character before cursor.
        if(text.size()) { //If there's actually something written
            if(sel_size) {
                text.erase(sel1, sel2 - sel1);
                cursor = sel1;
                sel_start = sel_end = cursor;
            } else if(cursor) {
                unsigned short cursor_start = cursor;
                cursor--;
                
                if(ctrl) { //Whole word.
                    while(cursor && text[cursor - 1] != ' ') {
                        cursor--;
                    }
                }
                
                text.erase(cursor, cursor_start - cursor);
            }
            call_change_handler();
        }
        
    } else if(keycode == ALLEGRO_KEY_DELETE) {
        //Delete - delete character after cursor.
        if(text.size()) {
            //If there's actually something written.
            if(sel_size) {
                text.erase(sel1, sel2 - sel1);
                cursor = sel1;
                sel_start = sel_end = cursor;
            } else if(cursor < text.size()) {
                unsigned short cursor_start = cursor;
                cursor++;
                
                if(ctrl) {
                    while(cursor < text.size() && text[cursor - 1] != ' ') {
                        cursor++;
                    }
                }
                
                text.erase(cursor_start, cursor - cursor_start);
                cursor = cursor_start;
            }
            call_change_handler();
        }
        
    } else if(keycode == ALLEGRO_KEY_TAB && !ctrl) {
        //Tab - switch to the next textbox in the parent.
        
        if(parent) {
            size_t
            next_tab_index = UINT_MAX,
            prev_tab_index = 0,
            shortest_tab_index = UINT_MAX,
            longest_tab_index = 0;
            widget* next_textbox = NULL;
            widget* prev_textbox = NULL;
            widget* first_textbox = NULL;
            widget* last_textbox = NULL;
            
            for(
                auto w = parent->widgets.begin();
                w != parent->widgets.end(); ++w
            ) {
                if(typeid(*w->second) == typeid(textbox)) {
                
                    if(w->second == this) continue;
                    
                    size_t i = ((textbox*) w->second)->tab_index;
                    if(i < shortest_tab_index) {
                        shortest_tab_index = i;
                        first_textbox = w->second;
                    }
                    if(i > longest_tab_index) {
                        longest_tab_index = i;
                        last_textbox = w->second;
                    }
                    if(i >= tab_index && i < next_tab_index) {
                        next_tab_index = i;
                        next_textbox = w->second;
                    }
                    if(i < tab_index && i >= prev_tab_index) {
                        prev_tab_index = i;
                        prev_textbox = w->second;
                    }
                }
            }
            widget* new_focus;
            if(!shift) new_focus = next_textbox ? next_textbox : first_textbox;
            else new_focus = prev_textbox ? prev_textbox : last_textbox;
            if(new_focus) {
                parent->lose_focus();
                parent->focused_widget = new_focus;
                textbox* t = (textbox*) new_focus;
                t->sel_start = 0; t->sel_end = t->text.size();
            }
        }
        
    } else if(keycode == ALLEGRO_KEY_A && ctrl) {
        sel_start = 0;
        sel_end = text.size();
        
    } else if(
        unichar > 0 && keycode != ALLEGRO_KEY_ESCAPE &&
        keycode != ALLEGRO_KEY_TAB
    ) {
    
        int char_to_enter = unichar;
        //Other key - insert the character.
        if(keycode == ALLEGRO_KEY_ENTER || keycode == ALLEGRO_KEY_PAD_ENTER) {
            if(enter_key_widget) {
                enter_key_widget->call_left_mouse_click_handler(0, 0);
                return;
            } else {
                if(!multi_line) return;
                char_to_enter = '\n';
            }
        }
        
        if(sel_size) {
            text.erase(sel1, sel2 - sel1);
            cursor = sel1;
            sel_start = sel_end = cursor;
        }
        if(!ctrl) {
            text.insert(cursor, 1, char_to_enter);
            cursor++;
            call_change_handler();
        }
    }
    
    //Update the text scroll to match the cursor.
    unsigned int cursor_x =
        al_get_text_width(style->text_font, text.substr(0, cursor).c_str());
    unsigned int text_width =
        al_get_text_width(style->text_font, text.c_str());
    unsigned int box_width = (x2 - x1) - 4;
    
    if(text_width > box_width) {
        if(box_width / 2 < cursor_x) scroll_x = cursor_x - box_width / 2;
        else scroll_x = 0;
        scroll_x = min(scroll_x, (int) (text_width - box_width));
    } else scroll_x = 0;
}


/* ----------------------------------------------------------------------------
 * Handles the mouse being clicked. Places the caret on the
 * correct place.
 */
void textbox::widget_on_mouse_down(const int button, const int x, const int) {
    if(button != 1) return;
    
    cursor = mouse_to_char(x);
    sel_start = cursor;
    sel_end = cursor;
}


/* ----------------------------------------------------------------------------
 * Handles the mouse being moved.
 * If the mouse button is clicked, it changes the selection.
 */
void textbox::widget_on_mouse_move(const int x, const int) {
    if(!mouse_clicking) return;
    
    sel_end = mouse_to_char(x);
}


/* ----------------------------------------------------------------------------
 * Ticks one frame worth of time. Used to control the cursor blinking.
 */
void textbox::widget_on_tick(const float time) {
    cursor_change_time_left -= time;
    if(cursor_change_time_left <= 0.0f) {
        cursor_change_time_left = CURSOR_CHANGE_INTERVAL;
        cursor_visible = !cursor_visible;
    }
}


/* ----------------------------------------------------------------------------
 * Returns the number of the position the caret should go at,
 * when the mouse is clicked on the given X coordinate.
 * Takes into account text scroll, widget position, etc.
 */
unsigned int textbox::mouse_to_char(const int mouse_x) {
    //Get the relative X, from the start of the text.
    int rel_x = mouse_x - x1 + scroll_x;
    for(size_t c = 0; c < text.size(); ++c) {
        int width =
            al_get_text_width(style->text_font, text.substr(0, c + 1).c_str());
        if(rel_x < width) {
            return c;
        }
    }
    return text.size();
}

}
