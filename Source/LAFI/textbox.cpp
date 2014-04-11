#include <limits.h>

#include "../functions.h"
#include "textbox.h"

size_t lafi_textbox::cur_tab_index = 0;

/*
 * Creates a textbox given some parameters.
 */
lafi_textbox::lafi_textbox(int x1, int y1, int x2, int y2, string text, lafi_style* style, unsigned char flags)
    : lafi_widget(x1, y1, x2, y2, style, flags) {
    
    this->text = text;
    editable = true;
    cursor = sel_start = sel_end = 0;
    multi_line = false;
    change_handler = NULL;
    scroll_x = 0;
    enter_key_widget = NULL;
    
    tab_index = cur_tab_index++;
}

/*
 * Creates a textbox by copying the info from another textbox.
 */
lafi_textbox::lafi_textbox(lafi_textbox &t2) : lafi_widget(t2) {
    text = t2.text;
    editable = t2.editable;
    cursor = sel_start = sel_end = 0;
    multi_line = false;
    change_handler = NULL;
    scroll_x = 0;
    enter_key_widget = NULL;
    tab_index = cur_tab_index++;
}

/*
 * Destroys a textbox.
 */
lafi_textbox::~lafi_textbox() {}

void lafi_textbox::draw_self() {
    al_draw_filled_rectangle(x1, y1, x2, y2, get_bg_color());
    lafi_draw_line(this, LAFI_DRAW_LINE_TOP,    0, 1, 0, get_darker_bg_color());  //Top line.
    lafi_draw_line(this, LAFI_DRAW_LINE_LEFT,   0, 1, 0, get_darker_bg_color());  //Left line.
    lafi_draw_line(this, LAFI_DRAW_LINE_BOTTOM, 1, 0, 0, get_lighter_bg_color()); //Bottom line.
    lafi_draw_line(this, LAFI_DRAW_LINE_RIGHT,  1, 0, 0, get_lighter_bg_color()); //Right line.
    
    if(style->text_font) {
        int ocrx, ocry, ocrw, ocrh;
        al_get_clipping_rectangle(&ocrx, &ocry, &ocrw, &ocrh);
        al_set_clipping_rectangle(x1, y1, (x2 - x1), (y2 - y1)); {
            int text_start = x1 + 2 - scroll_x;
            
            if(parent->focused_widget == this) {
                al_draw_filled_rectangle(
                    text_start + al_get_text_width(style->text_font, text.substr(0, min(sel_start, sel_end)).c_str()),
                    y1 + 2,
                    text_start + al_get_text_width(style->text_font, text.substr(0, max(sel_start, sel_end)).c_str()),
                    y2 - 2,
                    get_alt_color()
                );
            }
            
            draw_text_lines(style->text_font, get_fg_color(), text_start, (y2 + y1) / 2, 0, 1, text);
            
            unsigned int cursor_x = al_get_text_width(style->text_font, text.substr(0, cursor).c_str());
            
            if(parent->focused_widget == this) {
                al_draw_line(
                    x1 + cursor_x + 1.5 - scroll_x,
                    y1 + 2,
                    x1 + cursor_x + 1.5 - scroll_x,
                    y2 - 2,
                    get_alt_color(),
                    1);
            }
        } al_set_clipping_rectangle(ocrx, ocry, ocrw, ocrh);
    }
}

//Calls the function that handles a change of the text.
void lafi_textbox::call_change_handler() { if(change_handler) change_handler(this); }

void lafi_textbox::widget_on_key_char(int keycode, int unichar, unsigned int modifiers) {
    bool ctrl = (modifiers & ALLEGRO_KEYMOD_CTRL) || (modifiers & ALLEGRO_KEYMOD_COMMAND);
    
    if(cursor > text.size()) cursor = 0; //If the text is somehow changed.
    
    int sel1 = min(sel_start, sel_end), sel2 = max(sel_start, sel_end);
    int sel_size = sel2 - sel1;
    
    if(keycode == ALLEGRO_KEY_LEFT && modifiers == 0) { //Left arrow - move cursor left.
        if(sel_size) {
            cursor = sel1;
            sel_start = sel_end = cursor;
        } else if(cursor && text.size()) {
            cursor--;   //Go one to the left anyway. This'll stop it from getting stuck on spaces.
            
            if(ctrl) {  //Whole word.
                while(cursor && text[cursor - 1] != ' ') {
                    cursor--;
                }
            }
            sel_start = sel_end = cursor;
        }
        
    } else if(keycode == ALLEGRO_KEY_RIGHT && modifiers == 0) { //Right arrow - move cursor right.
        if(sel_size) {
            cursor = sel2;
            sel_start = sel_end = cursor;
        } else if(cursor < text.size() && text.size()) {
            cursor++;   //Go one to the right anyway.
            
            if(ctrl) {
                while(cursor < text.size() && text[cursor - 1] != ' ') {
                    cursor++;
                }
            }
        }
        
    } else if(keycode == ALLEGRO_KEY_HOME && modifiers == 0) { //Home - place cursor at beginning.
        cursor = 0;
        sel_start = sel_end = cursor;
        
    } else if(keycode == ALLEGRO_KEY_END && modifiers == 0) {  //End - place cursor at end.
        cursor = text.size();
        sel_start = sel_end = cursor;
        
    } else if(keycode == ALLEGRO_KEY_BACKSPACE) { //Backspace - delete character before cursor.
        if(text.size()) { //If there's actually something written
            if(sel_size) {
                text.erase(sel1, sel2 - sel1);
                cursor = sel1;
                sel_start = sel_end = cursor;
            } else if(cursor) {
                unsigned short cursor_start = cursor;
                cursor--;
                
                if(ctrl) {  //Whole word.
                    while(cursor && text[cursor - 1] != ' ') {
                        cursor--;
                    }
                }
                
                text.erase(cursor, cursor_start - cursor);
            }
            call_change_handler();
        }
        
    } else if(keycode == ALLEGRO_KEY_DELETE) { //Delete - delete character after cursor.
        if(text.size()) { //If there's actually something written.
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
        
    } else if(keycode == ALLEGRO_KEY_TAB && modifiers == 0) { //Tab - switch to the next textbox in the parent.
        size_t
        next_tab_index = UINT_MAX,
        prev_tab_index = 0,
        shortest_tab_index = UINT_MAX,
        longest_tab_index = 0;
        lafi_widget* next_textbox = NULL, *prev_textbox = NULL, *first_textbox = NULL, *last_textbox = NULL;
        
        bool backwards = modifiers & ALLEGRO_KEYMOD_SHIFT;
        
        if(parent) {
            for(auto w = parent->widgets.begin(); w != parent->widgets.end(); w++) {
                if(typeid(*w->second) == typeid(lafi_textbox)) {
                
                    size_t i = ((lafi_textbox*) w->second)->tab_index;
                    if(i < shortest_tab_index) {
                        shortest_tab_index = i;
                        first_textbox = w->second;
                    }
                    if(i > longest_tab_index) {
                        longest_tab_index = i;
                        last_textbox = w->second;
                    }
                    if(i > tab_index && i < next_tab_index) {
                        next_tab_index = i;
                        next_textbox = w->second;
                    }
                    if(i < tab_index && i > prev_tab_index) {
                        prev_tab_index = i;
                        prev_textbox = w->second;
                    }
                }
            }
            lafi_widget* new_focus;
            if(!backwards) new_focus = next_textbox ? next_textbox : first_textbox;
            else new_focus = prev_textbox ? prev_textbox : last_textbox;
            if(new_focus) {
                parent->lose_focus();
                parent->focused_widget = new_focus;
                lafi_textbox* t = (lafi_textbox*) new_focus;
                t->sel_start = 0; t->sel_end = t->text.size();
            }
        }
        
    } else if(keycode == ALLEGRO_KEY_A && ctrl) {
        sel_start = 0;
        sel_end = text.size();
        
    } else if(unichar > 0 && keycode != ALLEGRO_KEY_ESCAPE && keycode != ALLEGRO_KEY_TAB) { //Other key - insert the character.
        if(keycode == ALLEGRO_KEY_ENTER || keycode == ALLEGRO_KEY_PAD_ENTER) {
            if(enter_key_widget) {
                enter_key_widget->call_left_mouse_click_handler(0, 0);
                return;
            } else {
                if(!multi_line) return;
                else unichar = '\n';
            }
        }
        
        if(sel_size) {
            text.erase(sel1, sel2 - sel1);
            cursor = sel1;
            sel_start = sel_end = cursor;
        }
        if(
            modifiers == 0 || modifiers & ALLEGRO_KEYMOD_ACCENT1 || modifiers & ALLEGRO_KEYMOD_ACCENT2 ||
            modifiers & ALLEGRO_KEYMOD_ACCENT3 || modifiers & ALLEGRO_KEYMOD_ACCENT4 || modifiers & ALLEGRO_KEYMOD_CAPSLOCK ||
            modifiers & ALLEGRO_KEYMOD_SHIFT
        ) {
            text.insert(cursor, 1, unichar);
            cursor++;
            call_change_handler();
        }
    }
    
    //Update the text scroll to match the cursor.
    unsigned int cursor_x = al_get_text_width(style->text_font, text.substr(0, cursor).c_str());
    unsigned int text_width = al_get_text_width(style->text_font, text.c_str());
    unsigned int box_width = (x2 - x1) - 4;
    
    if(text_width > box_width) {
        if(box_width / 2 < cursor_x) scroll_x = cursor_x - box_width / 2;
        else scroll_x = 0;
        scroll_x = min(scroll_x, (int) (text_width - box_width));
    } else scroll_x = 0;
}

void lafi_textbox::widget_on_mouse_down(int button, int x, int) {
    if(button != 1) return;
    
    cursor = mouse_to_char(x);
    sel_start = cursor;
    sel_end = cursor;
}

void lafi_textbox::widget_on_mouse_move(int x, int) {
    if(!mouse_clicking) return;
    
    sel_end = mouse_to_char(x);
}

unsigned int lafi_textbox::mouse_to_char(int mouse_x) {
    //Get the relative X, from the start of the text.
    int rel_x = mouse_x - x1 + scroll_x;
    for(size_t c = 0; c < text.size(); c++) {
        int width = al_get_text_width(style->text_font, text.substr(0, c + 1).c_str());
        if(rel_x < width) {
            return c;
        }
    }
    return text.size();
}