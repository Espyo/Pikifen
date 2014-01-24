#include "label.h"
#include "../functions.h"

lafi_label::lafi_label(int x1, int y1, int x2, int y2, string text, int text_flags, lafi_style* style, unsigned char flags)
    : lafi_widget(x1, y1, x2, y2, style, flags) {
    this->text = text;
    this->text_flags = text_flags;
    normal_bitmap = NULL;
}

lafi_label::~lafi_label() {
    if(normal_bitmap) al_destroy_bitmap(normal_bitmap);
}

void lafi_label::render() { }

void lafi_label::draw_self() {
    al_draw_filled_rectangle(x1, y1, x2, y2, style->bg_color);
    if(style->text_font) {
        int text_x = 1;
        if(text_flags == ALLEGRO_ALIGN_CENTER) text_x = (x2 - x1) / 2;
        else if(text_flags == ALLEGRO_ALIGN_RIGHT) text_x = (x2 - x1) - 1;
        text_x += x1;
        
        draw_text_lines(
            style->text_font,
            style->fg_color,
            text_x,
            y1 + (y2 - y1) / 2,
            text_flags,
            true,
            text
        );
    }
}