/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
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


game_state::game_state() :
    selected_widget(NULL) {
    
}


void game_state::set_selected_widget(menu_widget* widget) {
    if(selected_widget) selected_widget->selected = false;
    selected_widget = widget;
    if(selected_widget) selected_widget->selected = true;
}


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
            (ev.keyboard.keycode == ALLEGRO_KEY_SPACE || ev.keyboard.keycode == ALLEGRO_KEY_ENTER)
        )
    ) {
    
        if(selected_widget)
            selected_widget->click();
            
    }
    
    //Selecting a different widget with the arrow keys.
    if(ev.type == ALLEGRO_EVENT_KEY_CHAR) {
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
                    cur_pivot_x = selected_widget->x + selected_widget->w * 0.25;
                    cur_pivot_y = selected_widget->y;
                    w2_pivot_x = w_ptr->x - w_ptr->w * 0.25;
                    w2_pivot_y = w_ptr->y;
                    
                    if(selected_widget->x == w_ptr->x) continue;
                    if(cur_pivot_x > w2_pivot_x) w2_pivot_x += scr_w;
                    
                } else if(ev.keyboard.keycode == ALLEGRO_KEY_UP) {
                    cur_pivot_x = selected_widget->x;
                    cur_pivot_y = selected_widget->y - selected_widget->h * 0.25;
                    w2_pivot_x = w_ptr->x;
                    w2_pivot_y = w_ptr->y + w_ptr->h * 0.25;
                    
                    if(selected_widget->y == w_ptr->y) continue;
                    if(cur_pivot_y < w2_pivot_y) w2_pivot_y -= scr_h;
                    
                } else if(ev.keyboard.keycode == ALLEGRO_KEY_LEFT) {
                    cur_pivot_x = selected_widget->x - selected_widget->w * 0.25;
                    cur_pivot_y = selected_widget->y;
                    w2_pivot_x = w_ptr->x + w_ptr->w * 0.25;
                    w2_pivot_y = w_ptr->y;
                    
                    if(selected_widget->x == w_ptr->x) continue;
                    if(cur_pivot_x < w2_pivot_x) w2_pivot_x -= scr_w;
                    
                } else if(ev.keyboard.keycode == ALLEGRO_KEY_DOWN) {
                    cur_pivot_x = selected_widget->x;
                    cur_pivot_y = selected_widget->y + selected_widget->h * 0.25;
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



gameplay::gameplay() :
    game_state() {}


void gameplay::load() {
    draw_loading_screen("", "", 1.0f);
    al_flip_display();
    
    al_set_display_icon(display, bmp_icon);
    
    //Game content.
    load_game_content();
    
    //Initializing game things.
    spray_amounts.clear();
    size_t n_spray_types = spray_types.size();
    for(size_t s = 0; s < n_spray_types; ++s) { spray_amounts.push_back(0); }
    pikmin_in_onions.clear();
    for(auto o = pikmin_in_onions.begin(); o != pikmin_in_onions.end(); ++o) { o->second = 0; }
    
    load_area(area_to_load, false);
    load_area_textures();
    generate_area_images();
    
    //Generate mobs.
    for(size_t m = 0; m < cur_area_map.mob_generators.size(); ++m) {
        mob_gen* m_ptr = cur_area_map.mob_generators[m];
        if(m_ptr->category == MOB_CATEGORY_ENEMIES) {
            create_mob(new enemy(m_ptr->x, m_ptr->y, (enemy_type*) m_ptr->type, m_ptr->angle, m_ptr->vars));
        } else if(m_ptr->category == MOB_CATEGORY_LEADERS) {
            create_mob(new leader(m_ptr->x, m_ptr->y, (leader_type*) m_ptr->type, m_ptr->angle, m_ptr->vars));
        } else if(m_ptr->category == MOB_CATEGORY_ONIONS) {
            create_mob(new onion(m_ptr->x, m_ptr->y, (onion_type*) m_ptr->type, m_ptr->angle, m_ptr->vars));
        } else if(m_ptr->category == MOB_CATEGORY_PELLETS) {
            create_mob(new pellet(m_ptr->x, m_ptr->y, (pellet_type*) m_ptr->type, m_ptr->angle, m_ptr->vars));
        } else if(m_ptr->category == MOB_CATEGORY_PIKMIN) {
            create_mob(new pikmin(m_ptr->x, m_ptr->y, (pikmin_type*) m_ptr->type, m_ptr->angle, m_ptr->vars));
        } else if(m_ptr->category == MOB_CATEGORY_SHIPS) {
            create_mob(new ship(m_ptr->x, m_ptr->y, (ship_type*) m_ptr->type, m_ptr->angle, m_ptr->vars));
        } else if(m_ptr->category == MOB_CATEGORY_GATES) {
            create_mob(new gate(m_ptr->x, m_ptr->y, (gate_type*) m_ptr->type, m_ptr->angle, m_ptr->vars));
        } else if(m_ptr->category == MOB_CATEGORY_SPECIAL) {
            m_ptr->type->create_mob(m_ptr->x, m_ptr->y, m_ptr->angle, m_ptr->vars);
        } else if(m_ptr->category == MOB_CATEGORY_TREASURES) {
            create_mob(new treasure(m_ptr->x, m_ptr->y, (treasure_type*) m_ptr->type, m_ptr->angle, m_ptr->vars));
        }
    }
    
    /*
    create_mob(new pellet(320, -100, pellet_types["Red 1"], 0, ""));
    create_mob(new pellet(250, -100, pellet_types["Red 5"], 0, ""));
    create_mob(new pellet(150, -100, pellet_types["Red 10"], 0, ""));
    create_mob(new pellet(0, -100, pellet_types["Red 20"], 0, ""));*/
    spray_amounts[0] = spray_amounts[1] = 10;
    spray_types[0].bmp_spray = bmp_ub_spray;
    spray_types[1].bmp_spray = bmp_us_spray;
    pikmin_in_onions[pikmin_types["Red Pikmin"]] = 200;
    pikmin_in_onions[pikmin_types["Yellow Pikmin"]] = 180;
    pikmin_in_onions[pikmin_types["Blue Pikmin"]] = 160;
    
    cur_leader_nr = 0;
    cur_leader_ptr = leaders[cur_leader_nr];
    cur_leader_ptr->fsm.set_state(LEADER_STATE_ACTIVE);
    cur_leader_ptr->first_state_set = true;
    
    cam_zoom = 1.0;
    
    al_hide_mouse_cursor(display);
    
    area_title_fade_timer.start();
}


void gameplay::unload() {
    //TODO
}


void gameplay::handle_controls(ALLEGRO_EVENT ev) {
    handle_game_controls(ev);
}


void gameplay::do_logic() {
    do_game_logic();
}


void gameplay::do_drawing() {
    do_game_drawing();
}
