#define _USE_MATH_DEFINES

#include <math.h>

#include "const.h"
#include "radio_button_button.h"

lafi_radio_button_button::lafi_radio_button_button(int x1, int y1, bool selected, lafi_style* style, unsigned char flags)
    : lafi_widget(x1, y1, x1 + LAFI_RADIO_BUTTON_BUTTON_SIZE, y1 + LAFI_RADIO_BUTTON_BUTTON_SIZE, style, flags) {
    this->selected = selected;
    
    selected_bitmap = unselected_bitmap = NULL;
}

lafi_radio_button_button::~lafi_radio_button_button() {
    if(unselected_bitmap) al_destroy_bitmap(unselected_bitmap);
    if(selected_bitmap) al_destroy_bitmap(selected_bitmap);
}

void lafi_radio_button_button::render() {

    unsigned int w = x2 - x1;
    unsigned int h = y2 - y1;
    
    for(unsigned char b = 0; b < 2; b++) {
        ALLEGRO_BITMAP* bm;
        if(b == 0) bm = unselected_bitmap; else bm = selected_bitmap;
        
        if(bm) al_destroy_bitmap(bm);
        bm = al_create_bitmap(w, h);
        ALLEGRO_BITMAP* old_target_bitmap = al_get_target_bitmap();
        al_set_target_bitmap(bm); {
        
            al_draw_filled_circle(w / 2, h / 2, w / 2, style->bg_color);
            al_draw_arc(w / 2, h / 2, w / 2, -M_PI * 0.25, M_PI, style->lighter_bg_color, 1);
            al_draw_arc(w / 2, h / 2, w / 2, M_PI * 0.75, M_PI, style->darker_bg_color, 1);
            
            if(b == 1) {
                al_draw_filled_circle(w / 2, h / 2, w * 0.25, style->fg_color);
            }
            
        } al_set_target_bitmap(old_target_bitmap);
        
        if(b == 0) unselected_bitmap = bm; else selected_bitmap = bm;
    }
}

void lafi_radio_button_button::draw_self() {
    ALLEGRO_BITMAP* bm;
    if(!selected) bm = unselected_bitmap; else bm = selected_bitmap;
    al_draw_bitmap(bm, x1, y1, 0);
}