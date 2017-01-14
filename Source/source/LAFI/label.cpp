#include "label.h"

namespace lafi {

/* ----------------------------------------------------------------------------
 * Creates a label.
 */
label::label(
    const int x1, const int y1, const int x2, const int y2, const string &text,
    const int text_flags, lafi::style* style, const unsigned char flags
) :
    widget(x1, y1, x2, y2, style, flags),
    text(text),
    text_flags(text_flags) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a label.
 */
label::label(const string &text, const int text_flags) :
    label(0, 0, 0, 0, text, text_flags) {
    
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

}
