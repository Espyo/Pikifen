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
#include "../functions.h"
#include "../game.h"
#include "../utils/string_utils.h"


namespace RESULTS {
//Path to the GUI information file.
const string GUI_FILE_PATH = GUI_FOLDER_PATH + "/Results_menu.txt";
}


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
    time_spent(0.0f),
    area_name_text(nullptr),
    area_subtitle_text(nullptr),
    time_text(nullptr),
    pikmin_born_point_text(nullptr),
    pikmin_death_points_text(nullptr),
    seconds_left_points_text(nullptr),
    seconds_passed_points_text(nullptr),
    treasure_points_points_text(nullptr),
    enemy_points_points_text(nullptr) {
    
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
        
    //TODO add missing ones
    
    if(old_time_cp < new_time_cp) {
        switch(old_time_cp) {
        case 0: {
            area_name_text->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            break;
        } case 2: {
            time_text->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            break;
        } case 4: {
            treasure_points_points_text->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            break;
        } case 6: {
            enemy_points_points_text->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            break;
        } case 8: {
            pikmin_born_point_text->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
            break;
        } case 10: {
            pikmin_death_points_text->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_HIGH
            );
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
        if(game.states.area_ed->quick_play_area_path.empty()) {
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
    gui.register_coords("retry",                 20, 87.5, 25,  7);
    gui.register_coords("continue",              50, 87.5, 25,  7);
    gui.register_coords("pick_area",             80, 87.5, 25,  7);
    gui.register_coords("box",                   50,   52, 88, 40);
    gui.register_coords("area_name",             35,    7, 66, 10);
    gui.register_coords("area_subtitle",         35,   19, 66, 10);
    gui.register_coords("goal_stamp",            83,   11, 30, 18);
    gui.register_coords("finish_reason",         83,   22, 30,  4);
    gui.register_coords("time_label",            36,   28, 28,  4);
    gui.register_coords("time_amount",           64,   28, 28,  4);
    gui.register_coords("pikmin_born_label",     25,   37, 30,  6);
    gui.register_coords("pikmin_born_amount",    49,   37, 18,  6);
    gui.register_coords("pikmin_born_mult",      66,   37, 16,  6);
    gui.register_coords("pikmin_born_points",    81,   37, 14,  6);
    gui.register_coords("pikmin_deaths_label",   25,   43, 30,  6);
    gui.register_coords("pikmin_deaths_amount",  49,   43, 18,  6);
    gui.register_coords("pikmin_deaths_mult",    66,   43, 16,  6);
    gui.register_coords("pikmin_deaths_points",  81,   43, 14,  6);
    gui.register_coords("seconds_left_label",    25,   49, 30,  6);
    gui.register_coords("seconds_left_amount",   49,   49, 18,  6);
    gui.register_coords("seconds_left_mult",     66,   49, 16,  6);
    gui.register_coords("seconds_left_points",   81,   49, 14,  6);
    gui.register_coords("seconds_passed_label",  25,   55, 30,  6);
    gui.register_coords("seconds_passed_amount", 49,   55, 18,  6);
    gui.register_coords("seconds_passed_mult",   66,   55, 16,  6);
    gui.register_coords("seconds_passed_points", 81,   55, 14,  6);
    gui.register_coords("treasure_label",        25,   61, 30,  6);
    gui.register_coords("treasure_amount",       49,   61, 18,  6);
    gui.register_coords("treasure_mult",         66,   61, 16,  6);
    gui.register_coords("treasure_points",       81,   61, 14,  6);
    gui.register_coords("treasure_total",        89,   62, 10,  4);
    gui.register_coords("enemies_label",         25,   67, 30,  6);
    gui.register_coords("enemies_amount",        49,   67, 18,  6);
    gui.register_coords("enemies_mult",          66,   67, 16,  6);
    gui.register_coords("enemies_points",        81,   67, 14,  6);
    gui.register_coords("enemies_total",         89,   68, 10,  4);
    gui.register_coords("medal",                 88,   77, 20, 14);
    gui.register_coords("used_tools",            50,   80, 56,  4);
    gui.register_coords("final_score_label",     36,   76, 28,  4);
    gui.register_coords("final_score",           64,   76, 28,  4);
    gui.register_coords("tooltip",               50,   95, 95,  8);
    gui.read_coords(
        data_node(RESULTS::GUI_FILE_PATH).get_child_by_name("positions")
    );
    
    //Retry button.
    button_gui_item* retry_button =
        new button_gui_item("Retry", game.fonts.standard);
    retry_button->on_activate =
    [this] (const point &) {
        retry_area();
    };
    retry_button->on_get_tooltip =
    [] () { return "Retry the area from the start."; };
    gui.add_item(retry_button, "retry");
    
    //Keep playing button.
    if(can_continue) {
        button_gui_item* continue_button =
            new button_gui_item("Keep playing", game.fonts.standard);
        continue_button->on_activate =
        [this] (const point &) {
            continue_playing();
        };
        continue_button->on_get_tooltip =
        [] () { return "Continue playing anyway, from where you left off."; };
        gui.add_item(continue_button, "continue");
    }
    
    //Pick an area button.
    gui.back_item =
        new button_gui_item(
        game.states.area_ed->quick_play_area_path.empty() ?
        "Pick an area" :
        "Back to editor",
        game.fonts.standard
    );
    gui.back_item->on_activate =
    [this] (const point &) {
        leave();
    };
    gui.back_item->on_get_tooltip =
    [] () {
        return
            game.states.area_ed->quick_play_area_path.empty() ?
            "Return to the area selection menu." :
            "Return to the area editor.";
    };
    gui.add_item(gui.back_item, "pick_area");
    
    //Box.
    gui_item* box_item = new gui_item();
    box_item->on_draw =
    [] (const point & center, const point & size) {
        draw_filled_rounded_rectangle(
            center,
            size,
            20.0f,
            al_map_rgba(57, 54, 98, 48)
        );
        draw_filled_rounded_rectangle(
            center,
            size - 16.0f,
            20.0f,
            al_map_rgba(112, 106, 193, 48)
        );
    };
    gui.add_item(box_item, "box");
    
    //Area name text.
    area_name_text =
        new text_gui_item(area_name, game.fonts.area_name);
    gui.add_item(area_name_text, "area_name");
    
    //Area subtitle text.
    area_subtitle_text =
        new text_gui_item(
        get_subtitle_or_mission_goal(
            game.cur_area_data.subtitle,
            game.cur_area_data.type,
            game.cur_area_data.mission.goal
        ),
        game.fonts.area_name
    );
    gui.add_item(area_subtitle_text, "area_subtitle");
    
    //Finish reason text, if any.
    //TODO add the others
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
            game.fonts.standard, al_map_rgb(255, 192, 192)
        );
        gui.add_item(finish_reason_text, "finish_reason");
    }
    
    //TODO stamp
    
    //Maker tools usage disclaimer.
    if(game.maker_tools.used_helping_tools) {
        text_gui_item* used_tools_text =
            new text_gui_item(
            "(Maker tools were used.)", game.fonts.standard,
            al_map_rgb(255, 215, 192)
        );
        gui.add_item(used_tools_text, "used_tools");
    }
    
    //Time taken label text.
    text_gui_item* time_label_text =
        new text_gui_item(
        "Time taken:", game.fonts.standard, map_gray(255), ALLEGRO_ALIGN_LEFT
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
    
    //Pikmin born label text.
    text_gui_item* pikmin_born_label_text =
        new text_gui_item(
        "Pikmin born:", game.fonts.standard, map_gray(255), ALLEGRO_ALIGN_LEFT
    );
    gui.add_item(pikmin_born_label_text, "pikmin_born_label");
    
    //Pikmin born amount text.
    text_gui_item* pikmin_born_amount_text =
        new text_gui_item(
        i2s(pikmin_born),
        game.fonts.counter
    );
    gui.add_item(pikmin_born_amount_text, "pikmin_born_amount");
    
    //Pikmin born multiplier text.
    text_gui_item* pikmin_born_mult_text =
        new text_gui_item(
        "x " + i2s(game.cur_area_data.mission.points_per_pikmin_born) + " =",
        game.fonts.standard
    );
    gui.add_item(pikmin_born_mult_text, "pikmin_born_mult");
    
    //Pikmin born point text.
    pikmin_born_point_text =
        new text_gui_item(
        i2s(pikmin_born * game.cur_area_data.mission.points_per_pikmin_born),
        game.fonts.counter
    );
    gui.add_item(pikmin_born_point_text, "pikmin_born_points");
    
    //Pikmin death label text.
    text_gui_item* pikmin_deaths_label_text =
        new text_gui_item(
        "Pikmin deaths:", game.fonts.standard, map_gray(255), ALLEGRO_ALIGN_LEFT
    );
    gui.add_item(pikmin_deaths_label_text, "pikmin_deaths_label");
    
    //Pikmin death amount text.
    text_gui_item* pikmin_death_amount_text =
        new text_gui_item(
        i2s(pikmin_deaths),
        game.fonts.counter
    );
    gui.add_item(pikmin_death_amount_text, "pikmin_deaths_amount");
    
    //Pikmin death multiplier text.
    text_gui_item* pikmin_death_mult_text =
        new text_gui_item(
        "x " + i2s(game.cur_area_data.mission.points_per_pikmin_death) + " =",
        game.fonts.standard
    );
    gui.add_item(pikmin_death_mult_text, "pikmin_deaths_mult");
    
    //Pikmin death points text.
    pikmin_death_points_text =
        new text_gui_item(
        i2s(pikmin_deaths * game.cur_area_data.mission.points_per_pikmin_death),
        game.fonts.counter
    );
    gui.add_item(pikmin_death_points_text, "pikmin_deaths_points");
    
    //Seconds left label text.
    text_gui_item* seconds_left_label_text =
        new text_gui_item(
        "Seconds left:", game.fonts.standard, map_gray(255), ALLEGRO_ALIGN_LEFT
    );
    gui.add_item(seconds_left_label_text, "seconds_left_label");
    
    //Seconds left amount text.
    text_gui_item* seconds_left_amount_text =
        new text_gui_item(
        i2s(pikmin_deaths), //TODO
        game.fonts.counter
    );
    gui.add_item(seconds_left_amount_text, "seconds_left_amount");
    
    //Seconds left multiplier text.
    text_gui_item* seconds_left_mult_text =
        new text_gui_item(
        "x " + i2s(game.cur_area_data.mission.points_per_sec_left) + " =",
        game.fonts.standard
    );
    gui.add_item(seconds_left_mult_text, "seconds_left_mult");
    
    //Seconds left points text.
    seconds_left_points_text =
        new text_gui_item(
        i2s(pikmin_deaths * game.cur_area_data.mission.points_per_sec_left), //TODO
        game.fonts.counter
    );
    gui.add_item(seconds_left_points_text, "seconds_left_points");
    
    //Seconds passed label text.
    text_gui_item* seconds_passed_label_text =
        new text_gui_item(
        "Seconds passed:", game.fonts.standard, map_gray(255), ALLEGRO_ALIGN_LEFT
    );
    gui.add_item(seconds_passed_label_text, "seconds_passed_label");
    
    //Seconds passed amount text.
    text_gui_item* seconds_passed_amount_text =
        new text_gui_item(
        i2s(pikmin_deaths), //TODO
        game.fonts.counter
    );
    gui.add_item(seconds_passed_amount_text, "seconds_passed_amount");
    
    //Seconds passed multiplier text.
    text_gui_item* seconds_passed_mult_text =
        new text_gui_item(
        "x " + i2s(game.cur_area_data.mission.points_per_sec_passed) + " =",
        game.fonts.standard
    );
    gui.add_item(seconds_passed_mult_text, "seconds_passed_mult");
    
    //Seconds passed points text.
    seconds_passed_points_text =
        new text_gui_item(
        i2s(pikmin_deaths * game.cur_area_data.mission.points_per_sec_passed), //TODO
        game.fonts.counter
    );
    gui.add_item(seconds_passed_points_text, "seconds_passed_points");
    
    //Treasure points label text.
    text_gui_item* treasure_points_label_text =
        new text_gui_item(
        "Treasure points:", game.fonts.standard,
        map_gray(255), ALLEGRO_ALIGN_LEFT
    );
    gui.add_item(treasure_points_label_text, "treasure_label");
    
    //Treasure points amount text.
    text_gui_item* treasure_points_amount_text =
        new text_gui_item(
        i2s(points_obtained),
        game.fonts.counter
    );
    gui.add_item(treasure_points_amount_text, "treasure_amount");
    
    //Treasure points multiplier text.
    text_gui_item* treasure_points_mult_text =
        new text_gui_item(
        "x " + i2s(game.cur_area_data.mission.points_per_treasure_point) + " =",
        game.fonts.standard
    );
    gui.add_item(treasure_points_mult_text, "treasure_mult");
    
    //Treasure points points text.
    treasure_points_points_text =
        new text_gui_item(
        i2s(points_obtained * game.cur_area_data.mission.points_per_treasure_point),
        game.fonts.counter
    );
    gui.add_item(treasure_points_points_text, "treasure_points");
    
    //Treasure points total text.
    text_gui_item* treasure_points_total_text =
        new text_gui_item(
        "/ " + i2s(points_total),
        game.fonts.counter,
        COLOR_WHITE,
        ALLEGRO_ALIGN_LEFT
    );
    gui.add_item(treasure_points_total_text, "treasure_total");
    
    //Enemy points label text.
    text_gui_item* enemy_points_label_text =
        new text_gui_item(
        "Enemy points:", game.fonts.standard,
        map_gray(255), ALLEGRO_ALIGN_LEFT
    );
    gui.add_item(enemy_points_label_text, "enemies_label");
    
    //Enemy points amount text.
    text_gui_item* enemy_points_amount_text =
        new text_gui_item(
        i2s(enemies_beaten),
        game.fonts.counter
    );
    gui.add_item(enemy_points_amount_text, "enemies_amount");
    
    //Enemy points multiplier text.
    text_gui_item* enemy_points_mult_text =
        new text_gui_item(
        "x " + i2s(game.cur_area_data.mission.points_per_enemy_point) + " =",
        game.fonts.standard
    );
    gui.add_item(enemy_points_mult_text, "enemies_mult");
    
    //Enemy points points text.
    enemy_points_points_text =
        new text_gui_item(
        i2s(enemies_beaten * game.cur_area_data.mission.points_per_enemy_point),
        game.fonts.counter
    );
    gui.add_item(enemy_points_points_text, "enemies_points");
    
    //Enemy points total text.
    text_gui_item* enemy_points_total_text =
        new text_gui_item(
        "/ " + i2s(enemies_total),
        game.fonts.counter,
        COLOR_WHITE,
        ALLEGRO_ALIGN_LEFT
    );
    gui.add_item(enemy_points_total_text, "enemies_total");
    
    //Final score label text.
    text_gui_item* final_score_label_text =
        new text_gui_item(
        "Final score:", game.fonts.standard,
        map_gray(255), ALLEGRO_ALIGN_LEFT
    );
    gui.add_item(final_score_label_text, "final_score_label");
    
    //Final score amount text.
    final_score_text =
        new text_gui_item(
        "1234", //TODO
        game.fonts.counter
    );
    gui.add_item(final_score_text, "final_score");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&gui);
    gui.add_item(tooltip_text, "tooltip");
    
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
