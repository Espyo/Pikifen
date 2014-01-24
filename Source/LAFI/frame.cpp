#include "frame.h"

/*
 * Creates a frame given some parameters.
 */
lafi_frame::lafi_frame(int x1, int y1, int x2, int y2, lafi_style* style, unsigned char flags)
    : lafi_widget(x1, y1, x2, y2, style, flags) {
    
    normal_bitmap = NULL;
}

/*
 * Creates a frame by copying the info from another frame.
 */
lafi_frame::lafi_frame(lafi_frame &f2) : lafi_widget(f2) {
    normal_bitmap = NULL;
}

/*
 * Destroys a frame.
 */
lafi_frame::~lafi_frame() {
    if(normal_bitmap) al_destroy_bitmap(normal_bitmap);
}

/*
 * Renders the frame.
 */
void lafi_frame::render() {
    if(normal_bitmap) al_destroy_bitmap(normal_bitmap);
    
    unsigned int w = x2 - x1;
    unsigned int h = y2 - y1;
    
    normal_bitmap = al_create_bitmap(w, h);
    ALLEGRO_BITMAP* old_target_bitmap = al_get_target_bitmap();
    al_set_target_bitmap(normal_bitmap); {
        al_clear_to_color(style->bg_color);
        al_draw_line(0, 0.5, w - 1, 0.5, style->lighter_bg_color, 1);           //Top line, outermost.
        al_draw_line(1, 1.5, w - 2, 1.5, style->darker_bg_color, 1);            //Top line, innermost.
        al_draw_line(0.5, 0, 0.5, h - 1, style->lighter_bg_color, 1);           //Left line, outermost.
        al_draw_line(1.5, 1, 1.5, h - 2, style->darker_bg_color, 1);            //Left line, innermost.
        al_draw_line(1, h - 0.5, w, h - 0.5, style->darker_bg_color, 1);        //Bottom line, outermost.
        al_draw_line(2, h - 1.5, w - 1, h - 1.5, style->lighter_bg_color, 1);   //Bottom line, innermost.
        al_draw_line(w - 0.5, 1, w - 0.5, h, style->darker_bg_color, 1);        //Right line, outermost.
        al_draw_line(w - 1.5, 2, w - 1.5, h - 1, style->lighter_bg_color, 1);   //Right line, innermost.
    } al_set_target_bitmap(old_target_bitmap);
}

void lafi_frame::draw_self() {
    al_draw_bitmap(normal_bitmap, x1, y1, 0);
}