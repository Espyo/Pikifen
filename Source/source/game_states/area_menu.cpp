/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Area menu state class and area menu state-related functions.
 */

#include <algorithm>

#include "menus.h"

#include "../drawing.h"
#include "../functions.h"
#include "../game.h"
#include "../load.h"
#include "../utils/string_utils.h"


namespace AREA_MENU {
//Path to the area info GUI information file.
const string INFO_GUI_FILE_PATH = GUI_FOLDER_PATH + "/Area_menu_info.txt";
//Path to the main GUI information file.
const string GUI_FILE_PATH = GUI_FOLDER_PATH + "/Area_menu.txt";
//How long to animate the page swapping for.
const float PAGE_SWAP_DURATION = 0.5f;
//Path to the mission specs GUI information file.
const string SPECS_GUI_FILE_PATH = GUI_FOLDER_PATH + "/Area_menu_specs.txt";
}


/* ----------------------------------------------------------------------------
 * Creates an "area menu" state.
 */
area_menu_state::area_menu_state() :
    game_state(),
    area_type(AREA_TYPE_SIMPLE),
    bmp_menu_bg(nullptr),
    prev_selected_item(nullptr),
    list_box(nullptr),
    name_text(nullptr),
    subtitle_text(nullptr),
    description_text(nullptr),
    difficulty_text(nullptr),
    tags_text(nullptr),
    maker_text(nullptr),
    version_text(nullptr),
    cur_thumb(nullptr),
    goal_text(nullptr),
    time_limit_text(nullptr),
    loss_text(nullptr),
    grading_mode_text(nullptr),
    grading_criteria_text(nullptr),
    medal_scores_text(nullptr),
    platinum_medal_text(nullptr),
    show_mission_specs(false) {
    
}


/* ----------------------------------------------------------------------------
 * Draws the area menu.
 */
void area_menu_state::do_drawing() {
    al_clear_to_color(COLOR_BLACK);
    draw_bitmap(
        bmp_menu_bg, point(game.win_w * 0.5, game.win_h * 0.5),
        point(game.win_w, game.win_h), 0, map_gray(64)
    );
    
    gui.draw();
    info_gui.draw();
    specs_gui.draw();
    
    game.fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Ticks time by one frame of logic.
 */
void area_menu_state::do_logic() {
    if(!areas_to_pick.empty() && prev_selected_item != gui.selected_item) {
    
        size_t area_idx = INVALID;
        
        name_text->text.clear();
        subtitle_text->text.clear();
        description_text->text.clear();
        difficulty_text->text.clear();
        tags_text->text.clear();
        maker_text->text.clear();
        version_text->text.clear();
        cur_thumb = NULL;
        goal_text->text.clear();
        time_limit_text->text.clear();
        loss_text->text.clear();
        grading_mode_text->text.clear();
        grading_criteria_text->text.clear();
        medal_scores_text->text.clear();
        platinum_medal_text->text.clear();
        
        name_text->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
        );
        subtitle_text->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
        );
        description_text->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
        );
        difficulty_text->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
        );
        tags_text->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
        );
        maker_text->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
        );
        version_text->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
        );
        goal_text->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
        );
        time_limit_text->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
        );
        loss_text->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
        );
        grading_mode_text->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
        );
        grading_criteria_text->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
        );
        medal_scores_text->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_MEDIUM
        );
        platinum_medal_text->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
        );
        
        if(gui.selected_item && gui.selected_item->parent == list_box) {
            //One of the area buttons is selected.
            //Figure out which.
            
            for(size_t c = 0; c < list_box->children.size(); ++c) {
                if(list_box->children[c] == gui.selected_item) {
                    area_idx = c;
                    break;
                }
            }
        }
        
        if(area_idx < areas_to_pick.size()) {
            name_text->text =
                area_names[area_idx];
            subtitle_text->text =
                get_subtitle_or_mission_goal(
                    area_subtitles[area_idx],
                    area_type,
                    area_mission_data[area_idx].goal
                );
            description_text->text =
                area_descriptions[area_idx];
            difficulty_text->text =
                (
                    area_difficulties[area_idx] == 0 ?
                    "" :
                    "Difficulty: " + i2s(area_difficulties[area_idx])
                );
            tags_text->text =
                (
                    area_tags[area_idx].empty() ?
                    "" :
                    "Tags: " + area_tags[area_idx]
                );
            maker_text->text =
                (
                    area_makers[area_idx].empty() ?
                    "Unknown maker" :
                    "Maker: " + area_makers[area_idx]
                );
            version_text->text =
                (
                    area_versions[area_idx].empty() ?
                    "" :
                    "Version: " + area_versions[area_idx]
                );
            cur_thumb = area_thumbs[area_idx];
            
            const mission_data &mission = area_mission_data[area_idx];
            switch(mission.goal) {
            case MISSION_GOAL_NONE: {
                goal_text->text = "None.";
                break;
            } case MISSION_GOAL_COLLECT_TREASURE: {
                if(mission.goal_all_mobs) {
                    goal_text->text = "Collect all treasures.";
                } else {
                    goal_text->text =
                        "Collect the specified treasures (" +
                        i2s(mission.goal_mob_idxs.size()) +
                        ").";
                }
                break;
            } case MISSION_GOAL_BATTLE_ENEMIES: {
                if(mission.goal_all_mobs) {
                    goal_text->text = "Defeat all enemies.";
                } else {
                    goal_text->text =
                        "Defeat the specified enemies (" +
                        i2s(mission.goal_mob_idxs.size()) +
                        ").";
                }
                break;
            } case MISSION_GOAL_TIMED_SURVIVAL: {
                goal_text->text =
                    "Survive for " +
                    time_to_str(
                        mission.goal_amount, "m", "s"
                    ) + ".";
                break;
            } case MISSION_GOAL_GET_TO_EXIT: {
                if(mission.goal_all_mobs) {
                    goal_text->text = "Get all leaders to the exit.";
                } else {
                    goal_text->text =
                        "Get the specified leaders (" +
                        i2s(mission.goal_mob_idxs.size()) +
                        ") to the exit.";
                }
                break;
            } case MISSION_GOAL_REACH_PIKMIN_AMOUNT: {
                goal_text->text =
                    "Reach a total of " +
                    i2s(mission.goal_amount) + " " +
                    (
                        mission.goal_higher_than ?
                        "or more" :
                        "or fewer"
                    ) +
                    " Pikmin.";
                break;
            } default: {
                break;
            }
            }
            time_limit_text->text =
                (
                    mission.loss_time_limit == 0 ||
                    !has_flag(
                        mission.loss_conditions,
                        MISSION_LOSS_COND_TIME_LIMIT
                    )
                ) ?
                "Time limit: none" :
                "Time limit: " +
                time_to_str(
                    mission.loss_time_limit, "m", "s"
                );
            loss_text->text =
                "- Lose all leaders\\n"
                "- Finish from the pause menu\\n";
            if(
                has_flag(
                    mission.loss_conditions, MISSION_LOSS_COND_PIKMIN_AMOUNT
                )
            ) {
                loss_text->text +=
                    "- Reach " + i2s(mission.loss_pik_amount) + " Pikmin or " +
                    (mission.loss_pik_higher_than ? "more" : "fewer") + "\\n";
            }
            if(
                has_flag(
                    mission.loss_conditions, MISSION_LOSS_COND_LOSE_PIKMIN
                )
            ) {
                loss_text->text +=
                    "- Lose " + i2s(mission.loss_pik_killed) + " Pikmin\\n";
            }
            if(
                has_flag(
                    mission.loss_conditions, MISSION_LOSS_COND_TAKE_DAMAGE
                )
            ) {
                loss_text->text +=
                    "- A leader takes damage\\n";
            }
            if(
                has_flag(
                    mission.loss_conditions, MISSION_LOSS_COND_LOSE_LEADERS
                )
            ) {
                loss_text->text +=
                    "- Lose " +
                    nr_and_plural(mission.loss_leaders_kod, "leader") + "\\n";
            }
            if(
                has_flag(
                    mission.loss_conditions, MISSION_LOSS_COND_KILL_ENEMIES
                )
            ) {
                loss_text->text +=
                    "- Kill " +
                    nr_and_plural(
                        mission.loss_enemies_killed, "enemy", "enemies"
                    ) + "\\n";
            }
            if(
                has_flag(
                    mission.loss_conditions, MISSION_LOSS_COND_TIME_LIMIT
                )
            ) {
                loss_text->text +=
                    "- Reach the time limit\\n";
            }
            //Erase the last \n on the loss text.
            if(!loss_text->text.empty()) {
                loss_text->text.erase(loss_text->text.size() - 1);
                loss_text->text.erase(loss_text->text.size() - 1);
            }
            
            grading_criteria_text->text.clear();
            switch(mission.grading_mode) {
            case MISSION_GRADING_POINTS: {
                grading_mode_text->text =
                    "Based on score:";
                if(mission.points_per_pikmin_born != 0) {
                    grading_criteria_text->text +=
                        "- Pikmin birth x " +
                        i2s(mission.points_per_pikmin_born) + "\\n";
                }
                if(mission.points_per_pikmin_death != 0) {
                    grading_criteria_text->text +=
                        "- Pikmin death x " +
                        i2s(mission.points_per_pikmin_death) + "\\n";
                }
                if(mission.points_per_sec_left != 0) {
                    grading_criteria_text->text +=
                        "- Time left x " +
                        i2s(mission.points_per_sec_left) + "\\n";
                }
                if(mission.points_per_sec_passed != 0) {
                    grading_criteria_text->text +=
                        "- Time passed x " +
                        i2s(mission.points_per_sec_passed) + "\\n";
                }
                if(mission.points_per_treasure_point != 0) {
                    grading_criteria_text->text +=
                        "- Treasure point x " +
                        i2s(mission.points_per_treasure_point) + "\\n";
                }
                if(mission.points_per_enemy_point != 0) {
                    grading_criteria_text->text +=
                        "- Enemy point x " +
                        i2s(mission.points_per_enemy_point) + "\\n";
                }
                medal_scores_text->text =
                    "Bronze: " + i2s(mission.bronze_req) + "+ points\\n"
                    "Silver: " + i2s(mission.silver_req) + "+ points\\n"
                    "Gold: " + i2s(mission.gold_req) + "+ points\\n";
                platinum_medal_text->text =
                    "Platinum: " +
                    i2s(mission.platinum_req) + "+ points";
                break;
            }
            case MISSION_GRADING_GOAL: {
                grading_mode_text->text =
                    "Based on the goal";
                platinum_medal_text->text =
                    "Platinum: Reach the goal";
                break;
            }
            case MISSION_GRADING_PARTICIPATION: {
                grading_mode_text->text =
                    "Based on playing";
                platinum_medal_text->text =
                    "Platinum: Just play";
                break;
            }
            }
        }
        
        prev_selected_item = gui.selected_item;
        
    }
    
    gui.tick(game.delta_t);
    info_gui.tick(game.delta_t);
    specs_gui.tick(game.delta_t);
    
    game.fade_mgr.tick(game.delta_t);
}


/* ----------------------------------------------------------------------------
 * Returns the name of this state.
 */
string area_menu_state::get_name() const {
    return "area menu";
}


/* ----------------------------------------------------------------------------
 * Handles Allegro events.
 * ev:
 *   Event to handle.
 */
void area_menu_state::handle_allegro_event(ALLEGRO_EVENT &ev) {
    if(game.fade_mgr.is_fading()) return;
    
    gui.handle_event(ev);
    info_gui.handle_event(ev);
    specs_gui.handle_event(ev);
}


/* ----------------------------------------------------------------------------
 * Initializes the area info page GUI items.
 */
void area_menu_state::init_gui_info_page() {
    info_gui.register_coords("info_box",      67,   51, 58, 78);
    info_gui.register_coords("name",          56,   18, 32,  8);
    info_gui.register_coords("subtitle",      56,   28, 32,  8);
    info_gui.register_coords("thumbnail",     84,   24, 20, 20);
    info_gui.register_coords("description",   67,   45, 54, 18);
    info_gui.register_coords("high_scores",   67,   63, 54, 14);
    info_gui.register_coords("difficulty",    67,   74, 54,  4);
    info_gui.register_coords("tags",          67,   80, 54,  4);
    info_gui.register_coords("maker",         54,   86, 28,  4);
    info_gui.register_coords("version",       82,   86, 24,  4);
    info_gui.read_coords(
        data_node(AREA_MENU::INFO_GUI_FILE_PATH).get_child_by_name("positions")
    );
    
    if(!areas_to_pick.empty()) {
    
        //Area info box.
        list_gui_item* info_box = new list_gui_item();
        info_gui.add_item(info_box, "info_box");
        
        //Name text.
        name_text = new text_gui_item("", game.fonts.area_name);
        info_gui.add_item(name_text, "name");
        
        //Subtitle text.
        subtitle_text = new text_gui_item("", game.fonts.area_name);
        info_gui.add_item(subtitle_text, "subtitle");
        
        gui_item* thumb_item = new gui_item();
        thumb_item->on_draw =
        [this] (const point & center, const point & size) {
            //Make it a square.
            point final_size(
                std::min(size.x, size.y),
                std::min(size.x, size.y)
            );
            //Align it to the top-right corner.
            point final_center(
                (center.x + size.x / 2.0f) - final_size.x / 2.0f,
                (center.y - size.y / 2.0f) + final_size.y / 2.0f
            );
            if(cur_thumb) {
                draw_bitmap(cur_thumb, final_center, final_size - 4.0f);
            }
            draw_rounded_rectangle(
                final_center, final_size, 8.0f,
                al_map_rgba(255, 255, 255, 128), 1.0f
            );
        };
        info_gui.add_item(thumb_item, "thumbnail");
        
        //Description text.
        description_text =
            new text_gui_item(
            "", game.fonts.standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
        );
        description_text->line_wrap = true;
        info_gui.add_item(description_text, "description");
        
        //TODO high scores
        
        //Difficulty text.
        difficulty_text =
            new text_gui_item(
            "", game.fonts.standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
        );
        info_gui.add_item(difficulty_text, "difficulty");
        
        //Tags text.
        tags_text =
            new text_gui_item(
            "", game.fonts.standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
        );
        info_gui.add_item(tags_text, "tags");
        
        //Maker text.
        maker_text =
            new text_gui_item(
            "", game.fonts.standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
        );
        info_gui.add_item(maker_text, "maker");
        
        //Version text.
        version_text =
            new text_gui_item(
            "", game.fonts.standard, COLOR_WHITE, ALLEGRO_ALIGN_RIGHT
        );
        info_gui.add_item(version_text, "version");
        
    }
}


/* ----------------------------------------------------------------------------
 * Initializes the main GUI items.
 */
void area_menu_state::init_gui_main() {
    gui.register_coords("back",          14,    7, 20,  6);
    gui.register_coords("pick_text",     45.5,  7, 39,  6);
    gui.register_coords("list",          19,   51, 30, 78);
    gui.register_coords("list_scroll",   36,   51,  2, 78);
    gui.register_coords("mission_specs", 81,    7, 30,  6);
    gui.register_coords("tooltip",       50,   95, 95,  8);
    gui.register_coords("no_areas_text", 50,   50, 96, 10);
    gui.read_coords(
        data_node(AREA_MENU::GUI_FILE_PATH).get_child_by_name("positions")
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
    
    //Instructions text.
    text_gui_item* pick_text =
        new text_gui_item(
        area_type == AREA_TYPE_SIMPLE ?
        "Pick a simple area:" :
        "Pick a mission:",
        game.fonts.standard
    );
    gui.add_item(pick_text, "pick_text");
    
    button_gui_item* first_area_button = NULL;
    
    if(!areas_to_pick.empty()) {
    
        //Area list box.
        list_box = new list_gui_item();
        gui.add_item(list_box, "list");
        
        //Area list scrollbar.
        scroll_gui_item* list_scroll = new scroll_gui_item();
        list_scroll->list_item = list_box;
        gui.add_item(list_scroll, "list_scroll");
        
        //Items for the various areas.
        for(size_t a = 0; a < areas_to_pick.size(); ++a) {
            string area_name = area_names[a];
            string area_folder = areas_to_pick[a];
            
            //Area button.
            button_gui_item* area_button =
                new button_gui_item(area_name, game.fonts.standard);
            area_button->center = point(0.50f, 0.045f + a * 0.10f);
            area_button->size = point(1.0f, 0.09f);
            area_button->on_activate =
            [this, area_folder] (const point &) {
                game.states.gameplay->path_of_area_to_load =
                    get_base_area_folder_path(area_type, true) + "/" +
                    area_folder;
                game.fade_mgr.start_fade(false, [] () {
                    game.change_state(game.states.gameplay);
                });
            };
            area_button->on_get_tooltip =
            [area_name] () { return "Play " + area_name + "."; };
            list_box->add_child(area_button);
            gui.add_item(area_button);
            if(!first_area_button) {
                first_area_button = area_button;
            }
        }
        
        if(area_type == AREA_TYPE_MISSION) {
            //Mission specs button.
            button_gui_item* specs_button =
                new button_gui_item(
                "Show mission specs",
                game.fonts.standard
            );
            specs_button->on_activate =
            [this, specs_button] (const point &) {
                if(show_mission_specs) {
                    specs_gui.start_animation(
                        GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
                        AREA_MENU::PAGE_SWAP_DURATION
                    );
                    specs_gui.responsive = false;
                    info_gui.start_animation(
                        GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
                        AREA_MENU::PAGE_SWAP_DURATION
                    );
                    info_gui.responsive = true;
                    info_gui.show_items();
                    show_mission_specs = false;
                    specs_button->text = "Show mission specs";
                } else {
                    info_gui.start_animation(
                        GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
                        AREA_MENU::PAGE_SWAP_DURATION
                    );
                    info_gui.responsive = false;
                    specs_gui.start_animation(
                        GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
                        AREA_MENU::PAGE_SWAP_DURATION
                    );
                    specs_gui.responsive = true;
                    specs_gui.show_items();
                    show_mission_specs = true;
                    specs_button->text = "Show standard info";
                }
            };
            gui.add_item(specs_button, "mission_specs");
        }
        
    } else {
    
        //No areas found text.
        text_gui_item* no_areas_text =
            new text_gui_item(
            "No areas found! Try making your own in the area editor!",
            game.fonts.standard
        );
        gui.add_item(no_areas_text, "no_areas_text");
        
    }
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&gui);
    gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    if(areas_to_pick.size() > 1) {
        gui.set_selected_item(first_area_button);
    }
}


/* ----------------------------------------------------------------------------
 * Initializes the mission specs page GUI items.
 */
void area_menu_state::init_gui_specs_page() {
    specs_gui.register_coords("specs_box",        67, 51, 58, 78);
    specs_gui.register_coords("goal_header",      67, 16, 54,  4);
    specs_gui.register_coords("goal",             67, 22, 54,  4);
    specs_gui.register_coords("loss_header",      53, 28, 26,  4);
    specs_gui.register_coords("loss",             53, 26, 26, 50);
    specs_gui.register_coords("time_limit",       53, 86, 26,  4);
    specs_gui.register_coords("grading_header",   81, 28, 26,  4);
    specs_gui.register_coords("grading_mode",     81, 34, 26,  4);
    specs_gui.register_coords("grading_criteria", 81, 55, 26, 34);
    specs_gui.register_coords("medal_scores",     81, 78, 26,  8);
    specs_gui.register_coords("platinum_medal",   81, 86, 26,  4);
    
    specs_gui.read_coords(
        data_node(AREA_MENU::SPECS_GUI_FILE_PATH).get_child_by_name("positions")
    );
    
    if(!areas_to_pick.empty()) {
        //Mission specs box.
        list_gui_item* specs_box = new list_gui_item();
        specs_gui.add_item(specs_box, "specs_box");
        
        //Goal header text.
        text_gui_item* goal_header_text =
            new text_gui_item("Goal", game.fonts.area_name);
        specs_gui.add_item(goal_header_text, "goal_header");
        
        //Goal explanation text.
        goal_text =
            new text_gui_item("", game.fonts.standard);
        specs_gui.add_item(goal_text, "goal");
        
        //Loss conditions header text.
        text_gui_item* loss_header_text =
            new text_gui_item("Loss conditions", game.fonts.area_name);
        specs_gui.add_item(loss_header_text, "loss_header");
        
        //Time limit text.
        time_limit_text =
            new text_gui_item("", game.fonts.standard);
        specs_gui.add_item(time_limit_text, "time_limit");
        
        //Loss condition explanation text.
        loss_text =
            new text_gui_item(
            "", game.fonts.standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
        );
        loss_text->line_wrap = true;
        specs_gui.add_item(loss_text, "loss");
        
        //Grading header text.
        text_gui_item* grading_header_text =
            new text_gui_item("Grading", game.fonts.area_name);
        specs_gui.add_item(grading_header_text, "grading_header");
        
        //Grading mode text.
        grading_mode_text =
            new text_gui_item("", game.fonts.standard);
        specs_gui.add_item(grading_mode_text, "grading_mode");
        
        //Grading criteria text.
        grading_criteria_text =
            new text_gui_item(
            "", game.fonts.standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
        );
        grading_criteria_text->line_wrap = true;
        specs_gui.add_item(grading_criteria_text, "grading_criteria");
        
        //Medal scores text.
        medal_scores_text =
            new text_gui_item(
            "", game.fonts.standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
        );
        medal_scores_text->line_wrap = true;
        specs_gui.add_item(medal_scores_text, "medal_scores");
        
        //Platinum medal text.
        platinum_medal_text =
            new text_gui_item("", game.fonts.standard);
        specs_gui.add_item(platinum_medal_text, "platinum_medal");
    }
}


/* ----------------------------------------------------------------------------
 * Leaves the area menu and goes into the main menu.
 */
void area_menu_state::leave() {
    game.fade_mgr.start_fade(false, [] () {
        game.states.main_menu->page_to_load = MAIN_MENU_PAGE_PLAY;
        game.change_state(game.states.main_menu);
    });
}


/* ----------------------------------------------------------------------------
 * Loads the area menu into memory.
 */
void area_menu_state::load() {
    bmp_menu_bg = NULL;
    prev_selected_item = NULL;
    cur_thumb = NULL;
    show_mission_specs = false;
    
    //Areas.
    areas_to_pick =
        folder_to_vector(
            get_base_area_folder_path(area_type, true),
            true
        );
        
    for(size_t a = 0; a < areas_to_pick.size(); ++a) {
        string actual_name = areas_to_pick[a];
        data_node data(
            get_base_area_folder_path(area_type, true) +
            "/" + actual_name + "/" + AREA_DATA_FILE_NAME
        );
        if(data.file_was_opened) {
            string s = data.get_child_by_name("name")->value;
            if(!s.empty()) {
                actual_name = s;
            }
        }
        
        area_names.push_back(
            actual_name
        );
        area_subtitles.push_back(
            data.get_child_by_name("subtitle")->value
        );
        area_descriptions.push_back(
            data.get_child_by_name("description")->value
        );
        area_difficulties.push_back(
            s2i(data.get_child_by_name("difficulty")->value)
        );
        area_tags.push_back(
            data.get_child_by_name("tags")->value
        );
        area_makers.push_back(
            data.get_child_by_name("maker")->value
        );
        area_versions.push_back(
            data.get_child_by_name("version")->value
        );
        
        string thumbnail_path =
            get_base_area_folder_path(area_type, true) +
            "/" + areas_to_pick[a] + "/Thumbnail.png";
            
        area_thumbs.push_back(al_load_bitmap(thumbnail_path.c_str()));
        
        area_mission_data.push_back(mission_data());
        load_area_mission_data(&data, area_mission_data.back());
    }
    
    bmp_menu_bg = load_bmp(game.asset_file_names.main_menu);
    
    init_gui_main();
    init_gui_info_page();
    init_gui_specs_page();
    
    info_gui.show_items();
    info_gui.responsive = true;
    specs_gui.hide_items();
    specs_gui.responsive = false;
    
    game.fade_mgr.start_fade(true, nullptr);
    
}


/* ----------------------------------------------------------------------------
 * Unloads the area menu from memory.
 */
void area_menu_state::unload() {

    //Resources.
    al_destroy_bitmap(bmp_menu_bg);
    
    //Menu items.
    gui.destroy();
    info_gui.destroy();
    specs_gui.destroy();
    
    //Misc
    areas_to_pick.clear();
    area_names.clear();
    area_subtitles.clear();
    area_descriptions.clear();
    area_difficulties.clear();
    area_tags.clear();
    area_makers.clear();
    area_versions.clear();
    area_mission_data.clear();
    
    cur_thumb = NULL;
    for(size_t a = 0; a < area_thumbs.size(); ++a) {
        if(area_thumbs[a]) {
            al_destroy_bitmap(area_thumbs[a]);
        }
    }
    area_thumbs.clear();
    
}
