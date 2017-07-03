/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Game state class and
 * game state-related functions.
 */

#include "game_state.h"
#include "vars.h"


/* ----------------------------------------------------------------------------
 * Creates a game state.
 */
game_state::game_state() :
    selected_widget(NULL) {
    
}


/* ----------------------------------------------------------------------------
 * Sets the currently selected widget.
 */
void game_state::set_selected_widget(menu_widget* widget) {
    if(selected_widget) selected_widget->selected = false;
    selected_widget = widget;
    if(selected_widget) selected_widget->selected = true;
}


/* ----------------------------------------------------------------------------
 * Pass an Allegro event to this so the state's widgets can
 * handle it if necessary.
 */
void game_state::handle_widget_events(ALLEGRO_EVENT ev) {

    //Mousing over a widget and clicking.
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN
    ) {
        set_selected_widget(NULL);
        for(size_t w = 0; w < menu_widgets.size(); w++) {
            menu_widget* w_ptr = menu_widgets[w];
            if(
                w_ptr->mouse_on(point(ev.mouse.x, ev.mouse.y)) &&
                w_ptr->is_clickable()
            ) {
                set_selected_widget(w_ptr);
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
            (
                ev.keyboard.keycode == ALLEGRO_KEY_SPACE ||
                ev.keyboard.keycode == ALLEGRO_KEY_ENTER
            )
        )
    ) {
    
        if(selected_widget)
            selected_widget->click();
            
    }
    
    //Selecting a different widget with the arrow keys.
    if(ev.type == ALLEGRO_EVENT_KEY_CHAR) {
    
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_RIGHT ||
            ev.keyboard.keycode == ALLEGRO_KEY_UP ||
            ev.keyboard.keycode == ALLEGRO_KEY_LEFT ||
            ev.keyboard.keycode == ALLEGRO_KEY_DOWN
        ) {
            if(!selected_widget) selected_widget = menu_widgets[0];
            
            menu_widget* closest_widget = NULL;
            dist closest_widget_dist;
            point cur_pivot;
            point w2_pivot;
            
            for(size_t w = 0; w < menu_widgets.size(); w++) {
                menu_widget* w_ptr = menu_widgets[w];
                if(
                    w_ptr == selected_widget ||
                    !w_ptr->is_clickable()
                ) {
                    continue;
                }
                
                if(ev.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
                    cur_pivot.x =
                        selected_widget->pos.x + selected_widget->size.x * 0.25;
                    cur_pivot.y =
                        selected_widget->pos.y;
                    w2_pivot.x = w_ptr->pos.x - w_ptr->size.x * 0.25;
                    w2_pivot.y = w_ptr->pos.y;
                    
                    if(selected_widget->pos.x == w_ptr->pos.x) continue;
                    if(cur_pivot.x > w2_pivot.x) w2_pivot.x += scr_w;
                    
                } else if(ev.keyboard.keycode == ALLEGRO_KEY_UP) {
                    cur_pivot.x =
                        selected_widget->pos.x;
                    cur_pivot.y =
                        selected_widget->pos.y - selected_widget->size.y * 0.25;
                    w2_pivot.x = w_ptr->pos.x;
                    w2_pivot.y = w_ptr->pos.y + w_ptr->size.y * 0.25;
                    
                    if(selected_widget->pos.y == w_ptr->pos.y) continue;
                    if(cur_pivot.y < w2_pivot.y) w2_pivot.y -= scr_h;
                    
                } else if(ev.keyboard.keycode == ALLEGRO_KEY_LEFT) {
                    cur_pivot.x =
                        selected_widget->pos.x - selected_widget->size.x * 0.25;
                    cur_pivot.y =
                        selected_widget->pos.y;
                    w2_pivot.x = w_ptr->pos.x + w_ptr->size.x * 0.25;
                    w2_pivot.y = w_ptr->pos.y;
                    
                    if(selected_widget->pos.x == w_ptr->pos.x) continue;
                    if(cur_pivot.x < w2_pivot.x) w2_pivot.x -= scr_w;
                    
                } else {
                    cur_pivot.x =
                        selected_widget->pos.x;
                    cur_pivot.y =
                        selected_widget->pos.y + selected_widget->size.y * 0.25;
                    w2_pivot.x = w_ptr->pos.x;
                    w2_pivot.y = w_ptr->pos.y - w_ptr->size.y * 0.25;
                    
                    if(selected_widget->pos.y == w_ptr->pos.y) continue;
                    if(cur_pivot.y > w2_pivot.y) w2_pivot.y += scr_h;
                }
                
                dist d(cur_pivot, w2_pivot);
                
                if(!closest_widget || d <= closest_widget_dist) {
                    closest_widget = w_ptr;
                    closest_widget_dist = d;
                }
            }
            
            if(closest_widget) {
                set_selected_widget(closest_widget);
            }
        }
    }
    
}


void game_state::update_transformations() { }
