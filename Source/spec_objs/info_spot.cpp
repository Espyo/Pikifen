#include <allegro5/allegro_font.h>

#include "../functions.h"
#include "info_spot.h"
#include "../vars.h"

info_spot::info_spot(float x, float y, sector* sec, string text, bool opens_box)
    : mob(x, y, sec->floors[0].z, info_spot_mob_type, sec) {
    
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