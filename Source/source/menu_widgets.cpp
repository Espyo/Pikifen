/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Menu widgets.
 */

#include <algorithm>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "drawing.h"
#include "menu_widgets.h"
#include "vars.h"

const float menu_widget::ICON_SWAY_DELTA = 1.0f;
const float menu_widget::ICON_SWAY_TIME_SCALE = 1.0f;
const float menu_widget::JUICY_GROW_DELTA = 0.05f;
const float menu_widget::JUICY_GROW_DURATION = 0.3f;

/* ----------------------------------------------------------------------------
 * Creates a menu widget.
 */
menu_widget::menu_widget(
    const int x, const int y, const int w, const int h,
    function<void()> click_handler
) :
    x(x),
    y(y),
    w(w),
    h(h),
    click_handler(click_handler),
    selected(false),
    enabled(true),
    juicy_grow_time_left(0) {


}


/* ----------------------------------------------------------------------------
 * Returns whether or not the mouse cursor is on top of this widget.
 */
bool menu_widget::mouse_on(const int mx, const int my) {
    return (
               mx >= x - w * 0.5 &&
               mx <= x + w * 0.5 &&
               my >= y - h * 0.5 &&
               my <= y + h * 0.5
           );
}


/* ----------------------------------------------------------------------------
 * Runs the widget's "click" code, used when the player clicks on the widget,
 * if possible.
 */
void menu_widget::click() {
    if(!enabled) return;
    on_click();
    if(click_handler) click_handler();
}


/* ----------------------------------------------------------------------------
 * Ticks an in-game frame worth of logic.
 */
void menu_widget::tick(const float time) {
    if(juicy_grow_time_left > 0) {
        juicy_grow_time_left = max(0.0f, juicy_grow_time_left - time);
    }
}


/* ----------------------------------------------------------------------------
 * Begins the growth animation process.
 */
void menu_widget::start_juicy_grow() {
    juicy_grow_time_left = JUICY_GROW_DURATION;
}


/* ----------------------------------------------------------------------------
 * Creates a clickable button widget.
 */
menu_button::menu_button(
    const int x, const int y, const int w, const int h,
    function<void()> click_handler, string text, ALLEGRO_FONT* font,
    const ALLEGRO_COLOR &color, const int align
) :
    menu_widget(x, y, w, h, click_handler),
    text(text),
    font(font),
    text_color(color),
    text_align(align) {


}

/* ----------------------------------------------------------------------------
 * Draws a button widget.
 */
void menu_button::draw(const float time_spent) {
    if(!font || !enabled) return;
    if(selected) {
        draw_sprite(
            bmp_icon, x - w * 0.5 + 16, y,
            16, 16, sin(time_spent * ICON_SWAY_TIME_SCALE) * ICON_SWAY_DELTA
        );
        draw_sprite(
            bmp_icon, x + w * 0.5 - 16, y,
            16, 16, sin(time_spent * ICON_SWAY_TIME_SCALE) * ICON_SWAY_DELTA
        );
    }

    int text_x = x;
    if(text_align == ALLEGRO_ALIGN_LEFT) {
        text_x = x - w * 0.5 + 32;
    } else if(text_align == ALLEGRO_ALIGN_RIGHT) {
        text_x = x + w * 0.5 - 32;
    }

    draw_text_lines(
        font, text_color,
        text_x, y,
        text_align, 1, text
    );
}


void menu_button::on_click() {}
bool menu_button::is_clickable() { return enabled; }


/* ----------------------------------------------------------------------------
 * Creates a checkbox widget.
 */
menu_checkbox::menu_checkbox(
    const int x, const int y, const int w, const int h,
    function<void()> click_handler, string text, ALLEGRO_FONT* font,
    const ALLEGRO_COLOR &color, const int align
) :
    menu_widget(x, y, w, h, click_handler),
    text(text),
    font(font),
    text_color(color),
    text_align(align),
    checked(false) {


}


/* ----------------------------------------------------------------------------
 * Draws a checkbox.
 */
void menu_checkbox::draw(const float time_spent) {
    if(!font || !enabled) return;
    if(selected) {
        draw_sprite(
            bmp_icon, x - w * 0.5 + 16, y,
            16, 16, sin(time_spent * ICON_SWAY_TIME_SCALE) * ICON_SWAY_DELTA
        );
        draw_sprite(
            bmp_icon, x + w * 0.5 - 16, y,
            16, 16, sin(time_spent * ICON_SWAY_TIME_SCALE) * ICON_SWAY_DELTA
        );
    }

    int text_x = x;
    if(text_align == ALLEGRO_ALIGN_LEFT) {
        text_x = x - w * 0.5 + 32;
    } else if(text_align == ALLEGRO_ALIGN_RIGHT) {
        text_x = x + w * 0.5 - 32;
    }

    draw_text_lines(
        font, text_color,
        text_x, y,
        text_align, 1, text
    );
    if(checked) {
        draw_sprite(
            bmp_checkbox_check, x + w * 0.5 - 40, y,
            32, -1
        );
    }
}


void menu_checkbox::on_click() { checked = !checked; }
bool menu_checkbox::is_clickable() { return enabled; }


/* ----------------------------------------------------------------------------
 * Creates a text widget.
 */
menu_text::menu_text(
    const int x, const int y, const int w, const int h, string text,
    ALLEGRO_FONT* font, const ALLEGRO_COLOR &color, const int align
) :
    menu_widget(x, y, w, h, nullptr),
    text(text),
    font(font),
    text_color(color),
    text_align(align) {


}


/* ----------------------------------------------------------------------------
 * Draws a text widget.
 */
void menu_text::draw(const float time_spent) {
    if(!font || !enabled) return;

    int text_x = x;
    if(text_align == ALLEGRO_ALIGN_LEFT) {
        text_x = x - w * 0.5 + 32;
    } else if(text_align == ALLEGRO_ALIGN_RIGHT) {
        text_x = x + w * 0.5 - 32;
    }

    float juicy_grow_amount =
        ease(
            EASE_UP_AND_DOWN,
            juicy_grow_time_left / JUICY_GROW_DURATION
        ) * JUICY_GROW_DELTA;

    draw_scaled_text(
        font, text_color,
        text_x, y,
        1.0 + juicy_grow_amount,
        1.0 + juicy_grow_amount,
        text_align, 1, text
    );
}


void menu_text::on_click() {}
bool menu_text::is_clickable() { return false; }
