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
    gui_time_spent(0.0f) {
    
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
    size_t stat_idx = stats_list->children.size() / 2.0f;
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
        point(0.50f, stat_center_y);
    label_bullet->size =
        point(0.96f, STAT_HEIGHT);
    stats_list->add_child(label_bullet);
    gui.add_item(label_bullet);
    
    text_gui_item* value_text =
        new text_gui_item(
        value, game.fonts.counter, color, ALLEGRO_ALIGN_RIGHT
    );
    value_text->center =
        point(0.75f, stat_center_y);
    value_text->size =
        point(0.44f, STAT_HEIGHT);
    stats_list->add_child(value_text);
    gui.add_item(value_text);
    text_to_animate.push_back(value_text);
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
        gui_time_spent, 6, 6, point(logo_width, logo_height), map_alpha(75),
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
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
        );
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
        AREA_TYPES area_type = game.cur_area_data.type;
        game.unload_loaded_state(game.states.gameplay);
        if(game.states.area_ed->quick_play_area_path.empty()) {
            game.states.area_menu->area_type = area_type;
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
    size_t secs_left =
        game.cur_area_data.mission.fail_time_limit -
        game.states.gameplay->area_time_passed;
        
    int pikmin_born_score =
        game.states.gameplay->pikmin_born *
        game.cur_area_data.mission.points_per_pikmin_born;
    bool lost_pikmin_born_score = false;
    if(
        has_flag(
            game.cur_area_data.mission.point_loss_data,
            MISSION_POINT_CRITERIA_PIKMIN_BORN
        ) &&
        game.states.gameplay->mission_fail_reason != 0
    ) {
        pikmin_born_score = 0;
        lost_pikmin_born_score = true;
    }
    
    int pikmin_death_score =
        game.states.gameplay->pikmin_deaths *
        game.cur_area_data.mission.points_per_pikmin_death;
    bool lost_pikmin_death_score = false;
    if(
        has_flag(
            game.cur_area_data.mission.point_loss_data,
            MISSION_POINT_CRITERIA_PIKMIN_DEATH
        ) &&
        game.states.gameplay->mission_fail_reason != 0
    ) {
        pikmin_death_score = 0;
        lost_pikmin_death_score = true;
    }
    
    int secs_left_score =
        secs_left *
        game.cur_area_data.mission.points_per_sec_left;
    bool lost_secs_left_score = false;
    if(
        has_flag(
            game.cur_area_data.mission.point_loss_data,
            MISSION_POINT_CRITERIA_SEC_LEFT
        ) &&
        game.states.gameplay->mission_fail_reason != 0
    ) {
        secs_left_score = 0;
        lost_secs_left_score = true;
    }
    
    int secs_passed_score =
        ((int) game.states.gameplay->area_time_passed) *
        game.cur_area_data.mission.points_per_sec_passed;
    bool lost_secs_passed_score = false;
    if(
        has_flag(
            game.cur_area_data.mission.point_loss_data,
            MISSION_POINT_CRITERIA_SEC_PASSED
        ) &&
        game.states.gameplay->mission_fail_reason != 0
    ) {
        secs_passed_score = 0;
        lost_secs_passed_score = true;
    }
    
    int treasure_points_score =
        game.states.gameplay->treasure_points_collected *
        game.cur_area_data.mission.points_per_treasure_point;
    bool lost_treasure_points_score = false;
    if(
        has_flag(
            game.cur_area_data.mission.point_loss_data,
            MISSION_POINT_CRITERIA_TREASURE_POINTS
        ) &&
        game.states.gameplay->mission_fail_reason != 0
    ) {
        treasure_points_score = 0;
        lost_treasure_points_score = true;
    }
    
    int enemy_points_score =
        game.states.gameplay->enemy_points_collected *
        game.cur_area_data.mission.points_per_enemy_point;
    bool lost_enemy_points_score = false;
    if(
        has_flag(
            game.cur_area_data.mission.point_loss_data,
            MISSION_POINT_CRITERIA_ENEMY_POINTS
        ) &&
        game.states.gameplay->mission_fail_reason != 0
    ) {
        enemy_points_score = 0;
        lost_enemy_points_score = true;
    }
    
    final_mission_score =
        pikmin_born_score +
        pikmin_death_score +
        secs_left_score +
        secs_passed_score +
        treasure_points_score +
        enemy_points_score;
        
    text_to_animate.clear();
    
    //Menu items.
    gui.register_coords("area_name",             50,  7, 45, 10);
    gui.register_coords("area_subtitle",         50, 18, 40, 10);
    gui.register_coords("goal_stamp",            15, 15, 22, 22);
    gui.register_coords("end_reason",            16, 30, 30,  4);
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
    text_gui_item* area_name_text =
        new text_gui_item(
        game.cur_area_data.name, game.fonts.area_name, COLOR_GOLD
    );
    gui.add_item(area_name_text, "area_name");
    text_to_animate.push_back(area_name_text);
    
    //Area subtitle text.
    string subtitle =
        get_subtitle_or_mission_goal(
            game.cur_area_data.subtitle,
            game.cur_area_data.type,
            game.cur_area_data.mission.goal
        );
    if(!subtitle.empty()) {
        text_gui_item* area_subtitle_text =
            new text_gui_item(subtitle, game.fonts.area_name);
        gui.add_item(area_subtitle_text, "area_subtitle");
        text_to_animate.push_back(area_subtitle_text);
    }
    
    if(game.cur_area_data.type == AREA_TYPE_MISSION) {
        //Goal stamp image item.
        gui_item* goal_stamp_item = new gui_item;
        goal_stamp_item->on_draw =
        [] (const point & center, const point & size) {
            draw_bitmap_in_box(
                game.states.gameplay->mission_fail_reason == 0 ?
                game.sys_assets.bmp_mission_clear :
                game.sys_assets.bmp_mission_fail,
                center,
                size
            );
        };
        gui.add_item(goal_stamp_item, "goal_stamp");
        
        //End reason text, if any.
        string end_reason;
        if(
            has_flag(
                game.states.gameplay->mission_fail_reason,
                MISSION_FAIL_COND_PAUSE_MENU
            )
        ) {
            end_reason = "Ended from the pause menu...";
        } else if(
            has_flag(
                game.states.gameplay->mission_fail_reason,
                MISSION_FAIL_COND_PIKMIN_AMOUNT
            )
        ) {
            end_reason =
                "Reached " +
                i2s(game.cur_area_data.mission.fail_pik_amount) +
                (
                    game.cur_area_data.mission.fail_pik_higher_than ?
                    "+" :
                    "-"
                ) +
                " Pikmin...";
        } else if(
            has_flag(
                game.states.gameplay->mission_fail_reason,
                MISSION_FAIL_COND_LOSE_PIKMIN
            )
        ) {
            end_reason =
                "Lost " +
                i2s(game.cur_area_data.mission.fail_pik_killed) +
                " Pikmin...";
        } else if(
            has_flag(
                game.states.gameplay->mission_fail_reason,
                MISSION_FAIL_COND_TAKE_DAMAGE
            )
        ) {
            end_reason = "A leader took damage...";
        } else if(
            has_flag(
                game.states.gameplay->mission_fail_reason,
                MISSION_FAIL_COND_LOSE_LEADERS
            )
        ) {
            end_reason =
                "Lost " +
                i2s(game.cur_area_data.mission.fail_leaders_kod) +
                " leaders...";
        } else if(
            has_flag(
                game.states.gameplay->mission_fail_reason,
                MISSION_FAIL_COND_KILL_ENEMIES
            )
        ) {
            end_reason =
                "Killed " +
                i2s(game.cur_area_data.mission.fail_enemies_killed) +
                " enemies...";
        } else if(
            has_flag(
                game.states.gameplay->mission_fail_reason,
                MISSION_FAIL_COND_TIME_LIMIT
            )
        ) {
            end_reason =
                "Took " +
                time_to_str(
                    game.cur_area_data.mission.fail_time_limit, "m", "s"
                ) +
                "...";
        } else if(game.cur_area_data.type == AREA_TYPE_MISSION) {
            switch(game.cur_area_data.mission.goal) {
            case MISSION_GOAL_END_MANUALLY: {
                end_reason = "Ended successfully!";
                break;
            } case MISSION_GOAL_COLLECT_TREASURE: {
                if(game.cur_area_data.mission.goal_all_mobs) {
                    end_reason = "Collected all treasures!";
                } else {
                    end_reason =
                        "Collected the " +
                        nr_and_plural(
                            game.cur_area_data.mission.goal_mob_idxs.size(),
                            "treasure"
                        ) +
                        "!";
                }
                break;
            } case MISSION_GOAL_BATTLE_ENEMIES: {
                if(game.cur_area_data.mission.goal_all_mobs) {
                    end_reason = "Defeated all enemies!";
                } else {
                    end_reason =
                        "Defeated the " +
                        nr_and_plural(
                            game.cur_area_data.mission.goal_mob_idxs.size(),
                            "enemy",
                            "enemies"
                        ) +
                        "!";
                }
                break;
            } case MISSION_GOAL_TIMED_SURVIVAL: {
                end_reason =
                    "Survived for " +
                    time_to_str(
                        game.cur_area_data.mission.goal_amount, "m", "s"
                    ) +
                    "!";
                break;
            } case MISSION_GOAL_GET_TO_EXIT: {
                end_reason = "Got to the exit!";
                break;
            } case MISSION_GOAL_REACH_PIKMIN_AMOUNT: {
                end_reason =
                    "Reached " +
                    i2s(game.cur_area_data.mission.goal_amount) +
                    " Pikmin!";
                break;
            }
            }
        }
        
        if(!end_reason.empty()) {
            text_gui_item* end_reason_text =
                new text_gui_item(
                end_reason, game.fonts.standard,
                game.states.gameplay->mission_fail_reason == 0 ?
                al_map_rgba(112, 200, 100, 192) :
                al_map_rgba(242, 160, 160, 192)
            );
            gui.add_item(end_reason_text, "end_reason");
        }
        
        MISSION_MEDALS medal;
        string medal_reason;
        ALLEGRO_COLOR medal_reason_color;
        switch(game.cur_area_data.mission.grading_mode) {
        case MISSION_GRADING_POINTS: {
            medal_reason = "Got " + i2s(final_mission_score) + " points";
            if(
                final_mission_score >=
                game.cur_area_data.mission.platinum_req
            ) {
                medal = MISSION_MEDAL_PLATINUM;
                medal_reason += "!";
                medal_reason_color = al_map_rgba(145, 226, 210, 192);
            } else if(
                final_mission_score >=
                game.cur_area_data.mission.gold_req
            ) {
                medal = MISSION_MEDAL_GOLD;
                medal_reason += "!";
                medal_reason_color = al_map_rgba(233, 200, 80, 192);
            } else if(
                final_mission_score >=
                game.cur_area_data.mission.silver_req
            ) {
                medal = MISSION_MEDAL_SILVER;
                medal_reason += "!";
                medal_reason_color = al_map_rgba(216, 216, 200, 192);
            } else if(
                final_mission_score >=
                game.cur_area_data.mission.bronze_req
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
        } case MISSION_GRADING_GOAL: {
            if(game.states.gameplay->mission_fail_reason == 0) {
                medal = MISSION_MEDAL_PLATINUM;
                medal_reason = "Reached the goal!";
                medal_reason_color = al_map_rgba(145, 226, 210, 192);
            } else {
                medal = MISSION_MEDAL_NONE;
                medal_reason = "Did not reach the goal...";
                medal_reason_color = al_map_rgba(200, 200, 200, 192);
            }
            break;
        } case MISSION_GRADING_PARTICIPATION: {
            medal = MISSION_MEDAL_PLATINUM;
            medal_reason = "Played the mission!";
            medal_reason_color = al_map_rgba(145, 226, 210, 192);
            break;
        }
        }
        
        //Medal image item.
        gui_item* medal_item = new gui_item;
        medal_item->on_draw =
        [medal] (const point & center, const point & size) {
            ALLEGRO_BITMAP* bmp = NULL;
            switch(medal) {
            case MISSION_MEDAL_NONE: {
                bmp = game.sys_assets.bmp_medal_none;
                break;
            } case MISSION_MEDAL_BRONZE: {
                bmp = game.sys_assets.bmp_medal_bronze;
                break;
            } case MISSION_MEDAL_SILVER: {
                bmp = game.sys_assets.bmp_medal_silver;
                break;
            } case MISSION_MEDAL_GOLD: {
                bmp = game.sys_assets.bmp_medal_gold;
                break;
            } case MISSION_MEDAL_PLATINUM: {
                bmp = game.sys_assets.bmp_medal_platinum;
                break;
            }
            }
            draw_bitmap_in_box(bmp, center, size);
        };
        gui.add_item(medal_item, "medal");
        
        //Medal reason.
        text_gui_item* medal_reason_text =
            new text_gui_item(
            medal_reason, game.fonts.standard, medal_reason_color
        );
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
    stats_list = new list_gui_item();
    stats_list->on_draw =
    [this] (const point & center, const point & size) {
        draw_filled_rounded_rectangle(
            center, size, 8.0f, al_map_rgba(0, 0, 0, 40)
        );
        draw_rounded_rectangle(
            center, size, 8.0f, al_map_rgba(255, 255, 255, 128), 1.0f
        );
    };
    gui.add_item(stats_list, "stats");
    
    //Stats list scrollbar.
    scroll_gui_item* stats_scroll = new scroll_gui_item();
    stats_scroll->list_item = stats_list;
    gui.add_item(stats_scroll, "stats_scroll");
    
    //Time taken bullet.
    unsigned char ms = fmod(game.states.gameplay->area_time_passed * 100, 100);
    unsigned char seconds = fmod(game.states.gameplay->area_time_passed, 60);
    size_t minutes = game.states.gameplay->area_time_passed / 60.0f;
    add_stat(
        "Time taken:",
        i2s(minutes) + ":" + pad_string(i2s(seconds), 2, '0') + "." + i2s(ms)
    );
    
    //Pikmin born bullet.
    add_stat("Pikmin born:", i2s(game.states.gameplay->pikmin_born));
    
    if(
        game.cur_area_data.type == AREA_TYPE_MISSION &&
        game.cur_area_data.mission.points_per_pikmin_born != 0
    ) {
        //Pikmin born points bullet.
        if(lost_pikmin_born_score) {
            add_stat(
                "    x 0 points (mission fail) = ",
                "0",
                COLOR_GOLD
            );
        } else {
            add_stat(
                "    x " +
                nr_and_plural(
                    game.cur_area_data.mission.points_per_pikmin_born,
                    "point"
                ) +
                " = ",
                i2s(pikmin_born_score),
                COLOR_GOLD
            );
        }
    }
    
    //Pikmin deaths bullet.
    add_stat("Pikmin deaths:", i2s(game.states.gameplay->pikmin_deaths));
    
    if(
        game.cur_area_data.type == AREA_TYPE_MISSION &&
        game.cur_area_data.mission.points_per_pikmin_death != 0
    ) {
        //Pikmin death points bullet.
        if(lost_pikmin_death_score) {
            add_stat(
                "    x 0 points (mission fail) = ",
                "0",
                COLOR_GOLD
            );
        } else {
            add_stat(
                "    x " +
                nr_and_plural(
                    game.cur_area_data.mission.points_per_pikmin_death,
                    "point"
                ) +
                " = ",
                i2s(pikmin_death_score),
                COLOR_GOLD
            );
        }
    }
    
    if(
        game.cur_area_data.type == AREA_TYPE_MISSION &&
        game.cur_area_data.mission.points_per_sec_left != 0
    ) {
        //Seconds left bullet.
        add_stat("Seconds left:", i2s(secs_left));
        
        //Seconds left points bullet.
        if(lost_secs_left_score) {
            add_stat(
                "    x 0 points (mission fail) = ",
                "0",
                COLOR_GOLD
            );
        } else {
            add_stat(
                "    x " +
                nr_and_plural(
                    game.cur_area_data.mission.points_per_sec_left,
                    "point"
                ) +
                " = ",
                i2s(secs_left_score),
                COLOR_GOLD
            );
        }
    }
    
    if(
        game.cur_area_data.type == AREA_TYPE_MISSION &&
        game.cur_area_data.mission.points_per_sec_passed != 0
    ) {
        //Seconds passed bullet.
        add_stat(
            "Seconds passed:",
            i2s(game.states.gameplay->area_time_passed)
        );
        
        //Seconds passed points bullet.
        if(lost_secs_passed_score) {
            add_stat(
                "    x 0 points (mission fail) = ",
                "0",
                COLOR_GOLD
            );
        } else {
            add_stat(
                "    x " +
                nr_and_plural(
                    game.cur_area_data.mission.points_per_sec_passed,
                    "point"
                ) +
                " = ",
                i2s(secs_passed_score),
                COLOR_GOLD
            );
        }
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
    
    if(
        game.cur_area_data.type == AREA_TYPE_MISSION &&
        game.cur_area_data.mission.points_per_treasure_point != 0
    ) {
        //Treasure points points bullet.
        int max =
            game.states.gameplay->treasure_points_total *
            game.cur_area_data.mission.points_per_treasure_point;
        if(lost_treasure_points_score) {
            add_stat(
                "    x 0 points (mission fail) = ",
                "0/" + i2s(max),
                COLOR_GOLD
            );
        } else {
            add_stat(
                "    x " +
                nr_and_plural(
                    game.cur_area_data.mission.points_per_treasure_point,
                    "point"
                ) +
                " = ",
                i2s(treasure_points_score) + "/" + i2s(max),
                COLOR_GOLD
            );
        }
    }
    
    //Enemy deaths bullet.
    add_stat(
        "Enemy deaths:",
        i2s(game.states.gameplay->enemy_deaths) + "/" +
        i2s(game.states.gameplay->enemy_total)
    );
    
    //Enemy points bullet.
    add_stat(
        "Enemy points:",
        i2s(game.states.gameplay->enemy_points_collected) + "/" +
        i2s(game.states.gameplay->enemy_points_total)
    );
    
    if(
        game.cur_area_data.type == AREA_TYPE_MISSION &&
        game.cur_area_data.mission.points_per_enemy_point != 0
    ) {
        //Enemy points points bullet.
        int max =
            game.states.gameplay->enemy_points_total *
            game.cur_area_data.mission.points_per_enemy_point;
        if(lost_enemy_points_score) {
            add_stat(
                "    x 0 points (mission fail) = ",
                "0/" + i2s(max),
                COLOR_GOLD
            );
        } else {
            add_stat(
                "    x " +
                nr_and_plural(
                    game.cur_area_data.mission.points_per_enemy_point,
                    "point"
                ) +
                " = ",
                i2s(enemy_points_score) + "/" + i2s(max),
                COLOR_GOLD
            );
        }
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
    if(
        game.states.gameplay->mission_fail_reason !=
        MISSION_FAIL_COND_PAUSE_MENU
    ) {
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
    gui_time_spent = 0.0f;
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
