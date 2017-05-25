/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
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
    const point &pos, const point &size,
    function<void()> click_handler
) :
    pos(pos),
    size(size),
    click_handler(click_handler),
    selected(false),
    enabled(true),
    juicy_grow_time_left(0) {
    
    
}


/* ----------------------------------------------------------------------------
 * Returns whether or not the mouse cursor is on top of this widget.
 */
bool menu_widget::mouse_on(const point &mc) {
    return
        (
            mc.x >= pos.x - size.x * 0.5 &&
            mc.x <= pos.x + size.x * 0.5 &&
            mc.y >= pos.y - size.y * 0.5 &&
            mc.y <= pos.y + size.y * 0.5
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
    const point &pos, const point &size,
    function<void()> click_handler, string text, ALLEGRO_FONT* font,
    const ALLEGRO_COLOR &color, const int align
) :
    menu_widget(pos, size, click_handler),
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
            bmp_icon, point(pos.x - size.x * 0.5 + 16, pos.y),
            point(16, 16),
            sin(time_spent * ICON_SWAY_TIME_SCALE) * ICON_SWAY_DELTA
        );
        draw_sprite(
            bmp_icon, point(pos.x + size.x * 0.5 - 16, pos.y),
            point(16, 16),
            sin(time_spent * ICON_SWAY_TIME_SCALE) * ICON_SWAY_DELTA
        );
    }
    
    int text_x = pos.x;
    if(text_align == ALLEGRO_ALIGN_LEFT) {
        text_x = pos.x - size.x * 0.5 + 32;
    } else if(text_align == ALLEGRO_ALIGN_RIGHT) {
        text_x = pos.x + size.x * 0.5 - 32;
    }
    
    draw_text_lines(
        font, text_color,
        point(text_x, pos.y),
        text_align, 1, text
    );
}


void menu_button::on_click() {}
bool menu_button::is_clickable() { return enabled; }


/* ----------------------------------------------------------------------------
 * Creates a checkbox widget.
 */
menu_checkbox::menu_checkbox(
    const point &pos, const point &size,
    function<void()> click_handler, string text, ALLEGRO_FONT* font,
    const ALLEGRO_COLOR &color, const int align
) :
    menu_widget(pos, size, click_handler),
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
            bmp_icon, point(pos.x - size.x * 0.5 + 16, pos.y),
            point(16, 16),
            sin(time_spent * ICON_SWAY_TIME_SCALE) * ICON_SWAY_DELTA
        );
        draw_sprite(
            bmp_icon, point(pos.x + size.x * 0.5 - 16, pos.y),
            point(16, 16),
            sin(time_spent * ICON_SWAY_TIME_SCALE) * ICON_SWAY_DELTA
        );
    }
    
    int text_x = pos.x;
    if(text_align == ALLEGRO_ALIGN_LEFT) {
        text_x = pos.x - size.x * 0.5 + 32;
    } else if(text_align == ALLEGRO_ALIGN_RIGHT) {
        text_x = pos.x + size.x * 0.5 - 32;
    }
    
    draw_text_lines(
        font, text_color,
        point(text_x, pos.y),
        text_align, 1, text
    );
    if(checked) {
        draw_sprite(
            bmp_checkbox_check,
            point(pos.x + size.x * 0.5 - 40, pos.y),
            point(32, -1)
        );
    }
}


void menu_checkbox::on_click() { checked = !checked; }
bool menu_checkbox::is_clickable() { return enabled; }


/* ----------------------------------------------------------------------------
 * Creates a text widget.
 */
menu_text::menu_text(
    const point &pos, const point &size, string text,
    ALLEGRO_FONT* font, const ALLEGRO_COLOR &color, const int align
) :
    menu_widget(pos, size, nullptr),
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
    
    int text_x = pos.x;
    if(text_align == ALLEGRO_ALIGN_LEFT) {
        text_x = pos.x - size.x * 0.5 + 32;
    } else if(text_align == ALLEGRO_ALIGN_RIGHT) {
        text_x = pos.x + size.x * 0.5 - 32;
    }
    
    float juicy_grow_amount =
        ease(
            EASE_UP_AND_DOWN,
            juicy_grow_time_left / JUICY_GROW_DURATION
        ) * JUICY_GROW_DELTA;
        
    draw_scaled_text(
        font, text_color,
        point(text_x, pos.y),
        point(1.0 + juicy_grow_amount, 1.0 + juicy_grow_amount),
        text_align, 1, text
    );
}


void menu_text::on_click() {}
bool menu_text::is_clickable() { return false; }
