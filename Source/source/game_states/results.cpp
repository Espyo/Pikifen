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
    treasure_points_obtained(0),
    treasure_points_total(0),
    time_taken(0.0f),
    time_spent(0.0f),
    area_name_text(nullptr),
    area_subtitle_text(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * Adds a new stat to the stats list GUI item.
 * label:
 *   Label text of this stat.
 * value:
 *   Value of this stat.
 * color:
 *   Color.
 */
void results_state::add_stat(
    const string &label, const string &value,
    const ALLEGRO_COLOR &color
) {
    size_t stat_idx = stats_box->children.size() / 2.0f;
    const float STAT_HEIGHT = 0.12f;
    const float STAT_PADDING = 0.02f;
    const float STATS_OFFSET = 0.01f;
    const float stat_center_y =
        (STATS_OFFSET + STAT_HEIGHT / 2.0f) +
        ((STAT_HEIGHT + STAT_PADDING) * stat_idx);
        
    bullet_point_gui_item* label_bullet =
        new bullet_point_gui_item(
        label, game.fonts.standard, color
    );
    label_bullet->center =
        point(0.25f, stat_center_y);
    label_bullet->size =
        point(0.48f, STAT_HEIGHT);
    stats_box->add_child(label_bullet);
    gui.add_item(label_bullet);
    
    text_gui_item* value_text =
        new text_gui_item(
        value, game.fonts.counter, color, ALLEGRO_ALIGN_RIGHT
    );
    value_text->center =
        point(0.75f, stat_center_y);
    value_text->size =
        point(0.44f, STAT_HEIGHT);
    stats_box->add_child(value_text);
    gui.add_item(value_text);
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
    gui.register_coords("area_name",             50,  7, 45, 10);
    gui.register_coords("area_subtitle",         50, 18, 40, 10);
    gui.register_coords("goal_stamp",            15, 15, 22, 22);
    gui.register_coords("finish_reason",         16, 30, 30,  4);
    gui.register_coords("medal",                 85, 15, 22, 22);
    gui.register_coords("medal_reason",          85, 30, 30,  4);
    gui.register_coords("stats_label",           50, 32, 36,  4);
    gui.register_coords("stats",                 50, 56, 80, 40);
    gui.register_coords("stats_scroll",          93, 56,  2, 40);
    gui.register_coords("retry",                 20, 85, 24, 10);
    gui.register_coords("continue",              50, 85, 24, 10);
    gui.register_coords("pick_area",             80, 85, 24, 10);
    gui.register_coords("tooltip",               50, 95, 96,  7);
    gui.read_coords(
        data_node(RESULTS::GUI_FILE_PATH).get_child_by_name("positions")
    );
    
    //Area name text.
    area_name_text =
        new text_gui_item(
        area_name, game.fonts.area_name, COLOR_GOLD
    );
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
    
    //TODO stamp
    
    //Finish reason text, if any.
    //TODO add the others
    string finish_reason;
    if(leader_ko) {
        finish_reason = "Total leader KO...";
    } else if(out_of_time) {
        finish_reason = "Out of time...";
    }
    
    if(!finish_reason.empty()) {
        text_gui_item* finish_reason_text =
            new text_gui_item(finish_reason, game.fonts.standard);
        gui.add_item(finish_reason_text, "finish_reason");
    }
    
    if(game.cur_area_data.type == AREA_TYPE_MISSION) {
        //TODO medal
        
        //Medal reason.
        string medal_reason;
        switch(game.cur_area_data.mission.grading_mode) {
        case MISSION_GRADING_POINTS: {
            //TODO
            break;
        } case MISSION_GRADING_GOAL: {
            //TODO
            break;
        } case MISSION_GRADING_PARTICIPATION: {
            //TODO
            break;
        }
        }
        text_gui_item* medal_reason_text =
            new text_gui_item(medal_reason, game.fonts.standard);
        gui.add_item(medal_reason_text, "medal_reason");
    }
    
    //Stats label text.
    string stats_label = "Stats:";
    if(game.maker_tools.used_helping_tools) {
        stats_label = "Maker tools were used. " + stats_label;
    }
    text_gui_item* stats_label_text =
        new text_gui_item(stats_label, game.fonts.standard);
    gui.add_item(stats_label_text, "stats_label");
    
    //Stats box.
    stats_box = new list_gui_item();
    gui.add_item(stats_box, "stats");
    
    //Stats list scrollbar.
    scroll_gui_item* stats_scroll = new scroll_gui_item();
    stats_scroll->list_item = stats_box;
    gui.add_item(stats_scroll, "stats_scroll");
    
    //Time taken bullet.
    unsigned char ms = fmod(time_taken * 100, 100);
    unsigned char seconds = fmod(time_taken, 60);
    size_t minutes = time_taken / 60.0f;
    add_stat(
        "Time taken:",
        i2s(minutes) + ":" + pad_string(i2s(seconds), 2, '0') + "." + i2s(ms)
    );
    
    //Pikmin born bullet.
    add_stat("Pikmin born:", i2s(pikmin_born));
    
    if(
        game.cur_area_data.type == AREA_TYPE_MISSION &&
        game.cur_area_data.mission.points_per_pikmin_born != 0
    ) {
        //Pikmin born points bullet.
        add_stat(
            "    x " +
            i2s(game.cur_area_data.mission.points_per_pikmin_born) +
            " points = ",
            i2s(
                pikmin_born*
                game.cur_area_data.mission.points_per_pikmin_born
            )
        );
    }
    
    //TODO losing a given row of points on failure.
    
    //Pikmin deaths bullet.
    add_stat("Pikmin deaths:", i2s(pikmin_deaths));
    
    if(
        game.cur_area_data.type == AREA_TYPE_MISSION &&
        game.cur_area_data.mission.points_per_pikmin_death != 0
    ) {
        //Pikmin death points bullet.
        add_stat(
            "    x " +
            i2s(game.cur_area_data.mission.points_per_pikmin_death) +
            " points = ",
            i2s(
                pikmin_deaths*
                game.cur_area_data.mission.points_per_pikmin_death
            ),
            COLOR_GOLD
        );
    }
    
    if(
        game.cur_area_data.type == AREA_TYPE_MISSION &&
        game.cur_area_data.mission.points_per_sec_left != 0
    ) {
        //Seconds left bullet.
        add_stat("Seconds left:", i2s(123)); //TODO
        
        //Seconds left points bullet.
        add_stat(
            "    x " +
            i2s(game.cur_area_data.mission.points_per_sec_left) +
            " points = ",
            i2s(
                123 * //TODO
                game.cur_area_data.mission.points_per_sec_left
            ),
            COLOR_GOLD
        );
    }
    
    if(
        game.cur_area_data.type == AREA_TYPE_MISSION &&
        game.cur_area_data.mission.points_per_sec_passed != 0
    ) {
        //Seconds passed bullet.
        add_stat("Seconds passed:", i2s(time_spent));
        
        //Seconds passed points bullet.
        add_stat(
            "    x " +
            i2s(game.cur_area_data.mission.points_per_sec_passed) +
            " points = ",
            i2s(
                time_spent*
                game.cur_area_data.mission.points_per_sec_passed
            ),
            COLOR_GOLD
        );
    }
    
    //Treasures bullet.
    add_stat("Treasures:", i2s(123) + "/" + i2s(456)); //TODO
    
    //Treasure points bullet.
    add_stat(
        "Treasure points:",
        i2s(treasure_points_obtained) + "/" + i2s(treasure_points_total)
    );
    
    if(
        game.cur_area_data.type == AREA_TYPE_MISSION &&
        game.cur_area_data.mission.points_per_treasure_point != 0
    ) {
        //Treasure points points bullet.
        add_stat(
            "    x " +
            i2s(game.cur_area_data.mission.points_per_treasure_point) +
            " points = ",
            i2s(
                treasure_points_obtained*
                game.cur_area_data.mission.points_per_treasure_point
            ),
            COLOR_GOLD
        );
    }
    
    //Enemies bullet.
    add_stat("Enemies:", i2s(enemies_beaten) + "/" + i2s(enemies_total));
    
    //Enemy points bullet.
    add_stat(
        "Enemy points:",
        i2s(123) + "/" + i2s(456) //TODO
    );
    
    if(
        game.cur_area_data.type == AREA_TYPE_MISSION &&
        game.cur_area_data.mission.points_per_enemy_point != 0
    ) {
        //Enemy points points bullet.
        add_stat(
            "    x " +
            i2s(game.cur_area_data.mission.points_per_enemy_point) +
            " points = ",
            i2s(
                123 * //TODO
                game.cur_area_data.mission.points_per_enemy_point
            ),
            COLOR_GOLD
        );
    }
    
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
    treasure_points_obtained = 0;
    treasure_points_total = 0;
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
