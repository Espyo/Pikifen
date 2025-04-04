/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Exploration/mission results state class and results state-related functions.
 */

#include "results.h"

#include "../core/drawing.h"
#include "../core/misc_functions.h"
#include "../core/game.h"
#include "../util/allegro_utils.h"
#include "../util/general_utils.h"
#include "../util/string_utils.h"


using DrawInfo = GuiItem::DrawInfo;


namespace RESULTS {

//Name of the GUI information file.
const string GUI_FILE_NAME = "results_menu";

}


/**
 * @brief Adds a new mission score criterion-related stat to the stats list
 * GUI item, if applicable.
 *
 * @param criterion Mission score criterion to use.
 */
void Results::add_score_stat(const MISSION_SCORE_CRITERIA criterion) {
    if(
        game.cur_area_data->type != AREA_TYPE_MISSION ||
        game.cur_area_data->mission.grading_mode != MISSION_GRADING_MODE_POINTS
    ) {
        return;
    }
    
    MissionScoreCriterion* c_ptr = game.mission_score_criteria[criterion];
    MissionData* mission = &game.cur_area_data->mission;
    int mult = c_ptr->get_multiplier(mission);
    
    if(mult == 0) return;
    
    bool goal_was_cleared =
        game.states.gameplay->mission_fail_reason ==
        (MISSION_FAIL_COND) INVALID;
    bool lost =
        has_flag(
            game.cur_area_data->mission.point_loss_data,
            get_idx_bitmask(criterion)
        ) &&
        !goal_was_cleared;
        
    if(lost) {
        add_stat(
            "    x 0 points (mission fail) = ",
            "0",
            COLOR_GOLD
        );
    } else {
        add_stat(
            "    x " +
            amount_str(mult, "point") +
            " = ",
            i2s(c_ptr->get_score(game.states.gameplay, mission)),
            COLOR_GOLD
        );
    }
}


/**
 * @brief Adds a new stat to the stats list GUI item.
 *
 * @param label Label text of this stat.
 * @param value Value of this stat.
 * @param color Color.
 */
void Results::add_stat(
    const string &label, const string &value,
    const ALLEGRO_COLOR &color
) {
    size_t stat_idx = stats_list->children.size() / 2.0f;
    const float STAT_HEIGHT = 0.12f;
    const float STAT_PADDING = 0.02f;
    const float STATS_OFFSET = 0.01f;
    const float stat_center_y =
        (STATS_OFFSET + STAT_HEIGHT / 2.0f) +
        ((STAT_HEIGHT + STAT_PADDING) * stat_idx);
        
    BulletGuiItem* label_bullet =
        new BulletGuiItem(
        label, game.sys_content.fnt_standard, color
    );
    label_bullet->ratio_center =
        Point(0.50f, stat_center_y);
    label_bullet->ratio_size =
        Point(0.96f, STAT_HEIGHT);
    stats_list->add_child(label_bullet);
    gui.add_item(label_bullet);
    
    TextGuiItem* value_text =
        new TextGuiItem(
        value, game.sys_content.fnt_counter, color, ALLEGRO_ALIGN_RIGHT
    );
    value_text->ratio_center =
        Point(0.75f, stat_center_y);
    value_text->ratio_size =
        Point(0.44f, STAT_HEIGHT);
    stats_list->add_child(value_text);
    gui.add_item(value_text);
    text_to_animate.push_back(value_text);
}


/**
 * @brief Leaves the results menu and goes back to the gameplay state to
 * continue playing the area.
 */
void Results::continue_playing() {
    game.fade_mgr.start_fade(false, [] () {
        game.states.gameplay->after_hours = true;
        game.states.gameplay->mission_fail_reason =
            (MISSION_FAIL_COND) INVALID;
        game.change_state(game.states.gameplay, true, false);
        game.states.gameplay->enter();
    });
}


/**
 * @brief Draws the results state.
 */
void Results::do_drawing() {
    //Background.
    al_clear_to_color(al_map_rgb(143, 149, 62));
    
    float logo_width = al_get_bitmap_width(game.sys_content.bmp_icon);
    float logo_height = al_get_bitmap_height(game.sys_content.bmp_icon);
    logo_height = game.win_w * 0.08f * (logo_width / logo_height);
    logo_width = game.win_w * 0.08f;
    draw_background_logos(
        gui_time_spent, 6, 6, Point(logo_width, logo_height), map_alpha(75),
        Point(-60.0f, 30.0f), -TAU / 6.0f
    );
    
    gui.draw();
    
    draw_mouse_cursor(GAME::CURSOR_STANDARD_COLOR);
}


/**
 * @brief Ticks one frame's worth of logic.
 */
void Results::do_logic() {
    if(!game.fade_mgr.is_fading()) {
        for(size_t a = 0; a < game.player_actions.size(); a++) {
            gui.handle_player_action(game.player_actions[a]);
        }
    }
    
    gui_time_spent += game.delta_t;
    
    //Make the different texts grow every two or so seconds.
    const float TEXT_ANIM_ALL_DURATION = 1.5f;
    const float TEXT_ANIM_PAUSE_DURATION = 1.0f;
    const float anim_time =
        fmod(gui_time_spent, TEXT_ANIM_ALL_DURATION + TEXT_ANIM_PAUSE_DURATION);
    const float time_per_item = TEXT_ANIM_ALL_DURATION / text_to_animate.size();
    const int old_time_cp = (anim_time - game.delta_t) / time_per_item;
    const int new_time_cp = anim_time / time_per_item;
    
    if(
        old_time_cp != new_time_cp &&
        old_time_cp >= 0 &&
        old_time_cp <= (int) text_to_animate.size() - 1
    ) {
        text_to_animate[old_time_cp]->start_juice_animation(
            GuiItem::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
        );
    }
    
    gui.tick(game.delta_t);
    
    game.fade_mgr.tick(game.delta_t);
}


/**
 * @brief Returns the name of this state.
 *
 * @return The name.
 */
string Results::get_name() const {
    return "results";
}


/**
 * @brief Handles Allegro events.
 *
 * @param ev Event to handle.
 */
void Results::handle_allegro_event(ALLEGRO_EVENT &ev) {
    if(game.fade_mgr.is_fading()) return;
    
    gui.handle_allegro_event(ev);
}


/**
 * @brief Leaves the results menu and goes to the area menu.
 */
void Results::leave() {
    game.fade_mgr.start_fade(false, [] () {
        AREA_TYPE area_type = game.cur_area_data->type;
        game.unload_loaded_state(game.states.gameplay);
        if(game.states.area_ed->quick_play_area_path.empty()) {
            game.states.annex_screen->area_menu_area_type =
                area_type;
            game.states.annex_screen->menu_to_load =
                ANNEX_SCREEN_MENU_AREA_SELECTION;
            game.change_state(game.states.annex_screen);
        } else {
            game.change_state(game.states.area_ed);
        }
    });
}


/**
 * @brief Loads the results state into memory.
 */
void Results::load() {
    bool goal_was_cleared =
        game.states.gameplay->mission_fail_reason ==
        (MISSION_FAIL_COND) INVALID;
        
    //Calculate score things.
    final_mission_score = game.cur_area_data->mission.starting_points;
    
    for(size_t c = 0; c < game.mission_score_criteria.size(); c++) {
        MissionScoreCriterion* c_ptr =
            game.mission_score_criteria[c];
        int c_score =
            c_ptr->get_score(game.states.gameplay, &game.cur_area_data->mission);
        bool lost =
            has_flag(
                game.cur_area_data->mission.point_loss_data,
                get_idx_bitmask(c)
            ) &&
            !goal_was_cleared;
            
        if(!lost) {
            final_mission_score += c_score;
        }
    }
    
    //Record loading and saving logic.
    MissionRecord old_record;
    
    DataNode mission_records;
    mission_records.loadFile(FILE_PATHS_FROM_ROOT::MISSION_RECORDS, true, false, true);
    string record_entry_name =
        get_mission_record_entry_name(game.cur_area_data);
    DataNode* entry_node;
    if(mission_records.getNrOfChildrenByName(record_entry_name) > 0) {
        entry_node =
            mission_records.getChildByName(record_entry_name);
    } else {
        entry_node = mission_records.addNew(record_entry_name, "");
    }
    
    vector<string> old_record_parts = split(entry_node->value, ";", true);
    
    if(old_record_parts.size() == 3) {
        old_record.clear = old_record_parts[0] == "1";
        old_record.score = s2i(old_record_parts[1]);
        old_record.date = s2i(old_record_parts[2]);
    }
    
    bool is_new_record = false;
    if(!old_record.clear && goal_was_cleared) {
        is_new_record = true;
    } else if(old_record.clear == goal_was_cleared) {
        if(
            game.cur_area_data->mission.grading_mode == MISSION_GRADING_MODE_POINTS &&
            old_record.score < final_mission_score
        ) {
            is_new_record = true;
        }
    }
    
    bool saved_successfully = true;
    if(
        is_new_record &&
        game.states.area_ed->quick_play_area_path.empty() &&
        !game.maker_tools.used_helping_tools &&
        !game.states.gameplay->after_hours
    ) {
        string clear_str = goal_was_cleared ? "1" : "0";
        string score_str = i2s(final_mission_score);
        string date_str = get_current_time(false);
        
        entry_node->value = clear_str + ";" + score_str + ";" + date_str;
        saved_successfully =
            mission_records.saveFile(
                FILE_PATHS_FROM_ROOT::MISSION_RECORDS, true, false, true
            );
    }
    
    if(!saved_successfully) {
        show_system_message_box(
            nullptr, "Save failed!",
            "Could not save this result!",
            (
                "An error occured while saving the mission record to the "
                "file \"" + FILE_PATHS_FROM_ROOT::MISSION_RECORDS + "\". Make sure that "
                "the folder it is saving to exists and it is not read-only, "
                "and try beating the mission again."
            ).c_str(),
            nullptr,
            ALLEGRO_MESSAGEBOX_WARN
        );
    }
    
    text_to_animate.clear();
    
    //Menu items.
    gui.register_coords("area_name",        50,  7, 45, 10);
    gui.register_coords("area_subtitle",    50, 18, 40, 10);
    gui.register_coords("goal_stamp",       15, 15, 22, 22);
    gui.register_coords("end_reason",       15, 28, 26,  4);
    gui.register_coords("medal",            85, 15, 22, 22);
    gui.register_coords("medal_reason",     85, 28, 26,  4);
    gui.register_coords("conclusion_label", 50, 32, 36,  4);
    gui.register_coords("conclusion",       50, 36, 96,  4);
    gui.register_coords("stats_label",      50, 42, 36,  4);
    gui.register_coords("stats",            50, 63, 80, 38);
    gui.register_coords("stats_scroll",     93, 63,  2, 38);
    gui.register_coords("retry",            20, 88, 24,  8);
    gui.register_coords("continue",         50, 88, 24,  8);
    gui.register_coords("pick_area",        80, 88, 24,  8);
    gui.register_coords("pick_area_input",  91, 91,  4,  4);
    gui.register_coords("tooltip",          50, 96, 96,  4);
    gui.read_coords(
        game.content.gui_defs.list[RESULTS::GUI_FILE_NAME].getChildByName("positions")
    );
    
    //Area name text.
    TextGuiItem* area_name_text =
        new TextGuiItem(
        game.cur_area_data->name, game.sys_content.fnt_area_name, COLOR_GOLD
    );
    gui.add_item(area_name_text, "area_name");
    text_to_animate.push_back(area_name_text);
    
    //Area subtitle text.
    string subtitle =
        get_subtitle_or_mission_goal(
            game.cur_area_data->subtitle,
            game.cur_area_data->type,
            game.cur_area_data->mission.goal
        );
    if(!subtitle.empty()) {
        TextGuiItem* area_subtitle_text =
            new TextGuiItem(subtitle, game.sys_content.fnt_area_name);
        gui.add_item(area_subtitle_text, "area_subtitle");
        text_to_animate.push_back(area_subtitle_text);
    }
    
    if(game.cur_area_data->type == AREA_TYPE_MISSION) {
        //Goal stamp image item.
        GuiItem* goal_stamp_item = new GuiItem;
        goal_stamp_item->on_draw =
        [goal_was_cleared] (const DrawInfo & draw) {
            draw_bitmap_in_box(
                goal_was_cleared ?
                game.sys_content.bmp_mission_clear :
                game.sys_content.bmp_mission_fail,
                draw.center, draw.size,
                true
            );
        };
        gui.add_item(goal_stamp_item, "goal_stamp");
        
        //End reason text, if any.
        string end_reason;
        if(goal_was_cleared) {
            end_reason =
                game.mission_goals[game.cur_area_data->mission.goal]->
                get_end_reason(&game.cur_area_data->mission);
        } else {
            end_reason =
                game.mission_fail_conds[
                    game.states.gameplay->mission_fail_reason
                ]->get_end_reason(
                    &game.cur_area_data->mission
                );
        }
        
        if(!end_reason.empty()) {
            TextGuiItem* end_reason_text =
                new TextGuiItem(
                end_reason, game.sys_content.fnt_standard,
                goal_was_cleared ?
                al_map_rgba(112, 200, 100, 192) :
                al_map_rgba(242, 160, 160, 192)
            );
            gui.add_item(end_reason_text, "end_reason");
        }
        
        //Medal reason text, if any.
        MISSION_MEDAL medal = MISSION_MEDAL_NONE;
        string medal_reason;
        ALLEGRO_COLOR medal_reason_color;
        switch(game.cur_area_data->mission.grading_mode) {
        case MISSION_GRADING_MODE_POINTS: {
            medal_reason = "Got " + i2s(final_mission_score) + " points";
            if(
                final_mission_score >=
                game.cur_area_data->mission.platinum_req
            ) {
                medal = MISSION_MEDAL_PLATINUM;
                medal_reason += "!";
                medal_reason_color = al_map_rgba(145, 226, 210, 192);
            } else if(
                final_mission_score >=
                game.cur_area_data->mission.gold_req
            ) {
                medal = MISSION_MEDAL_GOLD;
                medal_reason += "!";
                medal_reason_color = al_map_rgba(233, 200, 80, 192);
            } else if(
                final_mission_score >=
                game.cur_area_data->mission.silver_req
            ) {
                medal = MISSION_MEDAL_SILVER;
                medal_reason += "!";
                medal_reason_color = al_map_rgba(216, 216, 200, 192);
            } else if(
                final_mission_score >=
                game.cur_area_data->mission.bronze_req
            ) {
                medal = MISSION_MEDAL_BRONZE;
                medal_reason += "!";
                medal_reason_color = al_map_rgba(200, 132, 74, 192);
            } else {
                medal = MISSION_MEDAL_NONE;
                medal_reason += "...";
                medal_reason_color = al_map_rgba(200, 200, 200, 192);
            }
            break;
        } case MISSION_GRADING_MODE_GOAL: {
            if(goal_was_cleared) {
                medal = MISSION_MEDAL_PLATINUM;
                medal_reason = "Reached the goal!";
                medal_reason_color = al_map_rgba(145, 226, 210, 192);
            } else {
                medal = MISSION_MEDAL_NONE;
                medal_reason = "Did not reach the goal...";
                medal_reason_color = al_map_rgba(200, 200, 200, 192);
            }
            break;
        } case MISSION_GRADING_MODE_PARTICIPATION: {
            medal = MISSION_MEDAL_PLATINUM;
            medal_reason = "Played the mission!";
            medal_reason_color = al_map_rgba(145, 226, 210, 192);
            break;
        }
        }
        
        //Medal image item.
        GuiItem* medal_item = new GuiItem;
        medal_item->on_draw =
        [medal] (const DrawInfo & draw) {
            ALLEGRO_BITMAP* bmp = nullptr;
            switch(medal) {
            case MISSION_MEDAL_NONE: {
                bmp = game.sys_content.bmp_medal_none;
                break;
            } case MISSION_MEDAL_BRONZE: {
                bmp = game.sys_content.bmp_medal_bronze;
                break;
            } case MISSION_MEDAL_SILVER: {
                bmp = game.sys_content.bmp_medal_silver;
                break;
            } case MISSION_MEDAL_GOLD: {
                bmp = game.sys_content.bmp_medal_gold;
                break;
            } case MISSION_MEDAL_PLATINUM: {
                bmp = game.sys_content.bmp_medal_platinum;
                break;
            }
            }
            draw_bitmap_in_box(bmp, draw.center, draw.size, true);
        };
        gui.add_item(medal_item, "medal");
        
        //Medal reason.
        TextGuiItem* medal_reason_text =
            new TextGuiItem(
            medal_reason, game.sys_content.fnt_standard, medal_reason_color
        );
        gui.add_item(medal_reason_text, "medal_reason");
    }
    
    //Conclusion label text.
    string conclusion_label = "Conclusion:";
    TextGuiItem* conclusion_label_text =
        new TextGuiItem(
        conclusion_label, game.sys_content.fnt_standard,
        al_map_rgba(255, 255, 255, 192)
    );
    gui.add_item(conclusion_label_text, "conclusion_label");
    
    //Conclusion text.
    string conclusion;
    switch(game.cur_area_data->type) {
    case AREA_TYPE_SIMPLE: {
        if(!game.states.area_ed->quick_play_area_path.empty()) {
            conclusion =
                "Area editor playtest ended.";
        } else if(game.maker_tools.used_helping_tools) {
            conclusion =
                "Nothing to report, other than maker tools being used.";
        } else {
            conclusion =
                "Nothing to report.";
        }
        break;
    } case AREA_TYPE_MISSION: {
        if(game.states.gameplay->after_hours) {
            conclusion =
                "Played in after hours, so the "
                "result past that point won't be saved.";
        } else if(!game.states.area_ed->quick_play_area_path.empty()) {
            conclusion =
                "This was an area editor playtest, "
                "so the result won't be saved.";
        } else if(game.maker_tools.used_helping_tools) {
            conclusion =
                "Maker tools were used, "
                "so the result won't be saved.";
        } else if(
            game.cur_area_data->mission.grading_mode == MISSION_GRADING_MODE_POINTS &&
            old_record.clear &&
            !goal_was_cleared &&
            old_record.score < final_mission_score
        ) {
            conclusion =
                "High score, but the old record was a "
                "clear, so this result won't be saved.";
        } else if(!is_new_record) {
            conclusion =
                "This result is not a new record, so "
                "it won't be saved.";
        } else if(!saved_successfully) {
            conclusion =
                "FAILED TO SAVE THIS RESULT AS A NEW RECORD!";
        } else {
            conclusion =
                "Saved this result as a new record!";
        }
    } case N_AREA_TYPES: {
        break;
    }
    }
    TextGuiItem* conclusion_text =
        new TextGuiItem(conclusion, game.sys_content.fnt_standard);
    gui.add_item(conclusion_text, "conclusion");
    
    //Stats label text.
    TextGuiItem* stats_label_text =
        new TextGuiItem(
        "Stats:", game.sys_content.fnt_standard, al_map_rgba(255, 255, 255, 192)
    );
    gui.add_item(stats_label_text, "stats_label");
    
    //Stats box.
    stats_list = new ListGuiItem();
    stats_list->on_draw =
    [this] (const DrawInfo & draw) {
        draw_filled_rounded_rectangle(
            draw.center, draw.size, 8.0f, al_map_rgba(0, 0, 0, 40)
        );
        draw_textured_box(
            draw.center, draw.size, game.sys_content.bmp_frame_box,
            COLOR_TRANSPARENT_WHITE
        );
    };
    gui.add_item(stats_list, "stats");
    
    //Stats list scrollbar.
    ScrollGuiItem* stats_scroll = new ScrollGuiItem();
    stats_scroll->list_item = stats_list;
    gui.add_item(stats_scroll, "stats_scroll");
    
    if(
        game.cur_area_data->type == AREA_TYPE_MISSION &&
        game.cur_area_data->mission.starting_points != 0
    ) {
        //Starting score bullet.
        add_stat(
            "Starting score: ",
            i2s(game.cur_area_data->mission.starting_points),
            COLOR_GOLD
        );
    }
    
    //Time taken bullet.
    unsigned int ds =
        fmod(game.states.gameplay->gameplay_time_passed * 10, 10);
    unsigned char seconds =
        fmod(game.states.gameplay->gameplay_time_passed, 60);
    size_t minutes =
        game.states.gameplay->gameplay_time_passed / 60.0f;
    add_stat(
        "Time taken:",
        i2s(minutes) + ":" + pad_string(i2s(seconds), 2, '0') + "." + i2s(ds)
    );
    
    //Pikmin born bullet.
    add_stat("Pikmin born:", i2s(game.states.gameplay->pikmin_born));
    
    //Pikmin born points bullet.
    add_score_stat(MISSION_SCORE_CRITERIA_PIKMIN_BORN);
    
    //Pikmin deaths bullet.
    add_stat("Pikmin deaths:", i2s(game.states.gameplay->pikmin_deaths));
    
    //Pikmin death points bullet.
    add_score_stat(MISSION_SCORE_CRITERIA_PIKMIN_DEATH);
    
    if(
        game.cur_area_data->type == AREA_TYPE_MISSION &&
        game.cur_area_data->mission.points_per_sec_left != 0
    ) {
        //Seconds left bullet.
        add_stat(
            "Seconds left:",
            i2s(
                game.cur_area_data->mission.fail_time_limit -
                floor(game.states.gameplay->gameplay_time_passed)
            )
        );
        
        //Seconds left points bullet.
        add_score_stat(MISSION_SCORE_CRITERIA_SEC_LEFT);
    }
    
    if(
        game.cur_area_data->type == AREA_TYPE_MISSION &&
        game.cur_area_data->mission.points_per_sec_passed != 0
    ) {
        //Seconds passed bullet.
        add_stat(
            "Seconds passed:",
            i2s(game.states.gameplay->gameplay_time_passed)
        );
        
        //Seconds passed points bullet.
        add_score_stat(MISSION_SCORE_CRITERIA_SEC_PASSED);
    }
    
    //Treasures bullet.
    add_stat(
        "Treasures:",
        i2s(game.states.gameplay->treasures_collected) + "/" +
        i2s(game.states.gameplay->treasures_total)
    );
    
    //Treasure points bullet.
    add_stat(
        "Treasure points:",
        i2s(game.states.gameplay->treasure_points_collected) + "/" +
        i2s(game.states.gameplay->treasure_points_total)
    );
    
    //Treasure points points bullet.
    add_score_stat(MISSION_SCORE_CRITERIA_TREASURE_POINTS);
    
    //Enemy deaths bullet.
    add_stat(
        "Enemy deaths:",
        i2s(game.states.gameplay->enemy_deaths) + "/" +
        i2s(game.states.gameplay->enemy_total)
    );
    
    //Enemy points bullet.
    add_stat(
        "Enemy kill points:",
        i2s(game.states.gameplay->enemy_points_collected) + "/" +
        i2s(game.states.gameplay->enemy_points_total)
    );
    
    //Enemy points points bullet.
    add_score_stat(MISSION_SCORE_CRITERIA_ENEMY_POINTS);
    
    if(
        game.cur_area_data->type == AREA_TYPE_MISSION &&
        game.cur_area_data->mission.grading_mode == MISSION_GRADING_MODE_POINTS
    ) {
        //Final score bullet.
        add_stat(
            "Final score:",
            i2s(final_mission_score),
            COLOR_GOLD
        );
        
        //Old record bullet:
        add_stat(
            "Previous record:",
            old_record.date.empty() ? "-" : i2s(old_record.score),
            COLOR_WHITE
        );
        
        //Maker's record bullet.
        if(!game.cur_area_data->mission.maker_record_date.empty()) {
            add_stat(
                "Maker's record:",
                i2s(game.cur_area_data->mission.maker_record),
                COLOR_WHITE
            );
        }
    }
    
    
    //Retry button.
    ButtonGuiItem* retry_button =
        new ButtonGuiItem("Retry", game.sys_content.fnt_standard);
    retry_button->on_activate =
    [this] (const Point &) {
        retry_area();
    };
    retry_button->on_get_tooltip =
    [] () { return "Retry the area from the start."; };
    gui.add_item(retry_button, "retry");
    
    //Keep playing button.
    if(
        game.states.gameplay->mission_fail_reason ==
        MISSION_FAIL_COND_TIME_LIMIT
    ) {
        ButtonGuiItem* continue_button =
            new ButtonGuiItem("Keep playing", game.sys_content.fnt_standard);
        continue_button->on_activate =
        [this] (const Point &) {
            continue_playing();
        };
        continue_button->on_get_tooltip =
        [] () {
            return
                "Continue playing anyway, from where you left off. "
                "Your result after this point won't count.";
        };
        gui.add_item(continue_button, "continue");
    }
    
    //Pick an area button.
    gui.back_item =
        new ButtonGuiItem(
        game.states.area_ed->quick_play_area_path.empty() ?
        "Pick an area" :
        "Back to editor",
        game.sys_content.fnt_standard
    );
    gui.back_item->on_activate =
    [this] (const Point &) {
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
    
    //Pick an area input icon.
    gui_add_back_input_icon(&gui, "pick_area_input");
    
    //Tooltip text.
    TooltipGuiItem* tooltip_text =
        new TooltipGuiItem(&gui);
    gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    game.audio.set_current_song(game.sys_content_names.sng_editors);
    game.fade_mgr.start_fade(true, nullptr);
    gui.set_selected_item(gui.back_item, true);
    gui_time_spent = 0.0f;
}


/**
 * @brief Leaves the results menu and goes back to the gameplay state to retry
 * the area.
 */
void Results::retry_area() {
    game.fade_mgr.start_fade(false, [] () {
        game.unload_loaded_state(game.states.gameplay);
        game.change_state(game.states.gameplay);
    });
}


/**
 * @brief Unloads the results state from memory.
 */
void Results::unload() {
    //Menu items.
    gui.destroy();
}
