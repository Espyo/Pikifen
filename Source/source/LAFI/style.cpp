#include "style.h"

namespace lafi {

/* ----------------------------------------------------------------------------
 * Creates a style given some parameters.
 */
style::style(
    const ALLEGRO_COLOR bg_color, const ALLEGRO_COLOR fg_color,
    const ALLEGRO_COLOR alt_color, ALLEGRO_FONT* text_font
) :
    bg_color(bg_color),
    lighter_bg_color(lighten_color(bg_color)),
    darker_bg_color(darken_color(bg_color)),
    fg_color(fg_color),
    alt_color(alt_color),
    disabled_bg_color(darker_bg_color),
    lighter_disabled_bg_color(lighten_color(disabled_bg_color)),
    darker_disabled_bg_color(darken_color(disabled_bg_color)),
    disabled_fg_color(lighten_color(fg_color)),
    disabled_alt_color(darken_color(alt_color)),
    text_font(text_font) {
    
}


/* ----------------------------------------------------------------------------
 * Returns a color that's darker than the given color.
 */
ALLEGRO_COLOR style::darken_color(const ALLEGRO_COLOR color) {
    float indexes[4] = {
        static_cast<float>(color.r - COLOR_SHIFT_DELTA),
        static_cast<float>(color.g - COLOR_SHIFT_DELTA),
        static_cast<float>(color.b - COLOR_SHIFT_DELTA),
        color.a
    };
    
    for(unsigned char i = 0; i < 3; ++i) if(indexes[i] < 0) indexes[i] = 0;
    
    return al_map_rgba_f(indexes[0], indexes[1], indexes[2], indexes[3]);
}


/* ----------------------------------------------------------------------------
 * Returns a color that's ligther than the given color.
 */
ALLEGRO_COLOR style::lighten_color(const ALLEGRO_COLOR color) {
    float indexes[4] = {
        static_cast<float>(color.r + COLOR_SHIFT_DELTA),
        static_cast<float>(color.g + COLOR_SHIFT_DELTA),
        static_cast<float>(color.b + COLOR_SHIFT_DELTA),
        color.a
    };
    
    for(unsigned char i = 0; i < 3; ++i) if(indexes[i] > 1) indexes[i] = 1;
    
    return al_map_rgba_f(indexes[0], indexes[1], indexes[2], indexes[3]);
}

}
