/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
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
    right_pressed(false),
    up_pressed(false),
    left_pressed(false),
    down_pressed(false),
    ok_pressed(false),
    back_pressed(false),
    back_widget(NULL),
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
    
    if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && ev.mouse.button == 1) {
        if(selected_widget) selected_widget->click();
    }
    
    vector<action_from_event> actions = get_actions_from_event(ev);
    for(size_t a = 0; a < actions.size(); ++a) {
        handle_menu_button(
            actions[a].button, actions[a].pos, actions[a].player
        );
    }
    
}


/* ----------------------------------------------------------------------------
 * Handles a button "press" in a menu. Technically, it could also be
 * a button release.
 * button: The button's ID. Use BUTTON_*.
 * pos:    The position of the button, i.e., how much it's "held".
 *   0 means it was released. 1 means it was fully pressed.
 *   For controls with more sensitivity, values between 0 and 1 are important.
 *   Like a 0.5 for the group movement makes it move at half distance.
 * player: Number of the player that pressed.
 */
void game_state::handle_menu_button(
    const size_t action, const float pos, const size_t player
) {

    bool is_down = (pos >= 0.5);
    
    //Selecting a different widget with the arrow keys.
    if(
        action == BUTTON_MENU_RIGHT ||
        action == BUTTON_MENU_UP ||
        action == BUTTON_MENU_LEFT ||
        action == BUTTON_MENU_DOWN
    ) {
    
        size_t pressed = BUTTON_NONE;
        if(action == BUTTON_MENU_RIGHT) {
            if(!right_pressed && is_down) {
                pressed = BUTTON_MENU_RIGHT;
            }
            right_pressed = is_down;
            
        } else if(action == BUTTON_MENU_UP) {
            if(!up_pressed && is_down) {
                pressed = BUTTON_MENU_UP;
            }
            up_pressed = is_down;
            
        } else if(action == BUTTON_MENU_LEFT) {
            if(!left_pressed && is_down) {
                pressed = BUTTON_MENU_LEFT;
            }
            left_pressed = is_down;
            
        } else if(action == BUTTON_MENU_DOWN) {
            if(!down_pressed && is_down) {
                pressed = BUTTON_MENU_DOWN;
            }
            down_pressed = is_down;
            
        }
        
        if(pressed == BUTTON_NONE) return;
        
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
            
            if(pressed == BUTTON_MENU_RIGHT) {
                cur_pivot.x =
                    selected_widget->center.x + selected_widget->size.x * 0.25;
                cur_pivot.y =
                    selected_widget->center.y;
                w2_pivot.x = w_ptr->center.x - w_ptr->size.x * 0.25;
                w2_pivot.y = w_ptr->center.y;
                
                if(selected_widget->center.x == w_ptr->center.x) continue;
                if(cur_pivot.x > w2_pivot.x) w2_pivot.x += scr_w;
                
            } else if(pressed == BUTTON_MENU_UP) {
                cur_pivot.x =
                    selected_widget->center.x;
                cur_pivot.y =
                    selected_widget->center.y - selected_widget->size.y * 0.25;
                w2_pivot.x = w_ptr->center.x;
                w2_pivot.y = w_ptr->center.y + w_ptr->size.y * 0.25;
                
                if(selected_widget->center.y == w_ptr->center.y) continue;
                if(cur_pivot.y < w2_pivot.y) w2_pivot.y -= scr_h;
                
            } else if(pressed == BUTTON_MENU_LEFT) {
                cur_pivot.x =
                    selected_widget->center.x - selected_widget->size.x * 0.25;
                cur_pivot.y =
                    selected_widget->center.y;
                w2_pivot.x = w_ptr->center.x + w_ptr->size.x * 0.25;
                w2_pivot.y = w_ptr->center.y;
                
                if(selected_widget->center.x == w_ptr->center.x) continue;
                if(cur_pivot.x < w2_pivot.x) w2_pivot.x -= scr_w;
                
            } else {
                cur_pivot.x =
                    selected_widget->center.x;
                cur_pivot.y =
                    selected_widget->center.y + selected_widget->size.y * 0.25;
                w2_pivot.x = w_ptr->center.x;
                w2_pivot.y = w_ptr->center.y - w_ptr->size.y * 0.25;
                
                if(selected_widget->center.y == w_ptr->center.y) continue;
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
        
    } else if(action == BUTTON_MENU_OK && is_down) {
    
        if(selected_widget) selected_widget->click();
        
    } else if(action == BUTTON_MENU_BACK && is_down) {
    
        if(back_widget) back_widget->click();
        
    }
}


void game_state::handle_event(ALLEGRO_EVENT *ev) { }
void game_state::update_transformations() { }
game_state::~game_state() { }
