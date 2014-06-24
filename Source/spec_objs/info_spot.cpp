/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Info spot class and info-spot related functions.
 */

#include <allegro5/allegro_font.h>

#include "../functions.h"
#include "info_spot.h"
#include "../vars.h"

/* ----------------------------------------------------------------------------
 * Creates an info spot.
 */
info_spot::info_spot(float x, float y, string text, bool opens_box)
    : mob(x, y, special_mob_types["Info spot"], 0, "") {
    
    this->text = text;
    this->opens_box = opens_box;
    text_w = 0;
    
    vector<string> lines = split(text, "\n");
    size_t n_lines = lines.size();
    for(size_t l = 0; l < n_lines; l++) {
        unsigned int line_w = al_get_text_width(font, lines[l].c_str());
        if(line_w > text_w) text_w = line_w;
    }
}