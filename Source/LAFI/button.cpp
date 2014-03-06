#include "button.h"
#include "../functions.h"

/*
 * Creates a button given some parameters.
 */
lafi_button::lafi_button(int x1, int y1, int x2, int y2, string text, string description, ALLEGRO_BITMAP* icon, lafi_style* style, unsigned char flags)
    : lafi_widget(x1, y1, x2, y2, style, flags) {
    
    this->text = text;
    this->icon = icon;
    this->description = description;
    normal_bitmap = NULL;
    clicking_bitmap = NULL;
}

/*
 * Destroys a button.
 */
lafi_button::~lafi_button() {
    if(normal_bitmap) al_destroy_bitmap(normal_bitmap);
    if(clicking_bitmap) al_destroy_bitmap(clicking_bitmap);
}

/*
 * Renders the button.
 */
void lafi_button::render() {
    if(normal_bitmap) al_destroy_bitmap(normal_bitmap);
    normal_bitmap = create_button_bitmap(style->lighter_bg_color, style->darker_bg_color);
    if(clicking_bitmap) al_destroy_bitmap(clicking_bitmap);
    clicking_bitmap = create_button_bitmap(style->darker_bg_color, style->lighter_bg_color);
}

ALLEGRO_BITMAP* lafi_button::create_button_bitmap(ALLEGRO_COLOR top_color, ALLEGRO_COLOR bottom_color) {
    ALLEGRO_BITMAP* bitmap_to_create = NULL;
    
    unsigned int w = x2 - x1;
    unsigned int h = y2 - y1;
    
    bitmap_to_create = al_create_bitmap(w, h);
    ALLEGRO_BITMAP* old_target_bitmap = al_get_target_bitmap();
    al_set_target_bitmap(bitmap_to_create); {
    
        al_clear_to_color(style->bg_color);
        al_draw_line(0, 0.5, w - 1, 0.5, top_color, 1);  //Top line, outermost.
        al_draw_line(0, 1.5, w - 2, 1.5, top_color, 1);  //Top line, innermost.
        al_draw_line(0.5, 0, 0.5, h - 1, top_color, 1);  //Left line, outermost.
        al_draw_line(1.5, 0, 1.5, h - 2, top_color, 1);  //Left line, innermost.
        al_draw_line(w, h - 0.5, 1, h - 0.5, bottom_color, 1);  //Bottom line, outermost.
        al_draw_line(w, h - 1.5, 2, h - 1.5, bottom_color, 1);  //Bottom line, innermost.
        al_draw_line(w - 0.5, h, w - 0.5, 1, bottom_color, 1);  //Right line, outermost.
        al_draw_line(w - 1.5, h, w - 1.5, 2, bottom_color, 1);  //Right line, innermost.
        
    } al_set_target_bitmap(old_target_bitmap);
    
    return bitmap_to_create;
}

void lafi_button::draw_self() {
    ALLEGRO_BITMAP* bitmap_to_draw;
    if(mouse_clicking && mouse_in) bitmap_to_draw = clicking_bitmap;
    else bitmap_to_draw = normal_bitmap;
    
    al_draw_bitmap(bitmap_to_draw, x1, y1, 0);
    
    unsigned int w = x2 - x1;
    unsigned int h = y2 - y1;
    signed short final_text_y = 0;      //This is the center of the text, not top left. Also, relative coordinates.
    signed short final_icon_y = 0;      //Top left of the icon.
    
    if(icon && text.size()) {       //If there's an icon and text.
        unsigned short total_height = al_get_bitmap_height(icon) + al_get_font_line_height(style->text_font) + 2;
        //The icon goes to the top of the 2.
        final_icon_y = h / 2 - total_height / 2;
        //The text uses the same base y as the icon, except lowered, obviously.
        final_text_y = final_icon_y + al_get_bitmap_height(icon) + al_get_font_line_height(style->text_font) / 2 + 2;
        
    } else if(icon) {    //Icon, but no text.
        final_icon_y = h / 2 - al_get_bitmap_height(icon) / 2;
        
    } else if(!icon && text.size()) {    //Text, but no icon.
        final_text_y = h / 2;
    }
    
    if(icon) {
        al_draw_bitmap(
            icon,
            x1 + (w / 2 - al_get_bitmap_width(icon) / 2),
            y1 + final_icon_y,
            0);
    }
    
    if(text.size()) {
        draw_text_lines(
            style->text_font,
            style->fg_color,
            x1 + (w / 2),
            y1 + final_text_y,
            ALLEGRO_ALIGN_CENTRE,
            true,
            text);
    }
}