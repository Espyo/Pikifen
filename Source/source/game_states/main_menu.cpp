/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Main menu state class and main menu state-related functions.
 */

#include <algorithm>

#include "menus.h"

#include "../drawing.h"
#include "../functions.h"
#include "../game.h"
#include "../load.h"
#include "../utils/string_utils.h"


//Path to the GUI information file.
const string main_menu_state::GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Main_menu.txt";


/* ----------------------------------------------------------------------------
 * Creates a "main menu" state.
 */
main_menu_state::main_menu_state() :
    game_state(),
    bmp_menu_bg(NULL),
    logo_min_screen_limit(10.0f, 10.0f),
    logo_max_screen_limit(90.0f, 50.0f),
    logo_pikmin_max_speed(800.0f),
    logo_pikmin_min_speed(600.0f),
    logo_pikmin_speed_smoothness(0.08f),
    logo_pikmin_sway_amount(3.0f),
    logo_pikmin_sway_max_speed(5.5f),
    logo_pikmin_sway_min_speed(2.5f),
    logo_pikmin_size(3.5f, 3.5f) {
    
}


/* ----------------------------------------------------------------------------
 * Draws the main menu.
 */
void main_menu_state::do_drawing() {
    al_clear_to_color(COLOR_BLACK);
    draw_bitmap(
        bmp_menu_bg, point(game.win_w * 0.5, game.win_h * 0.5),
        point(game.win_w, game.win_h)
    );
    
    //Draw the logo Pikmin.
    point pik_size = logo_pikmin_size;
    pik_size.x *= game.win_w / 100.0f;
    pik_size.y *= game.win_h / 100.0f;
    
    for(size_t p = 0; p < logo_pikmin.size(); ++p) {
        logo_pik* pik = &logo_pikmin[p];
        
        draw_bitmap_in_box(pik->top, pik->pos, pik_size, pik->angle);
    }
    
    draw_scaled_text(
        game.fonts.standard, COLOR_WHITE,
        point(8, game.win_h  - 8),
        point(0.6, 0.6),
        ALLEGRO_ALIGN_LEFT, TEXT_VALIGN_BOTTOM,
        "Pikmin (c) Nintendo"
    );
    string version_text;
    if(!game.config.name.empty()) {
        version_text = game.config.name;
        if(!game.config.version.empty()) {
            version_text += " " + game.config.version;
        }
        version_text += ", powered by ";
    }
    version_text +=
        "Pikifen " +
        i2s(VERSION_MAJOR) + "." + i2s(VERSION_MINOR)  + "." + i2s(VERSION_REV);
    draw_scaled_text(
        game.fonts.standard, COLOR_WHITE,
        point(game.win_w - 8, game.win_h  - 8),
        point(0.6, 0.6),
        ALLEGRO_ALIGN_RIGHT, TEXT_VALIGN_BOTTOM,
        version_text
    );
    
    gui.draw();
    
    game.fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Ticks a frame's worth of logic.
 */
void main_menu_state::do_logic() {
    //Animate the logo Pikmin.
    for(size_t p = 0; p < logo_pikmin.size(); ++p) {
        logo_pik* pik = &logo_pikmin[p];
        
        if(!pik->reached_destination) {
            float a = get_angle(pik->pos, pik->destination);
            float speed =
                std::min(
                    (float) (pik->speed * game.delta_t),
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
            pik->sway_var += pik->sway_speed * game.delta_t;
            pik->pos.x =
                pik->destination.x +
                sin(pik->sway_var) * logo_pikmin_sway_amount;
        }
    }
    
    gui.tick(game.delta_t);
    
    //Fade manager needs to come last, because if
    //the fade finishes and the state changes, and
    //after that we still attempt to do stuff in
    //this function, we're going to have a bad time.
    game.fade_mgr.tick(game.delta_t);
    
}


/* ----------------------------------------------------------------------------
 * Returns the name of this state.
 */
string main_menu_state::get_name() const {
    return "main menu";
}


/* ----------------------------------------------------------------------------
 * Handles Allegro events.
 * ev:
 *   Event to handle.
 */
void main_menu_state::handle_allegro_event(ALLEGRO_EVENT &ev) {
    if(game.fade_mgr.is_fading()) return;
    
    gui.handle_event(ev);
}


/* ----------------------------------------------------------------------------
 * Loads the main menu into memory.
 */
void main_menu_state::load() {
    draw_loading_screen("", "", 1.0);
    al_flip_display();
    data_node settings_file(GUI_FILE_PATH);
    
    //Menu items.
    gui.register_coords("play",             50, 55, 50,  6);
    gui.register_coords("options",          50, 63, 50,  6);
    gui.register_coords("animation_editor", 50, 71, 50,  6);
    gui.register_coords("area_editor",      50, 79, 50,  6);
    gui.register_coords("exit",             50, 87, 50,  6);
    gui.register_coords("tooltip",          50, 95, 95,  8);
    gui.read_coords(settings_file.get_child_by_name("positions"));
    
    //Play button.
    button_gui_item* play_button =
        new button_gui_item("Play", game.fonts.area_name);
    play_button->on_activate =
    [] (const point &) {
        game.fade_mgr.start_fade(false, [] () {
            game.change_state(game.states.area_menu);
        });
    };
    play_button->on_get_tooltip =
    [] () { return "Pick an area, and start playing!"; };
    gui.add_item(play_button, "play");
    
    //Options button.
    button_gui_item* options_button =
        new button_gui_item("Options", game.fonts.area_name);
    options_button->on_activate =
    [] (const point &) {
        game.fade_mgr.start_fade(false, [] () {
            game.change_state(game.states.options_menu);
        });
    };
    options_button->on_get_tooltip =
    [] () { return "Customize your playing experience."; };
    gui.add_item(options_button, "options");
    
    //Animation editor button.
    button_gui_item* anim_ed_button =
        new button_gui_item("Animation editor", game.fonts.area_name);
    anim_ed_button->on_activate =
    [] (const point &) {
        game.fade_mgr.start_fade(false, [] () {
            game.change_state(game.states.animation_ed);
        });
    };
    anim_ed_button->on_get_tooltip =
    [] () { return "Make an animation for any object in the game."; };
    gui.add_item(anim_ed_button, "animation_editor");
    
    //Area editor button.
    button_gui_item* area_ed_button =
        new button_gui_item("Area editor", game.fonts.area_name);
    area_ed_button->on_activate =
    [] (const point &) {
        game.fade_mgr.start_fade(false, [] () {
            game.change_state(game.states.area_ed);
        });
    };
    area_ed_button->on_get_tooltip =
    [] () { return "Make an area to play on."; };
    gui.add_item(area_ed_button, "area_editor");
    
    //Exit button.
    gui.back_item =
        new button_gui_item("Exit", game.fonts.area_name);
    gui.back_item->on_activate =
    [] (const point &) {
        game.is_game_running = false;
    };
    gui.back_item->on_get_tooltip =
    [] () {
        return
            game.config.name.empty() ?
            "Quit Pikifen." :
            "Quit " + game.config.name + ".";
    };
    gui.add_item(gui.back_item, "exit");
    
    //Tooltip text.
    text_gui_item* tooltip_text =
        new text_gui_item("", game.fonts.standard);
    tooltip_text->on_draw =
        [this]
    (const point & center, const point & size) {
        draw_compressed_scaled_text(
            game.fonts.standard, COLOR_WHITE,
            center, point(0.7f, 0.7f),
            ALLEGRO_ALIGN_CENTER, TEXT_VALIGN_CENTER, size, false,
            gui.get_current_tooltip()
        );
    };
    gui.add_item(tooltip_text, "tooltip");
    
    //Resources.
    bmp_menu_bg = load_bmp(game.asset_file_names.main_menu);
    
    //Logo pikmin.
    data_node* logo_node = settings_file.get_child_by_name("logo");
    reader_setter logo_rs(logo_node);
    
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
            std::max(map_total_cols, map_node->get_child(r)->name.size());
    }
    
    logo_rs.set("min_screen_limit", logo_min_screen_limit);
    logo_rs.set("max_screen_limit", logo_max_screen_limit);
    logo_rs.set("pikmin_max_speed", logo_pikmin_max_speed);
    logo_rs.set("pikmin_min_speed", logo_pikmin_min_speed);
    logo_rs.set("pikmin_speed_smoothness", logo_pikmin_speed_smoothness);
    logo_rs.set("pikmin_sway_amount", logo_pikmin_sway_amount);
    logo_rs.set("pikmin_sway_max_speed", logo_pikmin_sway_max_speed);
    logo_rs.set("pikmin_sway_min_speed", logo_pikmin_sway_min_speed);
    logo_rs.set("pikmin_size", logo_pikmin_size);
    
    bool map_ok = true;
    
    for(size_t r = 0; r < map_total_rows; ++r) {
        string row = map_node->get_child(r)->name;
        
        for(size_t c = 0; c < row.size(); ++c) {
            if(row[c] == '.') continue;
            if(logo_type_bitmaps.find(row[c]) == logo_type_bitmaps.end()) {
                map_ok = false;
                log_error(
                    "Title screen Pikmin logo map has an unknown character \"" +
                    string(1, row[c]) + "\" on row " + i2s(r + 1) +
                    ", column " + i2s(c + 1) + "!"
                );
                break;
            }
            
            logo_pik pik;
            
            point min_pos = logo_min_screen_limit;
            min_pos.x *= game.win_w / 100.0f;
            min_pos.y *= game.win_h / 100.0f;
            point max_pos = logo_max_screen_limit;
            max_pos.x *= game.win_w / 100.0f;
            max_pos.y *= game.win_h / 100.0f;
            
            pik.top = logo_type_bitmaps[row[c]];
            pik.destination =
                point(
                    min_pos.x +
                    (max_pos.x - min_pos.x) *
                    (c / (float) map_total_cols),
                    min_pos.y +
                    (max_pos.y - min_pos.y) *
                    (r / (float) map_total_rows)
                );
                
            unsigned char h_side = randomi(0, 1);
            unsigned char v_side = randomi(0, 1);
            
            pik.pos =
                point(
                    randomf(0, game.win_w * 0.5),
                    randomf(0, game.win_h * 0.5)
                );
                
            if(h_side == 0) {
                pik.pos.x -= game.win_w * 1.2;
            } else {
                pik.pos.x += game.win_w * 1.2;
            }
            if(v_side == 0) {
                pik.pos.y -= game.win_h * 1.2;
            } else {
                pik.pos.y += game.win_h * 1.2;
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
    gui.set_selected_item(play_button);
    game.fade_mgr.start_fade(true, nullptr);
    
}


/* ----------------------------------------------------------------------------
 * Unloads the main menu from memory.
 */
void main_menu_state::unload() {

    //Resources.
    al_destroy_bitmap(bmp_menu_bg);
    
    //Menu items.
    gui.destroy();
    
    //Misc.
    logo_pikmin.clear();
    
}
