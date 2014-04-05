#include "style.h"

/*
 * Creates a style given some parameters.
 */
lafi_style::lafi_style(ALLEGRO_COLOR bg_color, ALLEGRO_COLOR fg_color, ALLEGRO_FONT* text_font) {
    this->bg_color = bg_color;
    this->lighter_bg_color = lighten_color(bg_color);
    this->darker_bg_color = darken_color(bg_color);
    this->fg_color = fg_color;
    this->text_font = text_font;
}

/*
 * Creates a style by copying the info from another style.
 */
lafi_style::lafi_style(lafi_style &s2) {
    bg_color = s2.bg_color;
    fg_color = s2.fg_color;
    text_font = s2.text_font;
    lighter_bg_color = s2.lighter_bg_color;
    darker_bg_color = s2.darker_bg_color;
}

/*
 * Destroys a style.
 */
lafi_style::~lafi_style() { }

/*
 * Returns a color that's ligther than the given color.
 */
ALLEGRO_COLOR lafi_style::lighten_color(ALLEGRO_COLOR color) {
    float indexes[4] = {
        static_cast<float>(color.r + LAFI_COLOR_SHIFT_DELTA),
        static_cast<float>(color.g + LAFI_COLOR_SHIFT_DELTA),
        static_cast<float>(color.b + LAFI_COLOR_SHIFT_DELTA),
        color.a
    };
    
    for(unsigned char i = 0; i < 3; i++) if(indexes[i] > 1) indexes[i] = 1;
    
    return al_map_rgba_f(indexes[0], indexes[1], indexes[2], indexes[3]);
}

/*
 * Returns a color that's darker than the given color.
 */
ALLEGRO_COLOR lafi_style::darken_color(ALLEGRO_COLOR color) {
    float indexes[4] = {
        static_cast<float>(color.r - LAFI_COLOR_SHIFT_DELTA),
        static_cast<float>(color.g - LAFI_COLOR_SHIFT_DELTA),
        static_cast<float>(color.b - LAFI_COLOR_SHIFT_DELTA),
        color.a
    };
    
    for(unsigned char i = 0; i < 3; i++) if(indexes[i] < 0) indexes[i] = 0;
    
    return al_map_rgba_f(indexes[0], indexes[1], indexes[2], indexes[3]);
}
