#ifndef LAFI_STYLE_INCLUDED
#define LAFI_STYLE_INCLUDED

#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "const.h"

namespace lafi {

/* ----------------------------------------------------------------------------
 * The style widgets have. This includes their main color,
 * a lighter and darker variant, some alternate colors, etc.
 * By having all widgets share the same style, they'll all
 * have a consistent color palette.
 */
class style {
public:
    ALLEGRO_COLOR bg_color;
    ALLEGRO_COLOR lighter_bg_color;
    ALLEGRO_COLOR darker_bg_color;

    ALLEGRO_COLOR fg_color;
    ALLEGRO_COLOR alt_color; //e.g. Background in selected textbox text.

    ALLEGRO_COLOR disabled_bg_color;
    ALLEGRO_COLOR lighter_disabled_bg_color;
    ALLEGRO_COLOR darker_disabled_bg_color;

    ALLEGRO_COLOR disabled_fg_color;
    ALLEGRO_COLOR disabled_alt_color;

    ALLEGRO_FONT* text_font;

    style(
        ALLEGRO_COLOR bg_color =
            al_map_rgb(DEF_STYLE_BG_R, DEF_STYLE_BG_G, DEF_STYLE_BG_B),
        ALLEGRO_COLOR fg_color =
            al_map_rgb(DEF_STYLE_FG_R, DEF_STYLE_FG_G, DEF_STYLE_FG_B),
        ALLEGRO_COLOR alt_color =
            al_map_rgb(DEF_STYLE_ALT_R, DEF_STYLE_ALT_G, DEF_STYLE_ALT_B),
        ALLEGRO_FONT* text_font =
            al_create_builtin_font()
    );
    style(style &s2);
    ~style();

    static ALLEGRO_COLOR lighten_color(ALLEGRO_COLOR color);
    static ALLEGRO_COLOR darken_color(ALLEGRO_COLOR color);
};

}

#endif //ifndef LAFI_STYLE_INCLUDED
