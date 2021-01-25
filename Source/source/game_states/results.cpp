/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Day results state class and results state-related functions.
 */

#include "results.h"

#include "../drawing.h"
#include "../game.h"
#include "../functions.h"
#include "../utils/string_utils.h"


//Path to the GUI information file.
const string results_state::GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Results_menu.txt";


/* ----------------------------------------------------------------------------
 * Creates a "results" state.
 */
results_state::results_state() :
    game_state(),
    can_continue(true),
    enemies_beaten(0),
    enemies_total(0),
    leader_ko(false),
    out_of_time(false),
    pikmin_born(0),
    pikmin_deaths(0),
    points_obtained(0),
    points_total(0),
    time_taken(0.0f),
    area_name_text(nullptr),
    enemies_text(nullptr),
    pikmin_born_text(nullptr),
    pikmin_deaths_text(nullptr),
    points_text(nullptr),
    time_text(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * Leaves the results menu and goes back to the gameplay state to continue
 * playing the area.
 */
void results_state::continue_playing() {
    game.fade_mgr.start_fade(false, [] () {
        game.change_state(game.states.gameplay, true, false);
        game.states.gameplay->enter();
    });
}


/* ----------------------------------------------------------------------------
 * Draws the results state.
 */
void results_state::do_drawing() {
    //Background.
    al_clear_to_color(al_map_rgb(143, 149, 62));
    
    float logo_width = al_get_bitmap_width(game.sys_assets.bmp_icon);
    float logo_height = al_get_bitmap_height(game.sys_assets.bmp_icon);
    logo_height = game.win_w * 0.08f * (logo_width / logo_height);
    logo_width = game.win_w * 0.08f;
    draw_background_logos(
        time_spent, 6, 6, point(logo_width, logo_height), map_alpha(75),
        point(-60.0f, 30.0f), -TAU / 6.0f
    );
    
    draw_filled_rounded_rectangle(
        point(game.win_w * 0.50f, game.win_h * 0.50f),
        point(game.win_w * 0.80f, game.win_h * 0.50f),
        20.0f,
        al_map_rgba(57, 54, 98, 48)
    );
    draw_filled_rounded_rectangle(
        point(game.win_w * 0.50f, game.win_h * 0.50f),
        point(game.win_w * 0.80f, game.win_h * 0.50f) - 16.0f,
        20.0f,
        al_map_rgba(112, 106, 193, 48)
    );
    
    gui.draw();
    
    game.fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Ticks one frame's worth of logic.
 */
void results_state::do_logic() {
    time_spent += game.delta_t;
    
    //Make the different texts grow every two or so seconds.
    size_t old_time_cp =
        (size_t) ((time_spent - game.delta_t) * 10) % 25;
    size_t new_time_cp =
        (size_t) (time_spent * 10) % 25;
        
    if(old_time_cp < new_time_cp) {
        switch(old_time_cp) {
        case 0: {
            area_name_text->start_juicy_grow();
            break;
        } case 2: {
            time_text->start_juicy_grow();
            break;
        } case 4: {
            points_text->start_juicy_grow();
            break;
        } case 6: {
            enemies_text->start_juicy_grow();
            break;
        } case 8: {
            pikmin_born_text->start_juicy_grow();
            break;
        } case 10: {
            pikmin_deaths_text->start_juicy_grow();
            break;
        }
        }
    }
    
    gui.tick(game.delta_t);
    
    game.fade_mgr.tick(game.delta_t);
}


/* ----------------------------------------------------------------------------
 * Returns the name of this state.
 */
string results_state::get_name() const {
    return "results";
}


/* ----------------------------------------------------------------------------
 * Handles Allegro events.
 * ev:
 *   Event to handle.
 */
void results_state::handle_allegro_event(ALLEGRO_EVENT &ev) {
    if(game.fade_mgr.is_fading()) return;
    
    gui.handle_event(ev);
}


/* ----------------------------------------------------------------------------
 * Leaves the results menu and goes to the area menu.
 */
void results_state::leave() {
    game.fade_mgr.start_fade(false, [] () {
        game.unload_loaded_state(game.states.gameplay);
        if(game.states.area_ed->quick_play_area.empty()) {
            game.change_state(game.states.area_menu);
        } else {
            game.change_state(game.states.area_ed);
        }
    });
}


/* ----------------------------------------------------------------------------
 * Loads the results state into memory.
 */
void results_state::load() {
    //Menu items.
    gui.register_coords("retry",                20, 90, 25,  6);
    gui.register_coords("continue",             50, 90, 25,  6);
    gui.register_coords("pick_area",            80, 90, 25,  6);
    gui.register_coords("area_name",            50, 10, 95, 10);
    gui.register_coords("finish_reason",        50, 17.5, 95, 10);
    gui.register_coords("time_label",           35, 30, 40, 10);
    gui.register_coords("time_amount",          70, 30, 50, 10);
    gui.register_coords("points_label",         35, 40, 40, 10);
    gui.register_coords("points_amount",        70, 40, 50, 10);
    gui.register_coords("enemies_label",        35, 50, 40, 10);
    gui.register_coords("enemies_amount",       70, 50, 50, 10);
    gui.register_coords("pikmin_born_label",    35, 60, 40, 10);
    gui.register_coords("pikmin_born_amount",   70, 60, 50, 10);
    gui.register_coords("pikmin_deaths_label",  35, 70, 40, 10);
    gui.register_coords("pikmin_deaths_amount", 70, 70, 50, 10);
    gui.read_coords(
        data_node(GUI_FILE_PATH).get_child_by_name("positions")
    );
    
    //Retry button.
    button_gui_item* retry_button =
        new button_gui_item("Retry", game.fonts.main);
    retry_button->on_activate =
    [this] (const point &) {
        retry_area();
    };
    gui.add_item(retry_button, "retry");
    
    //Keep playing button.
    if(can_continue) {
        button_gui_item* continue_button =
            new button_gui_item("Keep playing", game.fonts.main);
        continue_button->on_activate =
        [this] (const point &) {
            continue_playing();
        };
        gui.add_item(continue_button, "continue");
    }
    
    //Pick an area button.
    gui.back_item =
        new button_gui_item(
        game.states.area_ed->quick_play_area.empty() ?
        "Pick an area" :
        "Back to editor",
        game.fonts.main
    );
    gui.back_item->on_activate =
    [this] (const point &) {
        leave();
    };
    gui.add_item(gui.back_item, "pick_area");
    
    area_name_text =
        new text_gui_item(area_name, game.fonts.area_name);
    gui.add_item(area_name_text, "area_name");
    
    //Finish reason, if any.
    string finish_reason;
    if(leader_ko) {
        finish_reason = "Total leader KO!";
    } else if(out_of_time) {
        finish_reason = "Out of time!";
    }
    
    if(!finish_reason.empty()) {
        text_gui_item* finish_reason_text =
            new text_gui_item(
            finish_reason,
            game.fonts.main, al_map_rgb(255, 192, 192)
        );
        gui.add_item(finish_reason_text, "finish_reason");
    }
    
    //Time taken label text.
    text_gui_item* time_label_text =
        new text_gui_item(
        "Time taken:", game.fonts.main, map_gray(255), ALLEGRO_ALIGN_LEFT
    );
    gui.add_item(time_label_text, "time_label");
    
    //Time taken text.
    unsigned char ms = fmod(time_taken * 100, 100);
    unsigned char seconds = fmod(time_taken, 60);
    size_t minutes = time_taken / 60.0f;
    time_text =
        new text_gui_item(
        i2s(minutes) + ":" + pad_string(i2s(seconds), 2, '0') + "." + i2s(ms),
        game.fonts.counter
    );
    gui.add_item(time_text, "time_amount");
    
    //Points label text.
    text_gui_item* points_label_text =
        new text_gui_item(
        "Treasure points:", game.fonts.main, map_gray(255), ALLEGRO_ALIGN_LEFT
    );
    gui.add_item(points_label_text, "points_label");
    
    //Points text.
    points_text =
        new text_gui_item(
        i2s(points_obtained) + " / " + i2s(points_total),
        game.fonts.counter
    );
    gui.add_item(points_text, "points_amount");
    
    //Enemies label text.
    text_gui_item* enemies_label_text =
        new text_gui_item(
        "Enemies:", game.fonts.main, map_gray(255), ALLEGRO_ALIGN_LEFT
    );
    gui.add_item(enemies_label_text, "enemies_label");
    
    //Enemies text.
    enemies_text =
        new text_gui_item(
        i2s(enemies_beaten) + " / " + i2s(enemies_total),
        game.fonts.counter
    );
    gui.add_item(enemies_text, "enemies_amount");
    
    //Pikmin born label text.
    text_gui_item* pikmin_born_label_text =
        new text_gui_item(
        "Pikmin born:", game.fonts.main, map_gray(255), ALLEGRO_ALIGN_LEFT
    );
    gui.add_item(pikmin_born_label_text, "pikmin_born_label");
    
    //Pikmin born text.
    pikmin_born_text =
        new text_gui_item(
        i2s(pikmin_born),
        game.fonts.counter
    );
    gui.add_item(pikmin_born_text, "pikmin_born_amount");
    
    //Pikmin deaths label text.
    text_gui_item* pikmin_deaths_label_text =
        new text_gui_item(
        "Pikmin deaths:", game.fonts.main, map_gray(255), ALLEGRO_ALIGN_LEFT
    );
    gui.add_item(pikmin_deaths_label_text, "pikmin_deaths_label");
    
    //Pikmin deaths text.
    pikmin_deaths_text =
        new text_gui_item(
        i2s(pikmin_deaths),
        game.fonts.counter
    );
    gui.add_item(pikmin_deaths_text, "pikmin_deaths_amount");
    
    //Finishing touches.
    game.fade_mgr.start_fade(true, nullptr);
    gui.set_selected_item(gui.back_item);
    time_spent = 0.0f;
}


/* ----------------------------------------------------------------------------
 * Resets the state of the results screen.
 */
void results_state::reset() {
    area_name.clear();
    enemies_beaten = 0;
    enemies_total = 0;
    pikmin_born = 0;
    pikmin_deaths = 0;
    points_obtained = 0;
    points_total = 0;
    time_taken = 0.0f;
    can_continue = true;
    leader_ko = false;
}


/* ----------------------------------------------------------------------------
 * Leaves the results menu and goes back to the gameplay state to retry
 * the area.
 */
void results_state::retry_area() {
    game.fade_mgr.start_fade(false, [] () {
        game.unload_loaded_state(game.states.gameplay);
        game.change_state(game.states.gameplay);
    });
}


/* ----------------------------------------------------------------------------
 * Unloads the results state from memory.
 */
void results_state::unload() {
    //Menu items.
    gui.destroy();
}
