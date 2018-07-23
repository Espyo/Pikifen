/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
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
 * Creates an info spot mob.
 */
info_spot::info_spot(
    const point &pos, const float angle, const string &vars
) :
    mob(pos, spec_mob_types["Info spot"], angle, vars),
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


/* ----------------------------------------------------------------------------
 * Draw the info spot.
 */
void info_spot::draw_mob(bitmap_effect_manager* effect_manager) {
    bitmap_effect_manager effects;
    add_sector_brightness_bitmap_effect(&effects);
    
    draw_bitmap_with_effects(
        bmp_info_spot,
        pos, point(type->radius * 2, type->radius * 2),
        0, &effects
    );
}
