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
    text_font(text_font){
    
}


/* ----------------------------------------------------------------------------
 * Creates a style by copying the info from another style.
 */
style::style(const style &s2) :
    bg_color(s2.bg_color),
    lighter_bg_color(s2.lighter_bg_color),
    darker_bg_color(s2.darker_bg_color),
    fg_color(s2.fg_color),
    alt_color(s2.alt_color),
    disabled_bg_color(s2.disabled_bg_color),
    lighter_disabled_bg_color(s2.lighter_disabled_bg_color),
    darker_disabled_bg_color(s2.darker_disabled_bg_color),
    disabled_fg_color(s2.disabled_fg_color),
    disabled_alt_color(s2.disabled_alt_color),
    text_font(s2.text_font) {
    
}


/* ----------------------------------------------------------------------------
 * Destroys a style.
 */
style::~style() { }


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

}
