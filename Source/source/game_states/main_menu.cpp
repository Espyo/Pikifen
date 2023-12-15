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


namespace MAIN_MENU {
//Path to the GUI information file.
const string GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Main_menu.txt";
//How long the menu items take to move when switching pages.
const float HUD_MOVE_TIME = 0.5f;
//Path to the make page GUI information file.
const string MAKE_GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Main_menu_make.txt";
//Path to the play page GUI information file.
const string PLAY_GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Main_menu_play.txt";
//Path to the tutorial question page GUI information file.
const string TUTORIAL_GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Main_menu_tutorial.txt";
}


/* ----------------------------------------------------------------------------
 * Creates a "main menu" state.
 */
main_menu_state::main_menu_state() :
    game_state(),
    page_to_load(MAIN_MENU_PAGE_MAIN),
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
        
        draw_bitmap_in_box(pik->top, pik->pos, pik_size, pik->angle, true);
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
        "Pikifen " + get_engine_version_string();
    draw_scaled_text(
        game.fonts.standard, COLOR_WHITE,
        point(game.win_w - 8, game.win_h  - 8),
        point(0.6, 0.6),
        ALLEGRO_ALIGN_RIGHT, TEXT_VALIGN_BOTTOM,
        version_text
    );
    
    main_gui.draw();
    play_gui.draw();
    make_gui.draw();
    tutorial_gui.draw();
    
    draw_mouse_cursor(GAME::CURSOR_STANDARD_COLOR);
    
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
    
    vector<player_action> player_actions = game.controls.new_frame();
    for(size_t a = 0; a < player_actions.size(); ++a) {
        main_gui.handle_player_action(player_actions[a]);
        play_gui.handle_player_action(player_actions[a]);
        make_gui.handle_player_action(player_actions[a]);
        tutorial_gui.handle_player_action(player_actions[a]);
    }
    
    main_gui.tick(game.delta_t);
    play_gui.tick(game.delta_t);
    make_gui.tick(game.delta_t);
    tutorial_gui.tick(game.delta_t);
    
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
    
    main_gui.handle_event(ev);
    play_gui.handle_event(ev);
    make_gui.handle_event(ev);
    tutorial_gui.handle_event(ev);
    
    game.controls.handle_allegro_event(ev);
}


/* ----------------------------------------------------------------------------
 * Loads the GUI elements for the main menu's main page.
 */
void main_menu_state::init_main_page() {

    data_node gui_file(MAIN_MENU::GUI_FILE_PATH);
    
    //Menu items.
    main_gui.register_coords("play",    50, 58, 60, 12);
    main_gui.register_coords("make",    50, 72, 60, 12);
    main_gui.register_coords("options", 31, 83, 34,  6);
    main_gui.register_coords("stats",   69, 83, 34,  6);
    main_gui.register_coords("exit",    91, 91, 14,  6);
    main_gui.register_coords("tooltip", 50, 96, 96,  4);
    main_gui.read_coords(gui_file.get_child_by_name("positions"));
    
    //Play button.
    button_gui_item* play_button =
        new button_gui_item("Play", game.fonts.area_name);
    play_button->on_activate =
    [this] (const point &) {
        main_gui.responsive = false;
        main_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            MAIN_MENU::HUD_MOVE_TIME
        );
        if(game.statistics.area_entries == 0) {
            tutorial_gui.responsive = true;
            tutorial_gui.start_animation(
                GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
                MAIN_MENU::HUD_MOVE_TIME
            );
        } else {
            play_gui.responsive = true;
            play_gui.start_animation(
                GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
                MAIN_MENU::HUD_MOVE_TIME
            );
        }
    };
    play_button->on_get_tooltip =
    [] () { return "Choose an area to play in."; };
    main_gui.add_item(play_button, "play");
    
    //Make button.
    button_gui_item* make_button =
        new button_gui_item("Make", game.fonts.area_name);
    make_button->on_activate =
    [this] (const point &) {
        main_gui.responsive = false;
        main_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            MAIN_MENU::HUD_MOVE_TIME
        );
        make_gui.responsive = true;
        make_gui.start_animation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            MAIN_MENU::HUD_MOVE_TIME
        );
    };
    make_button->on_get_tooltip =
    [] () { return "Make your own content, like areas or animations."; };
    main_gui.add_item(make_button, "make");
    
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
    main_gui.add_item(options_button, "options");
    
    //Statistics button.
    button_gui_item* stats_button =
        new button_gui_item("Statistics", game.fonts.area_name);
    stats_button->on_activate =
    [] (const point &) {
        game.fade_mgr.start_fade(false, [] () {
            game.change_state(game.states.stats_menu);
        });
    };
    stats_button->on_get_tooltip =
    [] () { return "Check out some fun lifetime statistics."; };
    main_gui.add_item(stats_button, "stats");
    
    //Exit button.
    main_gui.back_item =
        new button_gui_item("Exit", game.fonts.area_name);
    main_gui.back_item->on_activate =
    [] (const point &) {
        save_statistics();
        game.is_game_running = false;
    };
    main_gui.back_item->on_get_tooltip =
    [] () {
        return
            game.config.name.empty() ?
            "Quit Pikifen." :
            "Quit " + game.config.name + ".";
    };
    main_gui.add_item(main_gui.back_item, "exit");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&main_gui);
    main_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    main_gui.set_selected_item(play_button);
    main_gui.responsive = false;
    main_gui.hide_items();
}


/* ----------------------------------------------------------------------------
 * Loads the GUI elements for the main menu's make page.
 */
void main_menu_state::init_make_page() {
    data_node gui_file(MAIN_MENU::MAKE_GUI_FILE_PATH);
    
    //Menu items.
    make_gui.register_coords("animation_editor", 50, 59,   60, 10);
    make_gui.register_coords("area_editor",      50, 71,   60, 10);
    make_gui.register_coords("gui_editor",       50, 81.5, 50,  7);
    make_gui.register_coords("back",              9, 91,   14,  6);
    make_gui.register_coords("more",             91, 91,   14,  6);
    make_gui.register_coords("tooltip",          50, 96,   96,  4);
    make_gui.read_coords(gui_file.get_child_by_name("positions"));
    
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
    make_gui.add_item(anim_ed_button, "animation_editor");
    
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
    make_gui.add_item(area_ed_button, "area_editor");
    
    //GUI editor button.
    button_gui_item* gui_ed_button =
        new button_gui_item("GUI editor", game.fonts.area_name);
    gui_ed_button->on_activate =
    [] (const point &) {
        game.fade_mgr.start_fade(false, [] () {
            game.change_state(game.states.gui_ed);
        });
    };
    gui_ed_button->on_get_tooltip =
    [] () { return "Change the way menus and the gameplay HUD look."; };
    make_gui.add_item(gui_ed_button, "gui_editor");
    
    //Back button.
    make_gui.back_item =
        new button_gui_item("Back", game.fonts.area_name);
    make_gui.back_item->on_activate =
    [this] (const point &) {
        make_gui.responsive = false;
        make_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            MAIN_MENU::HUD_MOVE_TIME
        );
        main_gui.responsive = true;
        main_gui.start_animation(
            GUI_MANAGER_ANIM_LEFT_TO_CENTER,
            MAIN_MENU::HUD_MOVE_TIME
        );
    };
    make_gui.back_item->on_get_tooltip =
    [] () {
        return "Return to the main page.";
    };
    make_gui.add_item(make_gui.back_item, "back");
    
    //More bullet point.
    bullet_point_gui_item* more_bullet =
        new bullet_point_gui_item("More...", game.fonts.standard);
    more_bullet->on_get_tooltip =
    [] () {
        return
            "For more help and more things that you can edit, "
            "check out the manual in the game's folder.";
    };
    make_gui.add_item(more_bullet, "more");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&make_gui);
    make_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    make_gui.set_selected_item(anim_ed_button);
    make_gui.responsive = false;
    make_gui.hide_items();
}


/* ----------------------------------------------------------------------------
 * Loads the GUI elements for the main menu's play page.
 */
void main_menu_state::init_play_page() {
    data_node gui_file(MAIN_MENU::PLAY_GUI_FILE_PATH);
    
    //Menu items.
    play_gui.register_coords("simple",  50, 60, 60, 12.5);
    play_gui.register_coords("mission", 50, 78, 60, 12.5);
    play_gui.register_coords("back",     9, 91, 14,    6);
    play_gui.register_coords("tooltip", 50, 96, 96,    4);
    play_gui.read_coords(gui_file.get_child_by_name("positions"));
    
    //Play a simple area button.
    button_gui_item* simple_button =
        new button_gui_item("Simple areas", game.fonts.area_name);
    simple_button->on_activate =
    [] (const point &) {
        game.fade_mgr.start_fade(false, [] () {
            game.states.area_menu->area_type = AREA_TYPE_SIMPLE;
            game.change_state(game.states.area_menu);
        });
    };
    simple_button->on_get_tooltip =
    [] () { return "Pick a simple area with no goal, and start playing!"; };
    play_gui.add_item(simple_button, "simple");
    
    //Play a mission area button.
    button_gui_item* mission_button =
        new button_gui_item("Missions", game.fonts.area_name);
    mission_button->on_activate =
    [] (const point &) {
        game.fade_mgr.start_fade(false, [] () {
            game.states.area_menu->area_type = AREA_TYPE_MISSION;
            game.change_state(game.states.area_menu);
        });
    };
    mission_button->on_get_tooltip =
    [] () {
        return
            "Pick a mission area with goals and limitations, "
            "and start playing!";
    };
    play_gui.add_item(mission_button, "mission");
    
    //Back button.
    play_gui.back_item =
        new button_gui_item("Back", game.fonts.area_name);
    play_gui.back_item->on_activate =
    [this] (const point &) {
        play_gui.responsive = false;
        play_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            MAIN_MENU::HUD_MOVE_TIME
        );
        main_gui.responsive = true;
        main_gui.start_animation(
            GUI_MANAGER_ANIM_LEFT_TO_CENTER,
            MAIN_MENU::HUD_MOVE_TIME
        );
    };
    play_gui.back_item->on_get_tooltip =
    [] () {
        return "Return to the main page.";
    };
    play_gui.add_item(play_gui.back_item, "back");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&play_gui);
    play_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    play_gui.set_selected_item(simple_button);
    play_gui.responsive = false;
    play_gui.hide_items();
}


/* ----------------------------------------------------------------------------
 * Loads the GUI elements for the main menu's tutorial question page.
 */
void main_menu_state::init_tutorial_page() {
    data_node gui_file(MAIN_MENU::TUTORIAL_GUI_FILE_PATH);
    
    //Menu items.
    tutorial_gui.register_coords("question", 50,     60, 60,  12.5);
    tutorial_gui.register_coords("no",       26, 80.875, 40, 10.25);
    tutorial_gui.register_coords("yes",      74,     81, 40,    10);
    tutorial_gui.register_coords("tooltip",  50,     96, 96,     4);
    tutorial_gui.read_coords(gui_file.get_child_by_name("positions"));
    
    //Question text.
    text_gui_item* question_text =
        new text_gui_item(
        "If you're new to Pikifen, it is recommended to play the "
        "\"Tutorial Meadow\" mission first.\n\n"
        "Do you want to play there now?",
        game.fonts.standard
    );
    question_text->line_wrap = true;
    tutorial_gui.add_item(question_text, "question");
    
    //No button.
    tutorial_gui.back_item =
        new button_gui_item("No", game.fonts.standard);
    tutorial_gui.back_item->on_activate =
    [this] (const point &) {
        tutorial_gui.responsive = false;
        tutorial_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            MAIN_MENU::HUD_MOVE_TIME
        );
        play_gui.responsive = true;
        play_gui.start_animation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            MAIN_MENU::HUD_MOVE_TIME
        );
    };
    tutorial_gui.back_item->on_get_tooltip =
    [] () {
        return
            "Go to the standard area selection menu.";
    };
    tutorial_gui.add_item(tutorial_gui.back_item, "no");
    
    //Yes button.
    button_gui_item* yes_button =
        new button_gui_item("Yes", game.fonts.standard);
    yes_button->on_activate =
    [] (const point &) {
        game.states.gameplay->path_of_area_to_load =
            get_base_area_folder_path(AREA_TYPE_MISSION, true) + "/" +
            "Tutorial Meadow";
        game.fade_mgr.start_fade(false, [] () {
            game.change_state(game.states.gameplay);
        });
    };
    yes_button->on_get_tooltip =
    [] () {
        return
            "Play Tutorial Meadow now.";
    };
    tutorial_gui.add_item(yes_button, "yes");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&tutorial_gui);
    tutorial_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    tutorial_gui.set_selected_item(yes_button);
    tutorial_gui.responsive = false;
    tutorial_gui.hide_items();
}


/* ----------------------------------------------------------------------------
 * Loads the main menu into memory.
 */
void main_menu_state::load() {
    draw_loading_screen("", "", 1.0);
    al_flip_display();
    
    init_main_page();
    init_play_page();
    init_make_page();
    init_tutorial_page();
    
    switch(page_to_load) {
    case MAIN_MENU_PAGE_MAIN: {
        main_gui.responsive = true;
        main_gui.show_items();
        break;
    } case MAIN_MENU_PAGE_PLAY: {
        play_gui.responsive = true;
        play_gui.show_items();
        break;
    } case MAIN_MENU_PAGE_MAKE: {
        make_gui.responsive = true;
        make_gui.show_items();
        break;
    }
    }
    page_to_load = MAIN_MENU_PAGE_MAIN;
    
    data_node settings_file(MAIN_MENU::GUI_FILE_PATH);
    
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
                game.errors.report(
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
    game.fade_mgr.start_fade(true, nullptr);
}


/* ----------------------------------------------------------------------------
 * Unloads the main menu from memory.
 */
void main_menu_state::unload() {

    //Resources.
    al_destroy_bitmap(bmp_menu_bg);
    
    //Menu items.
    main_gui.destroy();
    play_gui.destroy();
    make_gui.destroy();
    tutorial_gui.destroy();
    
    //Misc.
    logo_pikmin.clear();
    
}
