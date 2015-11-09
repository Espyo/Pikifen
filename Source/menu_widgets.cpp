/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Menu widgets.
 */

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "drawing.h"
#include "menu_widgets.h"
#include "vars.h"

menu_widget::menu_widget(const int x, const int y, const int w, const int h, function<void()> ch) :
    x(x),
    y(y),
    w(w),
    h(h),
    click_handler(ch),
    selected(false) {
    
    
}

bool menu_widget::mouse_on(const int mx, const int my) {
    return (
               mx >= x - w * 0.5 &&
               mx <= x + w * 0.5 &&
               my >= y - h * 0.5 &&
               my <= y + h * 0.5
           );
}

menu_button::menu_button(
    const int x, const int y, const int w, const int h, function<void()> ch,
    string t, ALLEGRO_FONT* f, const ALLEGRO_COLOR c
) :
    menu_widget(x, y, w, h, ch),
    text(t),
    font(f),
    text_color(c) {
    
    
}

void menu_button::draw() {
    if(!font) return;
    //al_draw_filled_rectangle(x - w * 0.5, y - h * 0.5, x + w * 0.5, y + h * 0.5, al_map_rgb(128, 64, 32));
    if(selected) {
        draw_sprite(
            bmp_icon, x - w * 0.5 + 16, y,
            16, 16
        );
        draw_sprite(
            bmp_icon, x + w * 0.5 - 16, y,
            16, 16
        );
    }
    draw_text_lines(
        font, text_color,
        x, y,
        ALLEGRO_ALIGN_CENTER, 1, text
    );
}
