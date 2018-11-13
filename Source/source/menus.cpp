/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Menus.
 */

#include <algorithm>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "drawing.h"
#include "functions.h"
#include "load.h"
#include "menus.h"
#include "utils/string_utils.h"
#include "vars.h"

using namespace std;


/* ----------------------------------------------------------------------------
 * Creates a "main menu" state.
 */
main_menu::main_menu() :
    game_state(),
    new_game_state(0),
    time_spent(0),
    bmp_menu_bg(NULL),
    logo_pikmin_max_speed(0),
    logo_pikmin_min_speed(0),
    logo_pikmin_speed_smoothness(0),
    logo_pikmin_sway_amount(0),
    logo_pikmin_sway_max_speed(0),
    logo_pikmin_sway_min_speed(0) {
    
}


/* ----------------------------------------------------------------------------
 * Loads the main menu into memory.
 */
void main_menu::load() {
    selected_widget = NULL;
    
    draw_loading_screen("", "", 1.0);
    al_flip_display();
    
    //Resources.
    bmp_menu_bg = load_bmp(asset_file_names.main_menu);
    data_node title_screen_file(TITLE_SCREEN_FILE_PATH);
    
    //Menu widgets.
    menu_widgets.push_back(
        new menu_button(
            point(scr_w * 0.5, scr_h * 0.55), point(scr_w * 0.8, scr_h * 0.06),
    [this] () {
        fade_mgr.start_fade(false, [] () {
            change_game_state(GAME_STATE_AREA_MENU);
        });
    }, "Play", font_area_name
        )
    );
    menu_widgets.push_back(
        new menu_button(
            point(scr_w * 0.5, scr_h * 0.63), point(scr_w * 0.8, scr_h * 0.06),
    [this] () {
        fade_mgr.start_fade(false, [] () {
            change_game_state(GAME_STATE_OPTIONS_MENU);
        });
    }, "Options", font_area_name
        )
    );
    menu_widgets.push_back(
        new menu_button(
            point(scr_w * 0.5, scr_h * 0.71), point(scr_w * 0.8, scr_h * 0.06),
    [this] () {
        fade_mgr.start_fade(false, [] () {
            change_game_state(GAME_STATE_ANIMATION_EDITOR);
        });
    }, "Animation editor", font_area_name
        )
    );
    menu_widgets.push_back(
        new menu_button(
            point(scr_w * 0.5, scr_h * 0.79), point(scr_w * 0.8, scr_h * 0.06),
    [this] () {
        fade_mgr.start_fade(false, [] () {
            change_game_state(GAME_STATE_AREA_EDITOR);
        });
    }, "Area editor", font_area_name
        )
    );
    back_widget =
        new menu_button(
        point(scr_w * 0.5, scr_h * 0.87), point(scr_w * 0.8, scr_h * 0.06),
    [] () {
        is_game_running = false;
    }, "Exit", font_area_name
    );
    menu_widgets.push_back(back_widget);
    
    //Logo pikmin.
    data_node* logo_node =
        title_screen_file.get_child_by_name("logo");
        
    data_node* pik_types_node =
        logo_node->get_child_by_name("pikmin_types");
    for(size_t t = 0; t < pik_types_node->get_nr_of_children(); ++t) {
        data_node* type_node = pik_types_node->get_child(t);
        if(type_node->name.empty()) continue;
        logo_type_bitmaps[type_node->name[0]] =
            load_bmp(type_node->value, type_node);
    }
    
    data_node* map_node =
        logo_node->get_child_by_name("map");
    size_t map_total_rows = map_node->get_nr_of_children();
    size_t map_total_cols = 0;
    for(size_t r = 0; r < map_total_rows; ++r) {
        map_total_cols =
            max(map_total_cols, map_node->get_child(r)->name.size());
    }
    
    logo_min_screen_limit =
        s2p(
            logo_node->get_child_by_name(
                "min_screen_limit"
            )->get_value_or_default("10 10")
        );
    logo_min_screen_limit.x *= scr_w / 100.0f;
    logo_min_screen_limit.y *= scr_h / 100.0f;
    logo_max_screen_limit =
        s2p(
            logo_node->get_child_by_name(
                "max_screen_limit"
            )->get_value_or_default("90 50")
        );
    logo_max_screen_limit.x *= scr_w / 100.0f;
    logo_max_screen_limit.y *= scr_h / 100.0f;
    logo_pikmin_max_speed =
        s2f(
            logo_node->get_child_by_name(
                "pikmin_max_speed"
            )->get_value_or_default("800")
        );
    logo_pikmin_min_speed =
        s2f(
            logo_node->get_child_by_name(
                "pikmin_min_speed"
            )->get_value_or_default("600")
        );
    logo_pikmin_speed_smoothness =
        s2f(
            logo_node->get_child_by_name(
                "pikmin_speed_smoothness"
            )->get_value_or_default("0.08")
        );
    logo_pikmin_sway_amount =
        s2f(
            logo_node->get_child_by_name(
                "pikmin_sway_amount"
            )->get_value_or_default("3")
        );
    logo_pikmin_sway_max_speed =
        s2f(
            logo_node->get_child_by_name(
                "pikmin_sway_max_speed"
            )->get_value_or_default("5.5")
        );
    logo_pikmin_sway_min_speed =
        s2f(
            logo_node->get_child_by_name(
                "pikmin_sway_min_speed"
            )->get_value_or_default("2.5")
        );
    logo_pikmin_size =
        s2p(
            logo_node->get_child_by_name(
                "pikmin_size"
            )->get_value_or_default("3.5 3.5")
        );
    logo_pikmin_size.x *= scr_w / 100.0f;
    logo_pikmin_size.y *= scr_h / 100.0f;
    
    bool map_ok = true;
    
    for(size_t r = 0; r < map_total_rows; ++r) {
        string row = map_node->get_child(r)->name;
        
        for(size_t c = 0; c < row.size(); ++c) {
            if(row[c] == '.') continue;
            if(logo_type_bitmaps.find(row[c]) == logo_type_bitmaps.end()) {
                map_ok = false;
                log_error(
                    "Title screen Pikmin logo map has an invalid character \"" +
                    string(1, row[c]) + "\" on row " + i2s(r + 1) +
                    ", column " + i2s(c + 1) + "!"
                );
                break;
            }
            
            logo_pik pik;
            
            pik.top = logo_type_bitmaps[row[c]];
            pik.destination =
                point(
                    logo_min_screen_limit.x +
                    (logo_max_screen_limit.x - logo_min_screen_limit.x) *
                    (c / (float) map_total_cols),
                    logo_min_screen_limit.y +
                    (logo_max_screen_limit.y - logo_min_screen_limit.y) *
                    (r / (float) map_total_rows)
                );
                
            unsigned char h_side = randomi(0, 1);
            unsigned char v_side = randomi(0, 1);
            
            pik.pos =
                point(
                    randomf(0, scr_w * 0.5),
                    randomf(0, scr_h * 0.5)
                );
                
            if(h_side == 0) {
                pik.pos.x -= scr_w * 1.2;
            } else {
                pik.pos.x += scr_w * 1.2;
            }
            if(v_side == 0) {
                pik.pos.y -= scr_h * 1.2;
            } else {
                pik.pos.y += scr_h * 1.2;
            }
            
            pik.angle = randomf(0, TAU);
            pik.speed = randomf(logo_pikmin_min_speed, logo_pikmin_max_speed);
            pik.sway_speed =
                randomf(logo_pikmin_sway_min_speed, logo_pikmin_sway_max_speed);
            pik.sway_var = 0;
            pik.reached_destination = false;
            logo_pikmin.push_back(pik);
        }
        
        if(!map_ok) break;
    }
    
    //Finishing touches.
    set_selected_widget(menu_widgets[0]);
    fade_mgr.start_fade(true, nullptr);
    
}


/* ----------------------------------------------------------------------------
 * Unloads the main menu from memory.
 */
void main_menu::unload() {

    //Resources.
    al_destroy_bitmap(bmp_menu_bg);
    
    //Menu widgets.
    set_selected_widget(NULL);
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        delete menu_widgets[w];
    }
    menu_widgets.clear();
    
    //Misc.
    logo_pikmin.clear();
    
}


/* ----------------------------------------------------------------------------
 * Handles Allegro events.
 */
void main_menu::handle_controls(const ALLEGRO_EVENT &ev) {
    if(fade_mgr.is_fading()) return;
    
    handle_widget_events(ev);
}


/* ----------------------------------------------------------------------------
 * Ticks a frame's worth of logic.
 */
void main_menu::do_logic() {
    time_spent += delta_t;
    
    //Animate the logo Pikmin.
    for(size_t p = 0; p < logo_pikmin.size(); ++p) {
        logo_pik* pik = &logo_pikmin[p];
        
        if(!pik->reached_destination) {
            float a = get_angle(pik->pos, pik->destination);
            float speed =
                min(
                    (float) (pik->speed * delta_t),
                    dist(pik->pos, pik->destination).to_float() *
                    logo_pikmin_speed_smoothness
                );
            pik->pos.x += cos(a) * speed;
            pik->pos.y += sin(a) * speed;
            if(
                fabs(pik->pos.x - pik->destination.x) < 1.0 &&
                fabs(pik->pos.y - pik->destination.y) < 1.0
            ) {
                pik->destination = pik->pos;
                pik->reached_destination = true;
            }
            
        } else {
            pik->sway_var += pik->sway_speed * delta_t;
            pik->pos.x =
                pik->destination.x +
                sin(pik->sway_var) * logo_pikmin_sway_amount;
        }
    }
    
    //Fade manager needs to come last, because if
    //the fade finishes and the state changes, and
    //after that we still attempt to do stuff in
    //this function, we're going to have a bad time.
    fade_mgr.tick(delta_t);
    
}


/* ----------------------------------------------------------------------------
 * Draws the main menu.
 */
void main_menu::do_drawing() {
    al_clear_to_color(al_map_rgb(0, 0, 0));
    draw_bitmap(
        bmp_menu_bg, point(scr_w * 0.5, scr_h * 0.5),
        point(scr_w, scr_h)
    );
    
    //Draw the logo Pikmin.
    for(size_t p = 0; p < logo_pikmin.size(); ++p) {
        logo_pik* pik = &logo_pikmin[p];
        
        draw_bitmap_in_box(pik->top, pik->pos, logo_pikmin_size, pik->angle);
    }
    
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->draw(time_spent);
    }
    
    draw_scaled_text(
        font_main, al_map_rgb(255, 255, 255),
        point(8, scr_h  - 8),
        point(0.4, 0.4),
        ALLEGRO_ALIGN_LEFT, 2,
        "Pikmin (c) Nintendo"
    );
    draw_scaled_text(
        font_main, al_map_rgb(255, 255, 255),
        point(scr_w - 8, scr_h  - 8),
        point(0.4, 0.4),
        ALLEGRO_ALIGN_RIGHT, 2,
        game_name + " " + game_version +
        ", powered by Pikifen " +
        i2s(VERSION_MAJOR) + "." + i2s(VERSION_MINOR)  + "." + i2s(VERSION_REV)
    );
    
    fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Creates an "options menu" state.
 */
options_menu::options_menu() :
    game_state() {
    
    //Let's fill in the list of preset resolutions. For that, we'll get
    //the display modes fetched by Allegro. These are usually nice round
    //resolutions, and they work on fullscreen mode.
    int n_modes = al_get_num_display_modes();
    for(int d = 0; d < n_modes; ++d) {
        ALLEGRO_DISPLAY_MODE d_info;
        if(!al_get_display_mode(d, &d_info)) continue;
        if(d_info.width < SMALLEST_SCR_W) continue;
        if(d_info.height < SMALLEST_SCR_H) continue;
        resolution_presets.push_back(make_pair(d_info.width, d_info.height));
    }
    
    //In case things go wrong, at least add these presets.
    resolution_presets.push_back(make_pair(DEF_SCR_W, DEF_SCR_H));
    resolution_presets.push_back(make_pair(SMALLEST_SCR_W, SMALLEST_SCR_H));
    
    //Sort the list.
    sort(
        resolution_presets.begin(), resolution_presets.end(),
    [] (pair<int, int> p1, pair<int, int> p2) -> bool {
        if(p1.first == p2.first) {
            return p1.second < p2.second;
        }
        return p1.first < p2.first;
    }
    );
    
    //Remove any duplicates.
    for(size_t p = 0; p < resolution_presets.size() - 1;) {
        if(resolution_presets[p] == resolution_presets[p + 1]) {
            resolution_presets.erase(resolution_presets.begin() + (p + 1));
        } else {
            ++p;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Loads the options menu into memory.
 */
void options_menu::load() {
    //Resources.
    bmp_menu_bg = load_bmp(asset_file_names.main_menu);
    
    //Menu widgets.
    back_widget =
        new menu_button(
        point(scr_w * 0.15, scr_h * 0.10),
        point(scr_w * 0.20, scr_h * 0.06),
    [this] () {
        leave();
    },
    "Back", font_main
    );
    menu_widgets.push_back(back_widget);
    
    fullscreen_widget =
        new menu_checkbox(
        point(scr_w * 0.25, scr_h * 0.20),
        point(scr_w * 0.45, scr_h * 0.08),
    [this] () {
        intended_scr_fullscreen = this->fullscreen_widget->checked;
        warning_widget->enabled = true;
        update();
    },
    "Fullscreen", font_main
    );
    menu_widgets.push_back(fullscreen_widget);
    
    menu_widgets.push_back(
        new menu_button(
            point(scr_w * 0.05, scr_h * 0.30),
            point(scr_w * 0.05, scr_h * 0.08),
    [this] () {
        change_resolution(-1);
    },
    "<", font_main
        )
    );
    
    resolution_widget =
        new menu_text(
        point(scr_w * 0.26, scr_h * 0.30),
        point(scr_w * 0.35, scr_h * 0.08),
        "Resolution: ", font_main,
        al_map_rgb(255, 255, 255), ALLEGRO_ALIGN_LEFT
    );
    menu_widgets.push_back(resolution_widget);
    
    menu_widgets.push_back(
        new menu_button(
            point(scr_w * 0.45, scr_h * 0.30),
            point(scr_w * 0.05, scr_h * 0.08),
    [this] () {
        change_resolution(1);
    },
    ">", font_main
        )
    );
    
    menu_widgets.push_back(
        new menu_button(
            point(scr_w * 0.25, scr_h * 0.40),
            point(scr_w * 0.45, scr_h * 0.08),
    [this] () {
        go_to_controls();
    },
    "Edit controls...", font_main,
    al_map_rgb(255, 255, 255), ALLEGRO_ALIGN_LEFT
        )
    );
    
    warning_widget =
        new menu_text(
        point(scr_w * 0.50, scr_h * 0.95),
        point(scr_w * 0.95, scr_h * 0.10),
        "Please restart for the changes to take effect.", font_main
    );
    warning_widget->enabled = false;
    menu_widgets.push_back(warning_widget);
    
    //Finishing touches.
    fade_mgr.start_fade(true, nullptr);
    set_selected_widget(menu_widgets[0]);
    update();
    
}


/* ----------------------------------------------------------------------------
 * Unloads the options menu from memory.
 */
void options_menu::unload() {

    //Resources.
    al_destroy_bitmap(bmp_menu_bg);
    
    //Menu widgets.
    set_selected_widget(NULL);
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        delete menu_widgets[w];
    }
    menu_widgets.clear();
    
}


/* ----------------------------------------------------------------------------
 * Handles Allegro events.
 */
void options_menu::handle_controls(const ALLEGRO_EVENT &ev) {
    if(fade_mgr.is_fading()) return;
    
    handle_widget_events(ev);
}


/* ----------------------------------------------------------------------------
 * Changes to the next resolution preset in the list.
 */
void options_menu::change_resolution(const signed int step) {
    size_t current_r_index = INVALID;
    
    for(size_t r = 0; r < resolution_presets.size(); ++r) {
        if(
            intended_scr_w == resolution_presets[r].first &&
            intended_scr_h == resolution_presets[r].second
        ) {
            current_r_index = r;
            break;
        }
    }
    
    if(current_r_index == INVALID) {
        current_r_index = 0;
    } else {
        current_r_index =
            sum_and_wrap(current_r_index, step, resolution_presets.size());
    }
    
    intended_scr_w = resolution_presets[current_r_index].first;
    intended_scr_h = resolution_presets[current_r_index].second;
    
    if(!warning_widget->enabled) {
        warning_widget->enabled = true;
        warning_widget->start_juicy_grow();
    }
    resolution_widget->start_juicy_grow();
    update();
}


/* ----------------------------------------------------------------------------
 * Ticks one frame's worth of logic.
 */
void options_menu::do_logic() {
    time_spent += delta_t;
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->tick(delta_t);
    }
    fade_mgr.tick(delta_t);
}


/* ----------------------------------------------------------------------------
 * Draws the options menu.
 */
void options_menu::do_drawing() {
    al_clear_to_color(al_map_rgb(0, 0, 0));
    
    draw_bitmap(
        bmp_menu_bg, point(scr_w * 0.5, scr_h * 0.5),
        point(scr_w, scr_h), 0, map_gray(64)
    );
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->draw(time_spent);
    }
    
    fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Goes to the controls menu.
 */
void options_menu::go_to_controls() {
    fade_mgr.start_fade(false, [] () {
        change_game_state(GAME_STATE_CONTROLS_MENU);
    });
}


/* ----------------------------------------------------------------------------
 * Updates the contents of the options menu.
 */
void options_menu::update() {
    size_t current_r_index = INVALID;
    
    for(size_t r = 0; r < resolution_presets.size(); ++r) {
        if(
            intended_scr_w == resolution_presets[r].first &&
            intended_scr_h == resolution_presets[r].second
        ) {
            current_r_index = r;
            break;
        }
    }
    
    if(current_r_index == INVALID) {
        resolution_widget->text = "Resolution: Custom";
    } else {
        resolution_widget->text =
            "Resolution: " +
            i2s(intended_scr_w) + "x" +
            i2s(intended_scr_h);
    }
    
    fullscreen_widget->checked = intended_scr_fullscreen;
}


/* ----------------------------------------------------------------------------
 * Leaves the options menu and goes to the main menu.
 */
void options_menu::leave() {
    fade_mgr.start_fade(false, [] () {
        change_game_state(GAME_STATE_MAIN_MENU);
    });
    save_options();
}


/* ----------------------------------------------------------------------------
 * Creates a "controls menu" state.
 */
controls_menu::controls_menu() :
    game_state(),
    bmp_menu_bg(NULL),
    time_spent(0),
    cur_player_nr(0),
    cur_page_nr(0),
    cur_player_nr_widget(NULL),
    cur_page_nr_widget(NULL),
    input_capture_msg_widget(NULL),
    capturing_input(false),
    input_capture_control_nr(0) {
    
}


/* ----------------------------------------------------------------------------
 * Loads the controls menu into memory.
 */
void controls_menu::load() {
    selected_widget = NULL;
    bmp_menu_bg = NULL;
    cur_page_nr = 0;
    cur_player_nr = 0;
    capturing_input = false;
    
    //Resources.
    bmp_menu_bg = load_bmp(asset_file_names.main_menu);
    
    //Menu widgets.
    menu_widgets.push_back(
        new menu_text(
            point(scr_w * 0.45, scr_h * 0.10),
            point(scr_w * 0.20, scr_h * 0.08),
            "Player:", font_main
        )
    );
    
    menu_widgets.push_back(
        new menu_button(
            point(scr_w * 0.60, scr_h * 0.10),
            point(scr_w * 0.15, scr_h * 0.08),
    [this] () {
        cur_page_nr = 0;
        cur_player_nr = sum_and_wrap(cur_player_nr, -1, MAX_PLAYERS);
        cur_player_nr_widget->start_juicy_grow();
        update();
    },
    "<", font_main
        )
    );
    
    cur_player_nr_widget =
        new menu_text(
        point(scr_w * 0.70, scr_h * 0.10),
        point(scr_w * 0.10, scr_h * 0.08),
        "", font_main
    );
    menu_widgets.push_back(cur_player_nr_widget);
    
    menu_widgets.push_back(
        new menu_button(
            point(scr_w * 0.80, scr_h * 0.10),
            point(scr_w * 0.15, scr_h * 0.08),
    [this] () {
        cur_page_nr = 0;
        cur_player_nr = sum_and_wrap(cur_player_nr, 1, MAX_PLAYERS);
        cur_player_nr_widget->start_juicy_grow();
        update();
    },
    ">", font_main
        )
    );
    
    back_widget =
        new menu_button(
        point(scr_w * 0.15, scr_h * 0.10),
        point(scr_w * 0.20, scr_h * 0.08),
    [this] () {
        leave();
    },
    "Back", font_main
    );
    menu_widgets.push_back(back_widget);
    
    for(size_t c = 0; c < 8; c++) {
        control_widgets.push_back(
            new menu_button(
                point(scr_w * 0.07, scr_h * (0.20 + 0.08 * c)),
                point(scr_w * 0.08, scr_h * 0.07),
        [] () { }, "-", font_main
            )
        );
        menu_widgets.push_back(control_widgets.back());
        control_widgets.push_back(
            new menu_button(
                point(scr_w * 0.16, scr_h * (0.20 + 0.08 * c)),
                point(scr_w * 0.08, scr_h * 0.07),
        [] () { }, "<", font_main
            )
        );
        menu_widgets.push_back(control_widgets.back());
        control_widgets.push_back(
            new menu_text(
                point(scr_w * 0.40, scr_h * (0.20 + 0.08 * c)),
                point(scr_w * 0.39, scr_h * 0.07),
                "", font_main, al_map_rgb(255, 255, 255),
                ALLEGRO_ALIGN_LEFT
            )
        );
        menu_widgets.push_back(control_widgets.back());
        control_widgets.push_back(
            new menu_button(
                point(scr_w * 0.65, scr_h * (0.20 + 0.08 * c)),
                point(scr_w * 0.08, scr_h * 0.07),
        [] () { }, ">", font_main
            )
        );
        menu_widgets.push_back(control_widgets.back());
        control_widgets.push_back(
            new menu_button(
                point(scr_w * 0.83, scr_h * (0.20 + 0.08 * c)),
                point(scr_w * 0.26, scr_h * 0.07),
        [] () { }, "", font_main
            )
        );
        menu_widgets.push_back(control_widgets.back());
        
    }
    
    bottom_widgets.push_back(
        new menu_button(
            point(scr_w * 0.85, scr_h * 0.90),
            point(scr_w * 0.20, scr_h * 0.07),
    [this] () {
        if(controls[cur_player_nr].size()) {
            size_t last_action =
                controls[cur_player_nr].back().action;
            controls[cur_player_nr].push_back(
                control_info(
                    last_action == N_BUTTONS - 1 ?
                    1 : //The "None" action is 0, so go to 1.
                    last_action + 1,
                    ""
                )
            );
        } else {
            controls[cur_player_nr].push_back(
                control_info(BUTTON_NONE, "")
            );
        }
        //Go to the new control's page.
        cur_page_nr = controls[cur_player_nr].size() / 8.0f;
        this->control_widgets[
        ((controls[cur_player_nr].size() - 1) % 8) * 5 + 2
        ]->start_juicy_grow();
        update();
    },
    "New", font_main
        )
    );
    menu_widgets.push_back(bottom_widgets.back());
    bottom_widgets.push_back(
        new menu_text(
            point(scr_w * 0.15, scr_h * 0.90),
            point(scr_w * 0.20, scr_h * 0.08),
            "Page:", font_main
        )
    );
    menu_widgets.push_back(bottom_widgets.back());
    bottom_widgets.push_back(
        new menu_button(
            point(scr_w * 0.30, scr_h * 0.90),
            point(scr_w * 0.15, scr_h * 0.08),
    [this] () {
        cur_page_nr =
            sum_and_wrap(
                cur_page_nr, -1, ceil(controls[cur_player_nr].size() / 8.0)
            );
        cur_page_nr_widget->start_juicy_grow();
        update();
    },
    "<", font_main
        )
    );
    menu_widgets.push_back(bottom_widgets.back());
    cur_page_nr_widget =
        new menu_text(
        point(scr_w * 0.40, scr_h * 0.90),
        point(scr_w * 0.10, scr_h * 0.08),
        "", font_main
    );
    bottom_widgets.push_back(cur_page_nr_widget);
    menu_widgets.push_back(bottom_widgets.back());
    bottom_widgets.push_back(
        new menu_button(
            point(scr_w * 0.50, scr_h * 0.90),
            point(scr_w * 0.15, scr_h * 0.08),
    [this] () {
        cur_page_nr =
            sum_and_wrap(
                cur_page_nr, 1,
                (size_t) ceil(controls[cur_player_nr].size() / 8.0)
            );
        cur_page_nr_widget->start_juicy_grow();
        update();
    },
    ">", font_main
        )
    );
    menu_widgets.push_back(bottom_widgets.back());
    input_capture_msg_widget =
        new menu_text(
        point(scr_w * 0.50, scr_h * 0.90),
        point(scr_w * 1.00, scr_h * 0.08),
        "Waiting for any input...", font_main
    );
    menu_widgets.push_back(input_capture_msg_widget);
    
    //Finishing touches.
    fade_mgr.start_fade(true, nullptr);
    set_selected_widget(menu_widgets[1]);
    update();
    
    al_reconfigure_joysticks();
    
}


/* ----------------------------------------------------------------------------
 * Unloads the controls menu from memory.
 */
void controls_menu::unload() {

    //Resources.
    al_destroy_bitmap(bmp_menu_bg);
    
    //Menu widgets.
    set_selected_widget(NULL);
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        delete menu_widgets[w];
    }
    menu_widgets.clear();
    control_widgets.clear();
    bottom_widgets.clear();
    cur_player_nr_widget = NULL;
    cur_page_nr_widget = NULL;
    input_capture_msg_widget = NULL;
    
}


/* ----------------------------------------------------------------------------
 * Handles Allegro events.
 */
void controls_menu::handle_controls(const ALLEGRO_EVENT &ev) {
    if(fade_mgr.is_fading()) return;
    
    if(capturing_input) {
    
        control_info* c_ptr =
            &controls[cur_player_nr][input_capture_control_nr];
        bool valid = true;
        
        if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            c_ptr->type = CONTROL_TYPE_KEYBOARD_KEY;
            c_ptr->button = ev.keyboard.keycode;
            
        } else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            c_ptr->type = CONTROL_TYPE_MOUSE_BUTTON;
            c_ptr->button = ev.mouse.button;
            
        } else if(ev.type == ALLEGRO_EVENT_MOUSE_AXES) {
            if(ev.mouse.dz > 0) {
                c_ptr->type = CONTROL_TYPE_MOUSE_WHEEL_UP;
            } else if(ev.mouse.dz < 0) {
                c_ptr->type = CONTROL_TYPE_MOUSE_WHEEL_DOWN;
            } else if(ev.mouse.dw > 0) {
                c_ptr->type = CONTROL_TYPE_MOUSE_WHEEL_RIGHT;
            } else if(ev.mouse.dw < 0) {
                c_ptr->type = CONTROL_TYPE_MOUSE_WHEEL_LEFT;
            } else {
                valid = false;
            }
            
        } else if(ev.type == ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN) {
            c_ptr->type = CONTROL_TYPE_JOYSTICK_BUTTON;
            c_ptr->device_nr = joystick_numbers[ev.joystick.id];
            c_ptr->button = ev.joystick.button;
            
        } else if(ev.type == ALLEGRO_EVENT_JOYSTICK_AXIS) {
            c_ptr->type =
                (
                    ev.joystick.pos > 0 ?
                    CONTROL_TYPE_JOYSTICK_AXIS_POS :
                    CONTROL_TYPE_JOYSTICK_AXIS_NEG
                );
            c_ptr->device_nr = joystick_numbers[ev.joystick.id];
            c_ptr->stick = ev.joystick.stick;
            c_ptr->axis = ev.joystick.axis;
            
        } else {
            valid = false;
            
        }
        
        if(valid) {
            capturing_input = false;
            update();
        }
        
    } else {
    
        handle_widget_events(ev);
        
    }
    
}


/* ----------------------------------------------------------------------------
 * Ticks one frame's worth of logic.
 */
void controls_menu::do_logic() {
    fade_mgr.tick(delta_t);
    time_spent += delta_t;
    
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->tick(delta_t);
    }
}


/* ----------------------------------------------------------------------------
 * Draws the controls menu.
 */
void controls_menu::do_drawing() {
    al_clear_to_color(al_map_rgb(0, 0, 0));
    
    draw_bitmap(
        bmp_menu_bg, point(scr_w * 0.5, scr_h * 0.5),
        point(scr_w, scr_h), 0, map_gray(64)
    );
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->draw(time_spent);
    }
    
    size_t control_nr = cur_page_nr * 8;
    size_t list_nr = 0;
    for(
        ;
        list_nr < 8 && control_nr < controls[cur_player_nr].size();
        ++control_nr, ++list_nr
    ) {
        control_info* c_ptr = &controls[cur_player_nr][control_nr];
        
        draw_control(
            font_main, *c_ptr,
            point(scr_w * 0.83, scr_h * (0.2 + 0.08 * list_nr)),
            point(scr_w * 0.23, scr_h * 0.07)
        );
    }
    
    fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Updates the contents of the controls menu.
 */
void controls_menu::update() {
    cur_page_nr =
        min(
            cur_page_nr,
            (size_t) (ceil(controls[cur_player_nr].size() / 8.0) - 1)
        );
    cur_player_nr_widget->text = i2s(cur_player_nr + 1);
    cur_page_nr_widget->text = i2s(cur_page_nr + 1);
    
    for(size_t cw = 0; cw < control_widgets.size(); ++cw) {
        control_widgets[cw]->enabled = false;
    }
    
    size_t control_nr = cur_page_nr * 8;
    size_t list_nr = 0;
    for(
        ;
        list_nr < 8 && control_nr < controls[cur_player_nr].size();
        ++control_nr, ++list_nr
    ) {
        control_info* c_ptr = &controls[cur_player_nr][control_nr];
        
        string action_name;
        for(size_t b = 0; b < N_BUTTONS; ++b) {
            if(c_ptr->action == buttons.list[b].id) {
                action_name = buttons.list[b].name;
                break;
            }
        }
        
        for(size_t cw = 0; cw < 5; ++cw) {
            control_widgets[list_nr * 5 + cw]->enabled = true;
        }
        //Delete button.
        ((menu_button*) control_widgets[list_nr * 5 + 0])->click_handler =
        [this, control_nr] () {
            controls[cur_player_nr].erase(
                controls[cur_player_nr].begin() + control_nr
            );
            update();
        };
        //Previous action.
        ((menu_button*) control_widgets[list_nr * 5 + 1])->click_handler =
        [this, c_ptr, list_nr] () {
            c_ptr->action = sum_and_wrap(c_ptr->action, -1, N_BUTTONS);
            control_widgets[list_nr * 5 + 2]->start_juicy_grow();
            update();
        };
        //Action name.
        ((menu_text*) control_widgets[list_nr * 5 + 2])->text = action_name;
        //Next action.
        ((menu_button*) control_widgets[list_nr * 5 + 3])->click_handler =
        [this, c_ptr, list_nr] () {
            c_ptr->action = sum_and_wrap(c_ptr->action, 1, N_BUTTONS);
            control_widgets[list_nr * 5 + 2]->start_juicy_grow();
            update();
        };
        //Set button.
        ((menu_button*) control_widgets[list_nr * 5 + 4])->click_handler =
        [this, control_nr] () {
            capturing_input = true;
            input_capture_control_nr = control_nr;
            update();
        };
        
        
    }
    
    //Show or hide the "please press something" message.
    if(capturing_input) {
        input_capture_msg_widget->enabled = true;
        for(size_t bw = 0; bw < bottom_widgets.size(); ++bw) {
            bottom_widgets[bw]->enabled = false;
        }
        
    } else {
        input_capture_msg_widget->enabled = false;
        for(size_t bw = 0; bw < bottom_widgets.size(); ++bw) {
            bottom_widgets[bw]->enabled = true;
        }
        
    }
}


/* ----------------------------------------------------------------------------
 * Leaves the controls menu and goes to the options menu.
 */
void controls_menu::leave() {
    fade_mgr.start_fade(false, [] () {
        change_game_state(GAME_STATE_OPTIONS_MENU);
    });
    save_options();
}


/* ----------------------------------------------------------------------------
 * Creates an "area menu" state.
 */
area_menu::area_menu() :
    game_state(),
    bmp_menu_bg(NULL),
    time_spent(0),
    cur_page_nr(0),
    cur_page_nr_widget(NULL) {
    
}


/* ----------------------------------------------------------------------------
 * Loads the area menu into memory.
 */
void area_menu::load() {
    selected_widget = NULL;
    bmp_menu_bg = NULL;
    time_spent = 0;
    cur_page_nr = 0;
    
    //Areas.
    areas_to_pick = folder_to_vector(AREAS_FOLDER_PATH, true);
    
    //If there's only one area, go there right away.
    if(areas_to_pick.size() == 1) {
        area_to_load = areas_to_pick[0];
        change_game_state(GAME_STATE_GAME);
        return;
    }
    
    for(size_t a = 0; a < areas_to_pick.size(); ++a) {
        string actual_name = areas_to_pick[a];
        data_node data(AREAS_FOLDER_PATH + "/" + actual_name + "/Data.txt");
        if(data.file_was_opened) {
            string s = data.get_child_by_name("name")->value;
            if(!s.empty()) {
                actual_name = s;
            }
        }
        
        area_names.push_back(actual_name);
    }
    
    //Resources.
    bmp_menu_bg = load_bmp(asset_file_names.main_menu);
    
    //Menu widgets.
    back_widget =
        new menu_button(
        point(scr_w * 0.15, scr_h * 0.10),
        point(scr_w * 0.20, scr_h * 0.06),
    [this] () {
        leave();
    },
    "Back", font_main
    );
    menu_widgets.push_back(back_widget);
    
    menu_widgets.push_back(
        new menu_text(
            point(scr_w * 0.5, scr_h * 0.1), point(scr_w * 0.3, scr_h * 0.1),
            "Pick an area:",
            font_main, al_map_rgb(255, 255, 255), ALLEGRO_ALIGN_CENTER
        )
    );
    
    for(size_t a = 0; a < 8; ++a) {
        menu_widgets.push_back(
            new menu_button(
                point(scr_w * 0.5, scr_h * (0.2 + 0.08 * a)),
                point(scr_w * 0.8, scr_h * 0.06),
        [] () {
        
        },
        "", font_area_name
            )
        );
        area_buttons.push_back(menu_widgets.back());
    }
    
    menu_widgets.push_back(
        new menu_text(
            point(scr_w * 0.15, scr_h * 0.9), point(scr_w * 0.2, scr_h * 0.1),
            "Page:", font_main
        )
    );
    menu_widgets.push_back(
        new menu_button(
            point(scr_w * 0.3, scr_h * 0.9), point(scr_w * 0.15, scr_h * 0.1),
    [this] () {
        cur_page_nr =
            sum_and_wrap(
                cur_page_nr, -1, ceil(areas_to_pick.size() / 8.0)
            );
        cur_page_nr_widget->start_juicy_grow();
        update();
    },
    "<", font_main
        )
    );
    cur_page_nr_widget =
        new menu_text(
        point(scr_w * 0.4, scr_h * 0.9), point(scr_w * 0.1, scr_h * 0.1),
        "", font_main
    );
    menu_widgets.push_back(cur_page_nr_widget);
    menu_widgets.push_back(
        new menu_button(
            point(scr_w * 0.5, scr_h * 0.9), point(scr_w * 0.15, scr_h * 0.1),
    [this] () {
        cur_page_nr =
            sum_and_wrap(
                cur_page_nr, 1, ceil(areas_to_pick.size() / 8.0)
            );
        cur_page_nr_widget->start_juicy_grow();
        update();
    },
    ">", font_main
        )
    );
    
    //Finishing touches.
    fade_mgr.start_fade(true, nullptr);
    update();
    if(menu_widgets.size() >= 3) {
        set_selected_widget(menu_widgets[2]);
    } else {
        set_selected_widget(menu_widgets[1]);
    }
    
}


/* ----------------------------------------------------------------------------
 * Unloads the area menu from memory.
 */
void area_menu::unload() {

    //Resources.
    al_destroy_bitmap(bmp_menu_bg);
    
    //Menu widgets.
    set_selected_widget(NULL);
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        delete menu_widgets[w];
    }
    menu_widgets.clear();
    area_buttons.clear();
    areas_to_pick.clear();
    area_names.clear();
    cur_page_nr_widget = NULL;
    
}


/* ----------------------------------------------------------------------------
 * Handles Allegro events.
 */
void area_menu::handle_controls(const ALLEGRO_EVENT &ev) {
    if(fade_mgr.is_fading()) return;
    
    handle_widget_events(ev);
    
}


/* ----------------------------------------------------------------------------
 * Ticks one frame's worth of logic.
 */
void area_menu::do_logic() {
    fade_mgr.tick(delta_t);
    time_spent += delta_t;
    
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->tick(delta_t);
    }
}


/* ----------------------------------------------------------------------------
 * Draws the area menu.
 */
void area_menu::do_drawing() {
    al_clear_to_color(al_map_rgb(0, 0, 0));
    draw_bitmap(
        bmp_menu_bg, point(scr_w * 0.5, scr_h * 0.5),
        point(scr_w, scr_h), 0, map_gray(64)
    );
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->draw(time_spent);
    }
    
    fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Leaves the area menu and goes into the main menu.
 */
void area_menu::leave() {
    fade_mgr.start_fade(false, [] () {
        change_game_state(GAME_STATE_MAIN_MENU);
    });
}


/* ----------------------------------------------------------------------------
 * Updates the contents of the area menu.
 */
void area_menu::update() {
    cur_page_nr =
        min(cur_page_nr, (size_t) (ceil(areas_to_pick.size() / 8.0) - 1));
    cur_page_nr_widget->text = i2s(cur_page_nr + 1);
    
    for(size_t aw = 0; aw < area_buttons.size(); ++aw) {
        area_buttons[aw]->enabled = false;
    }
    
    size_t area_nr = cur_page_nr * 8;
    size_t list_nr = 0;
    for(; list_nr < 8 && area_nr < areas_to_pick.size(); ++area_nr, ++list_nr) {
        string area_name = area_names[area_nr];
        string area_folder = areas_to_pick[area_nr];
        
        ((menu_button*) area_buttons[list_nr])->click_handler =
        [area_name, area_folder] () {
            area_to_load = area_folder;
            fade_mgr.start_fade(false, [] () {
                change_game_state(GAME_STATE_GAME);
            });
        };
        ((menu_button*) area_buttons[list_nr])->text = area_name;
        area_buttons[list_nr]->enabled = true;
        
    }
}
