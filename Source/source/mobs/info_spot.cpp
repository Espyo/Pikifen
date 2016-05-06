/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Info spot class and info-spot related functions.
 */

#include <allegro5/allegro_font.h>

#include "../drawing.h"
#include "../functions.h"
#include "info_spot.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates an info spot.
 */
info_spot::info_spot(
    const float x, const float y, const float angle, const string &vars
) :
    mob(x, y, spec_mob_types["Info spot"], angle, vars),
    text(get_var_value(vars, "text", "")),
    opens_box(s2b(get_var_value(vars, "opens_box", "0"))),
    text_w(0) {

    text = replace_all(text, "\\n", "\n");
    vector<string> lines = split(text, "\n");
    size_t n_lines = lines.size();
    for(size_t l = 0; l < n_lines; ++l) {
        unsigned int line_w = al_get_text_width(font_main, lines[l].c_str());
        if(line_w > text_w) text_w = line_w;
    }
}


void info_spot::draw() {
    draw_sprite(
        bmp_info_spot,
        x, y,
        type->radius * 2, type->radius * 2,
        0, map_gray(get_sprite_brightness(this))
    );
}
