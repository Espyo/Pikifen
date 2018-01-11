#include "button.h"

namespace lafi {
	
const float button::OFFSET_START_DELAY = 2.0f;
const float button::OFFSET_RESET_DELAY = 2.0f;
const float button::OFFSET_SPEED = 65.0f;


/* ----------------------------------------------------------------------------
 * Creates a button given some parameters.
 */
button::button(
    const int x1, const int y1, const int x2, const int y2,
    const string &text, const string &description, ALLEGRO_BITMAP* icon,
    lafi::style* style, const unsigned char flags
) :
    widget(x1, y1, x2, y2, style, flags),
    offset(0),
    offset_start_time_left(OFFSET_START_DELAY),
    offset_reset_time_left(OFFSET_RESET_DELAY),
    text(text),
    icon(icon),
    autoscroll(false) {
    
    this->description = description;
}

/* ----------------------------------------------------------------------------
 * Creates a button given some parameters.
 */
button::button(
    const string &text, const string &description, ALLEGRO_BITMAP* icon
) :
    widget(),
    offset(0),
    offset_start_time_left(OFFSET_START_DELAY),
    offset_reset_time_left(OFFSET_RESET_DELAY),
    text(text),
    icon(icon),
    autoscroll(false) {
    
    this->description = description;
}


/* ----------------------------------------------------------------------------
 * Destroys a button.
 */
button::~button() { }


/* ----------------------------------------------------------------------------
 * Draws the button. It' just a rectangle with a fancy border;
 * the latter is drawn one line at a time.
 */
void button::draw_self() {
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
    //Top line, outermost.
    draw_line(this, DRAW_LINE_TOP,    0, 1, 0, top_color);
    //Top line, innermost.
    draw_line(this, DRAW_LINE_TOP,    0, 2, 1, top_color);
    //Left line, outermost.
    draw_line(this, DRAW_LINE_LEFT,   0, 1, 0, top_color);
    //Left line, innermost.
    draw_line(this, DRAW_LINE_LEFT,   0, 2, 1, top_color);
    //Bottom line, outermost.
    draw_line(this, DRAW_LINE_BOTTOM, 1, 0, 0, bottom_color);
    //Bottom line, innermost.
    draw_line(this, DRAW_LINE_BOTTOM, 2, 0, 1, bottom_color);
    //Right line, outermost.
    draw_line(this, DRAW_LINE_RIGHT,  1, 0, 0, bottom_color);
    //Right line, innermost.
    draw_line(this, DRAW_LINE_RIGHT,  2, 0, 1, bottom_color);
    
    //This is the center of the text, not top left. Also, relative coordinates.
    signed short final_text_y = 0;
    //Top left of the icon.
    signed short final_icon_y = 0;
    
    if(icon && text.size()) {
        //If there's an icon and text.
        unsigned short total_height =
            al_get_bitmap_height(icon) +
            al_get_font_line_height(style->text_font) + 2;
        //The icon goes to the top of the 2.
        final_icon_y = h / 2 - total_height / 2;
        //The text uses the same base y as the icon, except lowered, obviously.
        final_text_y =
            final_icon_y + al_get_bitmap_height(icon) +
            al_get_font_line_height(style->text_font) / 2 + 2;
            
    } else if(icon) {    //Icon, but no text.
        final_icon_y = h / 2 - al_get_bitmap_height(icon) / 2;
        
    } else if(!icon && text.size()) {    //Text, but no icon.
        final_text_y = h / 2;
    }
    
    if(icon) {
        al_draw_tinted_bitmap(
            icon,
            get_fg_color(),
            x1 + (w / 2 - al_get_bitmap_width(icon) / 2),
            y1 + final_icon_y,
            0);
    }
    
    if(text.size()) {
        int text_x;
        unsigned int text_w = al_get_text_width(style->text_font, text.c_str());
        int align;
        if(text_w <= w) {
            text_x = x1 + (w / 2);
            align = ALLEGRO_ALIGN_CENTER;
        } else {
            text_x = x1;
            text_x -= offset;
            align = ALLEGRO_ALIGN_LEFT;
        }
        int text_y = y1 + final_text_y;
        
        draw_text_lines(
            style->text_font,
            get_fg_color(),
            text_x, text_y,
            align,
            true,
            text
        );
    }
}

/* ----------------------------------------------------------------------------
 * Ticks one frame worth of time. This is used for the auto-scrolling.
 */
void button::widget_on_tick(const float time) {
    if(!autoscroll || !style->text_font) return;
    
    float text_w = al_get_text_width(style->text_font, text.c_str());
    float text_x2 = x1 - offset + text_w;
    
    if(text != prev_text) {
        //Reset everything.
        prev_text = text;
        offset = 0;
        offset_start_time_left = OFFSET_START_DELAY;
        offset_reset_time_left = OFFSET_RESET_DELAY;
    }
    
    if(offset == 0 && text_x2 <= x2) {
        //We don't need to scroll!
        return;
    }
    
    if(text_x2 > x2) {
        //Text needs to scroll left.
        if(offset_start_time_left > 0) {
            offset_start_time_left -= time;
        } else {
            offset += OFFSET_SPEED * time;
        }
        offset_reset_time_left = OFFSET_RESET_DELAY;
        
    } else {
        //Text reached the end.
        if(offset_reset_time_left > 0) {
            offset_reset_time_left -= time;
        } else {
            offset = 0;
        }
        offset_start_time_left = OFFSET_START_DELAY;
    }
}

}
