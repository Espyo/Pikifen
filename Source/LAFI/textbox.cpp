#include "../functions.h"
#include "textbox.h"

/*
 * Creates a textbox given some parameters.
 */
lafi_textbox::lafi_textbox(int x1, int y1, int x2, int y2, string text, lafi_style* style, unsigned char flags)
    : lafi_widget(x1, y1, x2, y2, style, flags) {
    
    this->text = text;
    editable = true;
    cursor = 0;
    multi_line = false;
    change_handler = NULL;
    normal_bitmap = NULL;
    scroll_x = 0;
}

/*
 * Creates a textobx by copying the info from another textbox.
 */
lafi_textbox::lafi_textbox(lafi_textbox &t2) : lafi_widget(t2) {
    text = t2.text;
    editable = t2.editable;
    cursor = 0;
    multi_line = false;
    change_handler = NULL;
    normal_bitmap = NULL;
    scroll_x = 0;
}

/*
 * Destroys a textbox.
 */
lafi_textbox::~lafi_textbox() {
    if(normal_bitmap) al_destroy_bitmap(normal_bitmap);
}

/*
 * Renders the textbox.
 */
void lafi_textbox::render() {
    if(normal_bitmap) al_destroy_bitmap(normal_bitmap);
    
    unsigned int w = x2 - x1;
    unsigned int h = y2 - y1;
    
    normal_bitmap = al_create_bitmap(w, h);
    ALLEGRO_BITMAP* old_target_bitmap = al_get_target_bitmap();
    al_set_target_bitmap(normal_bitmap); {
    
        al_clear_to_color(style->bg_color);
        al_draw_line(0, 0.5, w - 1, 0.5, style->darker_bg_color, 1);  //Top line.
        al_draw_line(0.5, 0, 0.5, h - 1, style->darker_bg_color, 1);  //Left line.
        al_draw_line(1, h - 0.5, w, h - 0.5, style->lighter_bg_color, 1);  //Bottom line.
        al_draw_line(w - 0.5, 1, w - 0.5, h, style->lighter_bg_color, 1);  //Right line.
        
    } al_set_target_bitmap(old_target_bitmap);
}

void lafi_textbox::draw_self() {
    al_draw_bitmap(normal_bitmap, x1, y1, 0);
    
    if(style->text_font) {
        int ocrx, ocry, ocrw, ocrh;
        al_get_clipping_rectangle(&ocrx, &ocry, &ocrw, &ocrh);
        al_set_clipping_rectangle(x1, y1, (x2 - x1), (y2 - y1)); {
            draw_text_lines(style->text_font, style->fg_color, x1 + 2 - scroll_x, (y2 + y1) / 2, 0, 1, text);
            
            unsigned int cursor_x = al_get_text_width(style->text_font, text.substr(0, cursor).c_str());
            
            if(parent->focused_widget == this) {
                al_draw_line(
                    x1 + cursor_x + 1.5 - scroll_x,
                    y1 + 2,
                    x1 + cursor_x + 1.5 - scroll_x,
                    y2 - 2,
                    style->lighter_bg_color,
                    1);
            }
        } al_set_clipping_rectangle(ocrx, ocry, ocrw, ocrh);
    }
}

//Calls the function that handles a change of the text.
void lafi_textbox::call_change_handler() { if(change_handler) change_handler(this); }

void lafi_textbox::widget_on_key_char(int keycode, int unichar, unsigned int modifiers) {
    bool ctrl = (modifiers & ALLEGRO_KEYMOD_CTRL) || (modifiers & ALLEGRO_KEYMOD_COMMAND);
    
    if(keycode == ALLEGRO_KEY_LEFT) { //Left arrow - move cursor left.
        if(cursor && text.size()) {
            cursor--;   //Go one to the left anyway. This'll stop it from getting stuck on spaces.
            
            if(ctrl) {  //Whole word.
                while(cursor && text[cursor - 1] != ' ') {
                    cursor--;
                }
            }
        }
        
    } else if(keycode == ALLEGRO_KEY_RIGHT) { //Right arrow - move cursor right.
        if(cursor < text.size() && text.size()) {
            cursor++;   //Go one to the right anyway.
            
            if(ctrl) {
                while(cursor < text.size() && text[cursor - 1] != ' ') {
                    cursor++;
                }
            }
        }
        
    } else if(keycode == ALLEGRO_KEY_HOME) { //Home - place cursor at beginning.
        cursor = 0;
        
    } else if(keycode == ALLEGRO_KEY_END) {  //End - place cursor at end.
        cursor = text.size();
        
    } else if(keycode == ALLEGRO_KEY_BACKSPACE) { //Backspace - delete character before cursor.
        if(text.size() && cursor) { //If there's actually something written, and the cursor is not at the beginning.
            unsigned short cursor_start = cursor;
            cursor--;
            
            if(ctrl) {  //Whole word.
                while(cursor && text[cursor - 1] != ' ') {
                    cursor--;
                }
            }
            
            text.erase(cursor, cursor_start - cursor);
            call_change_handler();
        }
        
    } else if(keycode == ALLEGRO_KEY_DELETE) { //Delete - delete character after cursor.
        if(text.size() && cursor < text.size()) { //If there's actually something written, and the cursor is not at the end.
            unsigned short cursor_start = cursor;
            cursor++;
            
            if(ctrl) {
                while(cursor < text.size() && text[cursor - 1] != ' ') {
                    cursor++;
                }
            }
            
            text.erase(cursor_start, cursor - cursor_start);
            cursor = cursor_start;
            call_change_handler();
        }
        
    } else if(unichar > 0 && keycode != ALLEGRO_KEY_ESCAPE && keycode != ALLEGRO_KEY_TAB) { //Other key - insert the character.
        if(keycode == ALLEGRO_KEY_ENTER || keycode == ALLEGRO_KEY_PAD_ENTER) {
            if(!multi_line) return;
            else unichar = '\n';
        }
        text.insert(cursor, 1, unichar);
        cursor++;
        call_change_handler();
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