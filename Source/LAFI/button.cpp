#include "button.h"
#include "../functions.h"

/*
 * Creates a button given some parameters.
 */
lafi_button::lafi_button(int x1, int y1, int x2, int y2, string text, string description, ALLEGRO_BITMAP* icon, lafi_style* style, unsigned char flags)
    : lafi_widget(x1, y1, x2, y2, style, flags) {
    
    this->text = text;
    this->icon = icon;
    this->description = description;
}

/*
 * Destroys a button.
 */
lafi_button::~lafi_button() { }

void lafi_button::draw_self() {
    unsigned int w = x2 - x1;
    unsigned int h = y2 - y1;
    
    ALLEGRO_COLOR top_color, bottom_color;
    if(mouse_clicking && mouse_in) {
        top_color =    get_darker_bg_color();
        bottom_color = get_lighter_bg_color();
    } else {
        top_color =    get_lighter_bg_color();
        bottom_color = get_darker_bg_color();
    }
    
    al_draw_filled_rectangle(x1, y1, x2, y2, get_bg_color());
    lafi_draw_line(this, LAFI_DRAW_LINE_TOP,    0, 1, 0, top_color);    //Top line,    outermost.
    lafi_draw_line(this, LAFI_DRAW_LINE_TOP,    0, 2, 1, top_color);    //Top line,    innermost.
    lafi_draw_line(this, LAFI_DRAW_LINE_LEFT,   0, 1, 0, top_color);    //Left line,   outermost.
    lafi_draw_line(this, LAFI_DRAW_LINE_LEFT,   0, 2, 1, top_color);    //Left line,   innermost.
    lafi_draw_line(this, LAFI_DRAW_LINE_BOTTOM, 1, 0, 0, bottom_color); //Bottom line, outermost.
    lafi_draw_line(this, LAFI_DRAW_LINE_BOTTOM, 2, 0, 1, bottom_color); //Bottom line, innermost.
    lafi_draw_line(this, LAFI_DRAW_LINE_RIGHT,  1, 0, 0, bottom_color); //Right line,  outermost.
    lafi_draw_line(this, LAFI_DRAW_LINE_RIGHT,  2, 0, 1, bottom_color); //Right line,  innermost.
    
    signed short final_text_y = 0;      //This is the center of the text, not top left. Also, relative coordinates.
    signed short final_icon_y = 0;      //Top left of the icon.
    
    if(icon && text.size()) {       //If there's an icon and text.
        unsigned short total_height = al_get_bitmap_height(icon) + al_get_font_line_height(style->text_font) + 2;
        //The icon goes to the top of the 2.
        final_icon_y = h / 2 - total_height / 2;
        //The text uses the same base y as the icon, except lowered, obviously.
        final_text_y = final_icon_y + al_get_bitmap_height(icon) + al_get_font_line_height(style->text_font) / 2 + 2;
        
    } else if(icon) {    //Icon, but no text.
        final_icon_y = h / 2 - al_get_bitmap_height(icon) / 2;
        
    } else if(!icon && text.size()) {    //Text, but no icon.
        final_text_y = h / 2;
    }
    
    if(icon) {
        al_draw_bitmap(
            icon,
            x1 + (w / 2 - al_get_bitmap_width(icon) / 2),
            y1 + final_icon_y,
            0);
    }
    
    if(text.size()) {
        draw_text_lines(
            style->text_font,
            get_fg_color(),
            x1 + (w / 2),
            y1 + final_text_y,
            ALLEGRO_ALIGN_CENTRE,
            true,
            text);
    }
}