/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Game state class and
 * game state-related functions.
 */

#include "drawing.h"
#include "functions.h"
#include "game_state.h"
#include "logic.h"
#include "misc_structs.h"
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
                w_ptr->mouse_on(ev.mouse.x, ev.mouse.y) &&
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
            int cur_pivot_x, cur_pivot_y;
            int w2_pivot_x, w2_pivot_y;
            
            for(size_t w = 0; w < menu_widgets.size(); w++) {
                menu_widget* w_ptr = menu_widgets[w];
                if(
                    w_ptr == selected_widget ||
                    !w_ptr->is_clickable()
                ) {
                    continue;
                }
                
                if(ev.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
                    cur_pivot_x =
                        selected_widget->x + selected_widget->w * 0.25;
                    cur_pivot_y =
                        selected_widget->y;
                    w2_pivot_x = w_ptr->x - w_ptr->w * 0.25;
                    w2_pivot_y = w_ptr->y;
                    
                    if(selected_widget->x == w_ptr->x) continue;
                    if(cur_pivot_x > w2_pivot_x) w2_pivot_x += scr_w;
                    
                } else if(ev.keyboard.keycode == ALLEGRO_KEY_UP) {
                    cur_pivot_x =
                        selected_widget->x;
                    cur_pivot_y =
                        selected_widget->y - selected_widget->h * 0.25;
                    w2_pivot_x = w_ptr->x;
                    w2_pivot_y = w_ptr->y + w_ptr->h * 0.25;
                    
                    if(selected_widget->y == w_ptr->y) continue;
                    if(cur_pivot_y < w2_pivot_y) w2_pivot_y -= scr_h;
                    
                } else if(ev.keyboard.keycode == ALLEGRO_KEY_LEFT) {
                    cur_pivot_x =
                        selected_widget->x - selected_widget->w * 0.25;
                    cur_pivot_y =
                        selected_widget->y;
                    w2_pivot_x = w_ptr->x + w_ptr->w * 0.25;
                    w2_pivot_y = w_ptr->y;
                    
                    if(selected_widget->x == w_ptr->x) continue;
                    if(cur_pivot_x < w2_pivot_x) w2_pivot_x -= scr_w;
                    
                } else {
                    cur_pivot_x =
                        selected_widget->x;
                    cur_pivot_y =
                        selected_widget->y + selected_widget->h * 0.25;
                    w2_pivot_x = w_ptr->x;
                    w2_pivot_y = w_ptr->y - w_ptr->h * 0.25;
                    
                    if(selected_widget->y == w_ptr->y) continue;
                    if(cur_pivot_y > w2_pivot_y) w2_pivot_y += scr_h;
                }
                
                dist d(cur_pivot_x, cur_pivot_y, w2_pivot_x, w2_pivot_y);
                
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



/* ----------------------------------------------------------------------------
 * Creates the "gameplay" state.
 */
gameplay::gameplay() :
    game_state() {
    
    
}


/* ----------------------------------------------------------------------------
 * Loads the "gameplay" state into memory.
 */
void gameplay::load() {
    ready_for_input = false;
    
    if(loading_text_bmp) al_destroy_bitmap(loading_text_bmp);
    if(loading_subtext_bmp) al_destroy_bitmap(loading_subtext_bmp);
    loading_text_bmp = NULL;
    loading_subtext_bmp = NULL;
    
    draw_loading_screen("", "", 1.0f);
    al_flip_display();
    
    al_set_display_icon(display, bmp_icon);
    
    //Game content.
    load_game_content();
    
    //Initializing game things.
    spray_amounts.clear();
    size_t n_spray_types = spray_types.size();
    for(size_t s = 0; s < n_spray_types; ++s) { spray_amounts.push_back(0); }
    
    load_area(area_to_load, false, false);
    load_area_textures();
    generate_area_images();
    
    //Generate mobs.
    for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
        mob_gen* m_ptr = cur_area_data.mob_generators[m];
        if(m_ptr->category == MOB_CATEGORY_ENEMIES) {
            create_mob(
                new enemy(
                    m_ptr->x, m_ptr->y, (enemy_type*) m_ptr->type,
                    m_ptr->angle, m_ptr->vars
                )
            );
        } else if(m_ptr->category == MOB_CATEGORY_LEADERS) {
            create_mob(
                new leader(
                    m_ptr->x, m_ptr->y, (leader_type*) m_ptr->type,
                    m_ptr->angle, m_ptr->vars
                )
            );
        } else if(m_ptr->category == MOB_CATEGORY_ONIONS) {
            create_mob(
                new onion(
                    m_ptr->x, m_ptr->y, (onion_type*) m_ptr->type,
                    m_ptr->angle, m_ptr->vars
                )
            );
        } else if(m_ptr->category == MOB_CATEGORY_PELLETS) {
            create_mob(
                new pellet(
                    m_ptr->x, m_ptr->y, (pellet_type*) m_ptr->type,
                    m_ptr->angle, m_ptr->vars
                )
            );
        } else if(m_ptr->category == MOB_CATEGORY_PIKMIN) {
            create_mob(
                new pikmin(
                    m_ptr->x, m_ptr->y, (pikmin_type*) m_ptr->type,
                    m_ptr->angle, m_ptr->vars
                )
            );
        } else if(m_ptr->category == MOB_CATEGORY_SHIPS) {
            create_mob(
                new ship(
                    m_ptr->x, m_ptr->y, (ship_type*) m_ptr->type,
                    m_ptr->angle, m_ptr->vars
                )
            );
        } else if(m_ptr->category == MOB_CATEGORY_GATES) {
            create_mob(
                new gate(
                    m_ptr->x, m_ptr->y, (gate_type*) m_ptr->type,
                    m_ptr->angle, m_ptr->vars
                )
            );
        } else if(m_ptr->category == MOB_CATEGORY_SPECIAL) {
            m_ptr->type->create_mob(
                m_ptr->x, m_ptr->y, m_ptr->angle, m_ptr->vars
            );
        } else if(m_ptr->category == MOB_CATEGORY_TREASURES) {
            create_mob(
                new treasure(
                    m_ptr->x, m_ptr->y, (treasure_type*) m_ptr->type,
                    m_ptr->angle, m_ptr->vars
                )
            );
        } else if(m_ptr->category == MOB_CATEGORY_MISC) {
            create_mob(
                new mob(
                    m_ptr->x, m_ptr->y, m_ptr->type,
                    m_ptr->angle, m_ptr->vars
                )
            );
        }
    }
    
    cur_leader_nr = 0;
    cur_leader_ptr = leaders[cur_leader_nr];
    cur_leader_ptr->fsm.set_state(LEADER_STATE_ACTIVE);
    
    day_minutes = day_minutes_start;
    area_time_passed = 0;
    
    cam_x = cam_final_x = cur_leader_ptr->x;
    cam_y = cam_final_y = cur_leader_ptr->y;
    cam_zoom = cam_final_zoom = zoom_mid_level;
    
    for(size_t c = 0; c < controls[0].size(); ++c) {
        if(controls[0][c].action == BUTTON_THROW) {
            click_control_id = c;
            break;
        }
    }
    
    al_hide_mouse_cursor(display);
    
    area_title_fade_timer.start();
    
    //Aesthetic stuff.
    cur_message_char_timer =
        timer(
    message_char_interval, [] () {
        cur_message_char_timer.start(); cur_message_char++;
    }
        );
        
    //Debug stuff for convenience.
    //TODO remove.
    for(size_t s = 0; s < spray_types.size(); ++s) {
        spray_amounts[s] = 20;
    }
    
}


/* ----------------------------------------------------------------------------
 * Unloads the "gameplay" state from memory.
 */
void gameplay::unload() {
    //TODO
}


/* ----------------------------------------------------------------------------
 * Handle Allegro events.
 */
void gameplay::handle_controls(ALLEGRO_EVENT ev) {
    handle_game_controls(ev);
}


/* ----------------------------------------------------------------------------
 * Tick the gameplay logic by one frame.
 */
void gameplay::do_logic() {
    if(dev_tool_change_speed) {
        delta_t *= dev_tool_change_speed_mult;
    }
    
    do_gameplay_logic();
    do_aesthetic_logic();
}


/* ----------------------------------------------------------------------------
 * Draw the gameplay.
 */
void gameplay::do_drawing() {
    do_game_drawing();
}
