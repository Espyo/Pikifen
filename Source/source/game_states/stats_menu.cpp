/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Statistics menu state class and statistics menu state-related functions.
 */

#include <algorithm>

#include "menus.h"

#include "../drawing.h"
#include "../functions.h"
#include "../game.h"
#include "../load.h"
#include "../utils/string_utils.h"


namespace STATS_MENU {
//Path to the GUI information file.
const string GUI_FILE_PATH = GUI_FOLDER_PATH + "/Statistics_menu.txt";
}


/* ----------------------------------------------------------------------------
 * Creates a "statistics menu" state.
 */
stats_menu_state::stats_menu_state() :
    game_state(),
    bmp_menu_bg(nullptr),
    stats_list(nullptr),
    runtime_value_text(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * Adds a new header to the stats list GUI item.
 * label:
 *   Name of the header.
 */
void stats_menu_state::add_header(const string &label) {
    float list_bottom_y = stats_list->get_child_bottom();
    const float HEADER_HEIGHT = 0.09f;
    const float STAT_PADDING = 0.02f;
    const float STATS_OFFSET = 0.01f;
    const float stat_center_y =
        list_bottom_y + (HEADER_HEIGHT / 2.0f) +
        (list_bottom_y == 0.0f ? STATS_OFFSET : STAT_PADDING);
        
    text_gui_item* label_text =
        new text_gui_item(label, game.fonts.area_name);
    label_text->center =
        point(0.50f, stat_center_y);
    label_text->size =
        point(0.96f, HEADER_HEIGHT);
    stats_list->add_child(label_text);
    gui.add_item(label_text);
}


/* ----------------------------------------------------------------------------
 * Adds a new stat to the stats list GUI item.
 * Returns the text GUI item for the value.
 * label:
 *   Name of the statistic.
 * value:
 *   Its value.
 * description:
 *   Tooltip description.
 */
text_gui_item* stats_menu_state::add_stat(
    const string &label, const string &value, const string &description
) {
    float list_bottom_y = stats_list->get_child_bottom();
    const float STAT_HEIGHT = 0.08f;
    const float STAT_PADDING = 0.02f;
    const float STATS_OFFSET = 0.01f;
    const float stat_center_y =
        list_bottom_y + (STAT_HEIGHT / 2.0f) +
        (list_bottom_y == 0.0f ? STATS_OFFSET : STAT_PADDING);
        
    bullet_point_gui_item* label_bullet =
        new bullet_point_gui_item(
        label, game.fonts.standard
    );
    label_bullet->center =
        point(0.50f, stat_center_y);
    label_bullet->size =
        point(0.96f, STAT_HEIGHT);
    label_bullet->on_get_tooltip = [description] () { return description; };
    stats_list->add_child(label_bullet);
    gui.add_item(label_bullet);
    
    text_gui_item* value_text =
        new text_gui_item(
        value, game.fonts.counter, COLOR_WHITE, ALLEGRO_ALIGN_RIGHT
    );
    value_text->center =
        point(0.75f, stat_center_y);
    value_text->size =
        point(0.44f, STAT_HEIGHT);
    stats_list->add_child(value_text);
    gui.add_item(value_text);
    
    return value_text;
}


/* ----------------------------------------------------------------------------
 * Draws the statistics menu.
 */
void stats_menu_state::do_drawing() {
    al_clear_to_color(COLOR_BLACK);
    
    draw_bitmap(
        bmp_menu_bg, point(game.win_w * 0.5, game.win_h * 0.5),
        point(game.win_w, game.win_h), 0, map_gray(64)
    );
    
    gui.draw();
    
    draw_mouse_cursor(GAME::CURSOR_STANDARD_COLOR);
    
    game.fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Ticks one frame's worth of logic.
 */
void stats_menu_state::do_logic() {
    vector<player_action> player_actions = game.controls.new_frame();
    if(!game.fade_mgr.is_fading()) {
        for(size_t a = 0; a < player_actions.size(); ++a) {
            gui.handle_player_action(player_actions[a]);
        }
    }
    
    gui.tick(game.delta_t);
    
    update_runtime_value_text();
    
    game.fade_mgr.tick(game.delta_t);
}


/* ----------------------------------------------------------------------------
 * Returns the name of this state.
 */
string stats_menu_state::get_name() const {
    return "statistics menu";
}


/* ----------------------------------------------------------------------------
 * Handles Allegro events.
 * ev:
 *   Event to handle.
 */
void stats_menu_state::handle_allegro_event(ALLEGRO_EVENT &ev) {
    if(game.fade_mgr.is_fading()) return;
    
    gui.handle_event(ev);
}


/* ----------------------------------------------------------------------------
 * Leaves the statistics menu and goes to the main menu.
 */
void stats_menu_state::leave() {
    save_statistics();
    game.fade_mgr.start_fade(false, [] () {
        game.change_state(game.states.main_menu);
    });
}


/* ----------------------------------------------------------------------------
 * Loads the statistics menu into memory.
 */
void stats_menu_state::load() {
    //Resources.
    bmp_menu_bg = load_bmp(game.asset_file_names.bmp_main_menu);
    
    //Menu items.
    gui.register_coords("back",        12,  5, 20,  6);
    gui.register_coords("header",      50,  5, 50,  6);
    gui.register_coords("list",        50, 51, 76, 82);
    gui.register_coords("list_scroll", 91, 51,  2, 82);
    gui.register_coords("tooltip",     50, 96, 96,  4);
    gui.read_coords(
        data_node(STATS_MENU::GUI_FILE_PATH).get_child_by_name("positions")
    );
    
    //Back button.
    gui.back_item =
        new button_gui_item("Back", game.fonts.standard);
    gui.back_item->on_activate =
    [this] (const point &) {
        leave();
    };
    gui.back_item->on_get_tooltip =
    [] () { return "Return to the main menu."; };
    gui.add_item(gui.back_item, "back");
    
    //Header text.
    text_gui_item* header_text =
        new text_gui_item(
        "STATISTICS",
        game.fonts.area_name, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    gui.add_item(header_text, "header");
    
    //Statistics list.
    stats_list = new list_gui_item();
    gui.add_item(stats_list, "list");
    
    //Statistics list scrollbar.
    scroll_gui_item* list_scroll = new scroll_gui_item();
    list_scroll->list_item = stats_list;
    gui.add_item(list_scroll, "list_scroll");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&gui);
    gui.add_item(tooltip_text, "tooltip");
    
    populate_stats_list();
    
    //Finishing touches.
    game.fade_mgr.start_fade(true, nullptr);
    gui.set_selected_item(gui.back_item);
    
}


/* ----------------------------------------------------------------------------
 * Populates the stats menu with bullet points.
 */
void stats_menu_state::populate_stats_list() {
    add_header(
        (game.config.name.empty() ? "Pikifen" : game.config.name) +
        " use"
    );
    add_stat(
        "Startups", i2s(game.statistics.startups),
        "Total number of times " +
        (game.config.name.empty() ? "Pikifen" : game.config.name) +
        " was started."
    );
    runtime_value_text =
        add_stat(
            "Runtime", "",
            "Total amount of time " +
            (game.config.name.empty() ? "Pikifen" : game.config.name) +
            " was running for, in seconds."
        );
    update_runtime_value_text();
    add_stat(
        "Gameplay time",
        time_to_str3(game.statistics.gameplay_time, ":", ":", ""),
        "Total amount of gameplay time, in seconds. Menus, editors, "
        "pause menu, etc. don't count."
    );
    add_stat(
        "Area entries", i2s(game.statistics.area_entries),
        "Total number of times that areas were entered. Includes retries "
        "and area editor tests."
    );
    
    add_header("Pikmin life");
    add_stat(
        "Pikmin births", i2s(game.statistics.pikmin_births),
        "Total number of times Pikmin were born from an Onion."
    );
    add_stat(
        "Pikmin deaths", i2s(game.statistics.pikmin_deaths),
        "Total number of times Pikmin died in any way."
    );
    add_stat(
        "Pikmin eaten", i2s(game.statistics.pikmin_eaten),
        "Total number of times Pikmin were swallowed by an enemy."
    );
    add_stat(
        "Pikmin hazard deaths", i2s(game.statistics.pikmin_hazard_deaths),
        "Total number of times Pikmin died from a hazard."
    );
    add_stat(
        "Pikmin bloom count", i2s(game.statistics.pikmin_blooms),
        "Total number of times Pikmin matured (leaf to bud, leaf to flower, "
        "or bud to flower)."
    );
    add_stat(
        "Pikmin saved", i2s(game.statistics.pikmin_saved),
        "Total number of times the whistle saved Pikmin from a hazard that was "
        "killing them."
    );
    add_stat(
        "Enemy deaths", i2s(game.statistics.enemy_deaths),
        "Total number of enemies that died."
    );
    
    add_header("Leader control");
    add_stat(
        "Pikmin thrown", i2s(game.statistics.pikmin_thrown),
        "Total number of times Pikmin were thrown. Leaders thrown don't count."
    );
    add_stat(
        "Whistle uses", i2s(game.statistics.whistle_uses),
        "Total number of times the whistle was used."
    );
    add_stat(
        "Distance walked (m)",
        f2s((game.statistics.distance_walked * CM_PER_PIXEL) / 100.0),
        "Total distance walked by an active leader, in meters."
    );
    add_stat(
        "Leader damage suffered", i2s(game.statistics.leader_damage_suffered),
        "Total amount of damage suffered by leaders."
    );
    add_stat(
        "Punch damage caused", i2s(game.statistics.punch_damage_caused),
        "Total amount of damage caused by a leader punching."
    );
    add_stat(
        "Leader KOs", i2s(game.statistics.leader_kos),
        "Total amount of times a leader got KO'd."
    );
    add_stat(
        "Sprays used", i2s(game.statistics.sprays_used),
        "Total amount of times a spray was used."
    );
    
    vector<string> mission_folders =
        folder_to_vector(
            get_base_area_folder_path(AREA_TYPE_MISSION, true),
            true
        );
        
    data_node mission_records_file;
    mission_records_file.load_file(
        MISSION_RECORDS_FILE_PATH, true, false, true
    );
    
    size_t mission_clears = 0;
    size_t mission_platinums = 0;
    long mission_scores = 0;
    
    for(size_t a = 0; a < mission_folders.size(); ++a) {
        string name = mission_folders[a];
        data_node data(
            get_base_area_folder_path(AREA_TYPE_MISSION, true) +
            "/" + mission_folders[a] + "/" + AREA_DATA_FILE_NAME
        );
        if(!data.file_was_opened) continue;
        string s = data.get_child_by_name("name")->value;
        if(!s.empty()) {
            name = s;
        }
        mission_data mission;
        mission_record record;
        load_area_mission_data(&data, mission);
        load_area_mission_record(
            &mission_records_file,
            name,
            get_subtitle_or_mission_goal(
                data.get_child_by_name("subtitle")->value,
                AREA_TYPE_MISSION,
                mission.goal
            ),
            data.get_child_by_name("maker")->value,
            data.get_child_by_name("version")->value,
            record
        );
        if(record.clear) {
            mission_clears++;
        }
        if(record.is_platinum(mission)) {
            mission_platinums++;
        }
        if(mission.grading_mode == MISSION_GRADING_POINTS) {
            mission_scores += record.score;
        }
    }
    
    add_header("Missions");
    add_stat(
        "Cleared",
        i2s(mission_clears) + "/" + i2s(mission_folders.size()),
        "Total amount of missions where the current record is a goal clear."
    );
    add_stat(
        "Platinum medals",
        i2s(mission_platinums) + "/" + i2s(mission_folders.size()),
        "Total amount of missions where the current record is a platinum medal."
    );
    add_stat(
        "Combined score", i2s(mission_scores),
        "Total combined score points of the current records of all missions."
    );
}


/* ----------------------------------------------------------------------------
 * Unloads the statistics menu from memory.
 */
void stats_menu_state::unload() {

    //Resources.
    al_destroy_bitmap(bmp_menu_bg);
    
    //Menu items.
    gui.destroy();
}


/* ----------------------------------------------------------------------------
 * Updates the GUI text item for the runtime stat value.
 */
void stats_menu_state::update_runtime_value_text() {
    runtime_value_text->text =
        time_to_str3(game.statistics.runtime, ":", ":", "");
}
