#include "checkbox_box.h"
#include "const.h"

lafi_checkbox_box::lafi_checkbox_box(int x1, int y1, bool checked, lafi_style* style, unsigned char flags)
    : lafi_widget(x1, y1, x1 + LAFI_CHECKBOX_BOX_SIZE, y1 + LAFI_CHECKBOX_BOX_SIZE, style, flags) {
    unchecked_bitmap = NULL;
    checked_bitmap = NULL;
    
    this->checked = checked;
}

lafi_checkbox_box::~lafi_checkbox_box() {
    if(unchecked_bitmap) al_destroy_bitmap(unchecked_bitmap);
    if(checked_bitmap) al_destroy_bitmap(checked_bitmap);
}

void lafi_checkbox_box::render() {

    unsigned int w = x2 - x1;
    unsigned int h = y2 - y1;
    
    for(unsigned char b = 0; b < 2; b++) {
        ALLEGRO_BITMAP* bm;
        if(b == 0) bm = unchecked_bitmap; else bm = checked_bitmap;
        
        if(bm) al_destroy_bitmap(bm);
        bm = al_create_bitmap(w, h);
        ALLEGRO_BITMAP* old_target_bitmap = al_get_target_bitmap();
        al_set_target_bitmap(bm); {
        
            al_clear_to_color(style->bg_color);
            al_draw_line(0, 0.5, w - 1, 0.5, style->darker_bg_color, 1);       //Top line.
            al_draw_line(0.5, 0, 0.5, h - 1, style->darker_bg_color, 1);       //Left line.
            al_draw_line(1, h - 0.5, w, h - 0.5, style->lighter_bg_color, 1);  //Bottom line.
            al_draw_line(w - 0.5, 1, w - 0.5, h, style->lighter_bg_color, 1);  //Right line.
            
            if(b == 1) {
                al_draw_line(2.5, 6.5, 5.5, 9.5, style->fg_color, 3); //Southeast-going line.
                al_draw_line(3.5, 9.5, 10, 3, style->fg_color, 3);    //Northeast-going line.
            }
            
        } al_set_target_bitmap(old_target_bitmap);
        
        if(b == 0) unchecked_bitmap = bm; else checked_bitmap = bm;
    }
}

void lafi_checkbox_box::draw_self() {
    ALLEGRO_BITMAP* bm;
    if(!checked) bm = unchecked_bitmap; else bm = checked_bitmap;
    al_draw_bitmap(bm, x1, y1, 0);
}