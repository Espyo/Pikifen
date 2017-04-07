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

#include <algorithm>

#include "drawing.h"
#include "functions.h"
#include "game_state.h"
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
    
    lightmap_bmp = al_create_bitmap(scr_w, scr_h);
    
    //Generate mobs.
    for(size_t m = 0; m < cur_area_data.mob_generators.size(); ++m) {
        mob_gen* m_ptr = cur_area_data.mob_generators[m];
        if(m_ptr->category == MOB_CATEGORY_ENEMIES) {
            create_mob(
                new enemy(
                    m_ptr->pos, (enemy_type*) m_ptr->type,
                    m_ptr->angle, m_ptr->vars
                )
            );
        } else if(m_ptr->category == MOB_CATEGORY_LEADERS) {
            create_mob(
                new leader(
                    m_ptr->pos, (leader_type*) m_ptr->type,
                    m_ptr->angle, m_ptr->vars
                )
            );
        } else if(m_ptr->category == MOB_CATEGORY_ONIONS) {
            create_mob(
                new onion(
                    m_ptr->pos, (onion_type*) m_ptr->type,
                    m_ptr->angle, m_ptr->vars
                )
            );
        } else if(m_ptr->category == MOB_CATEGORY_PELLETS) {
            create_mob(
                new pellet(
                    m_ptr->pos, (pellet_type*) m_ptr->type,
                    m_ptr->angle, m_ptr->vars
                )
            );
        } else if(m_ptr->category == MOB_CATEGORY_PIKMIN) {
            create_mob(
                new pikmin(
                    m_ptr->pos, (pikmin_type*) m_ptr->type,
                    m_ptr->angle, m_ptr->vars
                )
            );
        } else if(m_ptr->category == MOB_CATEGORY_SHIPS) {
            create_mob(
                new ship(
                    m_ptr->pos, (ship_type*) m_ptr->type,
                    m_ptr->angle, m_ptr->vars
                )
            );
        } else if(m_ptr->category == MOB_CATEGORY_GATES) {
            create_mob(
                new gate(
                    m_ptr->pos, (gate_type*) m_ptr->type,
                    m_ptr->angle, m_ptr->vars
                )
            );
        } else if(m_ptr->category == MOB_CATEGORY_BRIDGES) {
            create_mob(
                new bridge(
                    m_ptr->pos, (bridge_type*) m_ptr->type,
                    m_ptr->angle, m_ptr->vars
                )
            );
        } else if(m_ptr->category == MOB_CATEGORY_SPECIAL) {
            m_ptr->type->create_mob(
                m_ptr->pos, m_ptr->angle, m_ptr->vars
            );
        } else if(m_ptr->category == MOB_CATEGORY_TREASURES) {
            create_mob(
                new treasure(
                    m_ptr->pos, (treasure_type*) m_ptr->type,
                    m_ptr->angle, m_ptr->vars
                )
            );
        } else if(m_ptr->category == MOB_CATEGORY_MISC) {
            create_mob(
                new mob(
                    m_ptr->pos, m_ptr->type,
                    m_ptr->angle, m_ptr->vars
                )
            );
        }
    }
    
    //Sort leaders.
    sort(
        leaders.begin(), leaders.end(),
    [] (leader * l1, leader * l2) -> bool {
        size_t priority_l1 =
        find(leader_order.begin(), leader_order.end(), l1->lea_type) -
        leader_order.begin();
        size_t priority_l2 =
        find(leader_order.begin(), leader_order.end(), l2->lea_type) -
        leader_order.begin();
        return priority_l1 < priority_l2;
    }
    );
    
    cur_leader_nr = 0;
    cur_leader_ptr = leaders[cur_leader_nr];
    cur_leader_ptr->fsm.set_state(LEADER_STATE_ACTIVE);
    
    cam_pos = cam_final_pos = cur_leader_ptr->pos;
    cam_zoom = cam_final_zoom = zoom_mid_level;
    update_transformations();
    
    leader_cursor_w.x = cur_leader_ptr->pos.x + cursor_max_dist / 2.0;
    leader_cursor_w.y = cur_leader_ptr->pos.y;
    leader_cursor_s = leader_cursor_w;
    al_transform_coordinates(
        &world_to_screen_transform,
        &leader_cursor_s.x, &leader_cursor_s.y
    );
    mouse_cursor_w = leader_cursor_w;
    mouse_cursor_s = leader_cursor_s;
    al_set_mouse_xy(display, mouse_cursor_s.x, mouse_cursor_s.y);
    
    day_minutes = day_minutes_start;
    area_time_passed = 0;
    
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
 * Updates the transformations, with the current camera coordinates, zoom, etc.
 */
void gameplay::update_transformations() {
    //World coordinates to screen coordinates.
    world_to_screen_transform = identity_transform;
    al_translate_transform(
        &world_to_screen_transform,
        -cam_pos.x + scr_w / 2.0 / cam_zoom,
        -cam_pos.y + scr_h / 2.0 / cam_zoom
    );
    al_scale_transform(&world_to_screen_transform, cam_zoom, cam_zoom);
    
    //Screen coordinates to world coordinates.
    screen_to_world_transform = world_to_screen_transform;
    al_invert_transform(&screen_to_world_transform);
}


/* ----------------------------------------------------------------------------
 * Handle Allegro events.
 */
void gameplay::handle_controls(const ALLEGRO_EVENT &ev) {
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
