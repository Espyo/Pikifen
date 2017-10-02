#include "label.h"

namespace lafi {

const float label::OFFSET_START_DELAY = 2.0f;
const float label::OFFSET_RESET_DELAY = 2.0f;
const float label::OFFSET_SPEED = 65.0f;

/* ----------------------------------------------------------------------------
 * Creates a label.
 */
label::label(
    const int x1, const int y1, const int x2, const int y2, const string &text,
    const int text_flags, const bool autoscroll,
    lafi::style* style, const unsigned char flags
) :
    widget(x1, y1, x2, y2, style, flags),
    offset(0),
    offset_start_time_left(OFFSET_START_DELAY),
    offset_reset_time_left(OFFSET_RESET_DELAY),
    text(text),
    text_flags(text_flags),
    autoscroll(autoscroll) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a label.
 */
label::label(const string &text, const int text_flags, const bool autoscroll) :
    widget(),
    offset(0),
    offset_start_time_left(OFFSET_START_DELAY),
    offset_reset_time_left(OFFSET_RESET_DELAY),
    text(text),
    text_flags(text_flags),
    autoscroll(autoscroll) {
    
}


/* ----------------------------------------------------------------------------
 * Destroys a label.
 */
label::~label() {}


/* ----------------------------------------------------------------------------
 * Draws a label. It draws a solid rectangle at the back, and then
 * draws the label's text.
 */
void label::draw_self() {
    al_draw_filled_rectangle(x1, y1, x2, y2, get_bg_color());
    if(style->text_font) {
        int text_x = 1;
        if(text_flags == ALLEGRO_ALIGN_CENTER) text_x = (x2 - x1) / 2;
        else if(text_flags == ALLEGRO_ALIGN_RIGHT) text_x = (x2 - x1) - 1;
        text_x += x1;
        text_x -= offset;
        
        al_draw_text(
            style->text_font,
            get_fg_color(),
            text_x,
            (y1 + y2) / 2 - al_get_font_line_height(style->text_font) / 2,
            text_flags,
            text.c_str()
        );
    }
}

/* ----------------------------------------------------------------------------
 * Ticks one frame worth of time. This is used for the auto-scrolling.
 */
void label::widget_on_tick(const float time) {
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
