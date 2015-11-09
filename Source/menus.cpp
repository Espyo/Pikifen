/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Menus.
 */

#include <iostream> //TODO remove

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "drawing.h"
#include "functions.h"
#include "menus.h"
#include "vars.h"

using namespace std;

namespace main_menu {
ALLEGRO_BITMAP* bmp_menu_bg;
unsigned int new_game_state = 0;
menu_widget* selected_widget = NULL;
vector<menu_widget*> menu_widgets;
}


void main_menu::load() {

    draw_loading_screen("", "", 1.0);
    al_flip_display();
    
    //Resources.
    bmp_menu_bg = load_bmp("Main_menu.png");
    
    //Menu widgets.
    menu_widgets.push_back(
        new menu_button(
            scr_w * 0.5, 250, 500, 48,
    [] () {
        new_game_state = GAME_STATE_GAME;
        fade_mgr.start_fade(false);
    }, "Play", font_area_name
        )
    );
    menu_widgets.push_back(
        new menu_button(
            scr_w * 0.5, 300, 500, 48,
    [] () {
        change_game_state(GAME_STATE_OPTIONS);
    }, "Options", font_area_name
        )
    );
    menu_widgets.push_back(
        new menu_button(
            scr_w * 0.5, 350, 500, 48,
    [] () {
        new_game_state = GAME_STATE_ANIMATION_EDITOR;
        fade_mgr.start_fade(false);
    }, "Animation editor", font_area_name
        )
    );
    menu_widgets.push_back(
        new menu_button(
            scr_w * 0.5, 400, 500, 48,
    [] () {
        new_game_state = GAME_STATE_AREA_EDITOR;
        fade_mgr.start_fade(false);
    }, "Area editor", font_area_name
        )
    );
    menu_widgets.push_back(
        new menu_button(
            scr_w * 0.5, 450, 500, 48,
    [] () {
        running = false;
    }, "Exit", font_area_name
        )
    );
    
    set_selected(menu_widgets[0]);
    
    fade_mgr.start_fade(true);
    
}


void main_menu::unload() {

    //Resources.
    al_destroy_bitmap(bmp_menu_bg);
    
    //Menu widgets.
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        delete menu_widgets[w];
    }
    menu_widgets.clear();
    
}


void main_menu::handle_controls(ALLEGRO_EVENT ev) {

    if(fade_mgr.is_fading()) return;
    
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN
    ) {
        set_selected(NULL);
        for(size_t w = 0; w < menu_widgets.size(); w++) {
            menu_widget* w_ptr = menu_widgets[w];
            if(w_ptr->mouse_on(ev.mouse.x, ev.mouse.y)) {
                set_selected(w_ptr);
                break;
            }
        }
    }
    
    if(
        (
            ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
            ev.mouse.button == 1
        ) || (
            ev.type == ALLEGRO_EVENT_KEY_DOWN &&
            (ev.keyboard.keycode == ALLEGRO_KEY_SPACE || ev.keyboard.keycode == ALLEGRO_KEY_ENTER)
        )
    ) {
    
        if(selected_widget && selected_widget->click_handler)
            selected_widget->click_handler();
            
    }
    
    //Selecting a different widget.
    if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
        bool ok_key = false;
        
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_RIGHT ||
            ev.keyboard.keycode == ALLEGRO_KEY_UP ||
            ev.keyboard.keycode == ALLEGRO_KEY_LEFT ||
            ev.keyboard.keycode == ALLEGRO_KEY_DOWN
        ) {
            ok_key = true;
        }
        
        if(ok_key) {
            if(!selected_widget) selected_widget = menu_widgets[0];
            
            menu_widget* closest_widget = NULL;
            dist closest_widget_dist;
            int pivot_x, pivot_y;
            
            for(size_t w = 0; w < menu_widgets.size(); w++) {
                menu_widget* w_ptr = menu_widgets[w];
                if(w_ptr == selected_widget) continue;
                pivot_x = w_ptr->x;
                pivot_y = w_ptr->y;
                
                if(ev.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
                    pivot_x = w_ptr->x;
                    pivot_y = w_ptr->y;
                    if(pivot_x == selected_widget->x) continue;
                    if(pivot_x < selected_widget->x) pivot_x += scr_w;
                    
                } else if(ev.keyboard.keycode == ALLEGRO_KEY_UP) {
                    pivot_x = w_ptr->x;
                    pivot_y = w_ptr->y;
                    if(pivot_y == selected_widget->y) continue;
                    if(pivot_y > selected_widget->y) pivot_y -= scr_w;
                    
                } else if(ev.keyboard.keycode == ALLEGRO_KEY_LEFT) {
                    pivot_x = w_ptr->x;
                    pivot_y = w_ptr->y;
                    if(pivot_x == selected_widget->x) continue;
                    if(pivot_x > selected_widget->x) pivot_x -= scr_w;
                    
                } else if(ev.keyboard.keycode == ALLEGRO_KEY_DOWN) {
                    pivot_x = w_ptr->x;
                    pivot_y = w_ptr->y;
                    if(pivot_y == selected_widget->y) continue;
                    if(pivot_y < selected_widget->y) pivot_y += scr_w;
                }
                
                dist d(selected_widget->x, selected_widget->y, pivot_x, pivot_y);
                
                if(!closest_widget || d <= closest_widget_dist) {
                    closest_widget = w_ptr;
                    closest_widget_dist = d;
                }
            }
            
            if(closest_widget) {
                set_selected(closest_widget);
            }
        }
    }
    
    if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
        if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
            running = false;
        }
    }
    
}


void main_menu::do_logic() {

    //Logic.
    if(fade_mgr.is_fading() && fade_mgr.get_time_left() == 0.0f) {
        if(!fade_mgr.is_fade_in()) {
            change_game_state(new_game_state);
            return;
        }
        fade_mgr.finish_fade();
    }
    fade_mgr.tick(delta_t);
    
    
    //Drawing.
    al_clear_to_color(al_map_rgb(0, 0, 0));
    draw_sprite(
        bmp_menu_bg, scr_w * 0.5, scr_h * 0.5,
        scr_w, scr_h
    );
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->draw();
    }
    
    fade_mgr.draw();
    
    al_flip_display();
}


void main_menu::set_selected(menu_widget* widget) {
    if(selected_widget) selected_widget->selected = false;
    selected_widget = widget;
    if(selected_widget) selected_widget->selected = true;
}
