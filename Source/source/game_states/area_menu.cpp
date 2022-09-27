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
 * Adds a new bullet point to either the failure condition list, or the
 * grading explanation list.
 * list:
 *   List to add to.
 * text:
 *   Text.
 */
void area_menu_state::add_bullet(list_gui_item* list, const string &text) {
    size_t bullet_idx = list->children.size();
    const float BULLET_HEIGHT = 0.18f;
    const float BULLET_PADDING = 0.01f;
    const float BULLETS_OFFSET = 0.01f;
    const float bullet_center_y =
        (BULLETS_OFFSET + BULLET_HEIGHT / 2.0f) +
        ((BULLET_HEIGHT + BULLET_PADDING) * bullet_idx);
        
    bullet_point_gui_item* bullet =
        new bullet_point_gui_item(
        text, game.fonts.standard, COLOR_WHITE
    );
    bullet->center = point(0.50f, bullet_center_y);
    bullet->size = point(0.96f, BULLET_HEIGHT);
    list->add_child(bullet);
    gui.add_item(bullet);
}


/* ----------------------------------------------------------------------------
 * Animates the GUI items inside of the info and specs pages.
 */
void area_menu_state::animate_info_and_specs() {
    info_name_text->start_juice_animation(
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
    if(area_type == AREA_TYPE_MISSION) {
        specs_name_text->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
        );
        goal_text->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
        );
        for(size_t c = 0; c < fail_list->children.size(); ++c) {
            fail_list->children[c]->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
            );
        }
        for(size_t c = 0; c < grading_list->children.size(); ++c) {
            grading_list->children[c]->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_ELASTIC_LOW
            );
        }
    }
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
    info_name_text(nullptr),
    specs_name_text(nullptr),
    subtitle_text(nullptr),
    description_text(nullptr),
    difficulty_text(nullptr),
    tags_text(nullptr),
    maker_text(nullptr),
    version_text(nullptr),
    cur_thumb(nullptr),
    goal_text(nullptr),
    fail_list(nullptr),
    grading_list(nullptr),
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
    
    game.fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Ticks time by one frame of logic.
 */
void area_menu_state::do_logic() {
    size_t area_idx = INVALID;
    
    if(!areas_to_pick.empty() && prev_selected_item != gui.selected_item) {
        if(gui.selected_item && gui.selected_item->parent == list_box) {
            //A new area buttons got selected. Figure out which.
            for(size_t c = 0; c < list_box->children.size(); ++c) {
                if(list_box->children[c] == gui.selected_item) {
                    area_idx = c;
                    break;
                }
            }
        }
    }
    
    if(area_idx < areas_to_pick.size()) {
    
        //Start by clearing them all, for sanitization's sake.
        info_name_text->text.clear();
        subtitle_text->text.clear();
        description_text->text.clear();
        difficulty_text->text.clear();
        tags_text->text.clear();
        maker_text->text.clear();
        version_text->text.clear();
        cur_thumb = NULL;
        if(area_type == AREA_TYPE_MISSION) {
            goal_text->text.clear();
            specs_name_text->text.clear();
            fail_list->delete_all_children();
            grading_list->delete_all_children();
        }
        
        //Fill in the area's info.
        info_name_text->text = area_names[area_idx];
        subtitle_text->text =
            get_subtitle_or_mission_goal(
                area_subtitles[area_idx],
                area_type,
                area_mission_data[area_idx].goal
            );
        description_text->text = area_descriptions[area_idx];
        if(area_difficulties[area_idx] == 0) {
            difficulty_text->text.clear();
        } else {
            difficulty_text->text =
                "Difficulty: " +
                i2s(area_difficulties[area_idx]) + "/5 - ";
            switch(area_difficulties[area_idx]) {
            case 1: {
                difficulty_text->text += "Very easy";
                break;
            } case 2: {
                difficulty_text->text += "Easy";
                break;
            } case 3: {
                difficulty_text->text += "Medium";
                break;
            } case 4: {
                difficulty_text->text += "Hard";
                break;
            } case 5: {
                difficulty_text->text += "Very hard";
                break;
            }
            }
        }
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
        
        //Now fill in the mission specs.
        if(area_type == AREA_TYPE_MISSION) {
            specs_name_text->text = area_names[area_idx];
            const mission_data &mission = area_mission_data[area_idx];
            switch(mission.goal) {
            case MISSION_GOAL_END_MANUALLY: {
                goal_text->text =
                    "End from the pause menu whenever you want.";
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
            
            if(
                has_flag(
                    mission.fail_conditions,
                    MISSION_FAIL_COND_TIME_LIMIT
                )
            ) {
                add_bullet(
                    fail_list,
                    "Run out of time. Time limit: " +
                    time_to_str(
                        mission.fail_time_limit, "m", "s"
                    ) + "."
                );
            }
            if(
                has_flag(
                    mission.fail_conditions, MISSION_FAIL_COND_PIKMIN_AMOUNT
                )
            ) {
                add_bullet(
                    fail_list,
                    "Reach " + i2s(mission.fail_pik_amount) + " Pikmin or " +
                    (mission.fail_pik_higher_than ? "more" : "fewer") + "."
                );
            }
            if(
                has_flag(
                    mission.fail_conditions, MISSION_FAIL_COND_LOSE_PIKMIN
                )
            ) {
                add_bullet(
                    fail_list,
                    "Lose " + i2s(mission.fail_pik_killed) + " Pikmin."
                );
            }
            if(
                has_flag(
                    mission.fail_conditions, MISSION_FAIL_COND_TAKE_DAMAGE
                )
            ) {
                add_bullet(
                    fail_list,
                    "A leader takes damage."
                );
            }
            if(
                has_flag(
                    mission.fail_conditions, MISSION_FAIL_COND_LOSE_LEADERS
                )
            ) {
                add_bullet(
                    fail_list,
                    "Lose " +
                    nr_and_plural(mission.fail_leaders_kod, "leader") + "."
                );
            }
            if(
                has_flag(
                    mission.fail_conditions, MISSION_FAIL_COND_KILL_ENEMIES
                )
            ) {
                add_bullet(
                    fail_list,
                    "Kill " +
                    nr_and_plural(
                        mission.fail_enemies_killed, "enemy", "enemies"
                    ) + "."
                );
            }
            if(
                mission.goal != MISSION_GOAL_END_MANUALLY
            ) {
                add_bullet(
                    fail_list,
                    "End from the pause menu."
                );
            }
            
            switch(mission.grading_mode) {
            case MISSION_GRADING_POINTS: {
                add_bullet(
                    grading_list,
                    "Your medal depends on your score:"
                );
                add_bullet(
                    grading_list,
                    "    Platinum: " + i2s(mission.platinum_req) + "+ points."
                );
                add_bullet(
                    grading_list,
                    "    Gold: " + i2s(mission.gold_req) + "+ points."
                );
                add_bullet(
                    grading_list,
                    "    Silver: " + i2s(mission.silver_req) + "+ points."
                );
                add_bullet(
                    grading_list,
                    "    Bronze: " + i2s(mission.bronze_req) + "+ points."
                );
                add_bullet(
                    grading_list,
                    "Your score is calculated like so:"
                );
                if(mission.points_per_pikmin_born != 0) {
                    add_bullet(
                        grading_list,
                        "    Pikmin born x " +
                        i2s(mission.points_per_pikmin_born) + "."
                    );
                }
                if(mission.points_per_pikmin_death != 0) {
                    add_bullet(
                        grading_list,
                        "    Pikmin deaths x " +
                        i2s(mission.points_per_pikmin_death) + "."
                    );
                }
                if(mission.points_per_sec_left != 0) {
                    add_bullet(
                        grading_list,
                        "    Seconds left x " +
                        i2s(mission.points_per_sec_left) + "."
                    );
                }
                if(mission.points_per_sec_passed != 0) {
                    add_bullet(
                        grading_list,
                        "    Seconds passed x " +
                        i2s(mission.points_per_sec_passed) + "."
                    );
                }
                if(mission.points_per_treasure_point != 0) {
                    add_bullet(
                        grading_list,
                        "    Treasure points x " +
                        i2s(mission.points_per_treasure_point) + "."
                    );
                }
                if(mission.points_per_enemy_point != 0) {
                    add_bullet(
                        grading_list,
                        "    Enemy points x " +
                        i2s(mission.points_per_enemy_point) + "."
                    );
                }
                break;
            }
            case MISSION_GRADING_GOAL: {
                add_bullet(
                    grading_list,
                    "You get a platinum medal if you clear the goal."
                );
                add_bullet(
                    grading_list,
                    "You get no medal if you fail."
                );
                break;
            }
            case MISSION_GRADING_PARTICIPATION: {
                add_bullet(
                    grading_list,
                    "You get a platinum medal just by playing the mission."
                );
                break;
            }
            }
        }
        
        animate_info_and_specs();
        
        prev_selected_item = gui.selected_item;
        
    }
    
    gui.tick(game.delta_t);
    
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
}


/* ----------------------------------------------------------------------------
 * Initializes the area info page GUI items.
 */
void area_menu_state::init_gui_info_page() {
    gui.register_coords("info_name",   36,  6, 68,  8);
    gui.register_coords("subtitle",    36, 16, 68,  8);
    gui.register_coords("thumbnail",   85, 14, 26, 24);
    gui.register_coords("description", 50, 42, 96, 28);
    gui.register_coords("high_scores", 50, 66, 96, 16);
    gui.register_coords("difficulty",  50, 79, 96,  6);
    gui.register_coords("tags",        50, 87, 96,  6);
    gui.register_coords("maker",       28, 95, 52,  6);
    gui.register_coords("version",     76, 95, 44,  6);
    gui.read_coords(
        data_node(AREA_MENU::INFO_GUI_FILE_PATH).get_child_by_name("positions")
    );
    
    if(!areas_to_pick.empty()) {
    
        //Name text.
        info_name_text =
            new text_gui_item("", game.fonts.area_name, COLOR_GOLD);
        info_box->add_child(info_name_text);
        gui.add_item(info_name_text, "info_name");
        
        //Subtitle text.
        subtitle_text = new text_gui_item("", game.fonts.area_name);
        info_box->add_child(subtitle_text);
        gui.add_item(subtitle_text, "subtitle");
        
        //Thumbnail.
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
        info_box->add_child(thumb_item);
        gui.add_item(thumb_item, "thumbnail");
        
        //Description text.
        description_text =
            new text_gui_item(
            "", game.fonts.standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
        );
        description_text->line_wrap = true;
        info_box->add_child(description_text);
        gui.add_item(description_text, "description");
        
        //TODO high scores
        
        //Difficulty text.
        difficulty_text =
            new text_gui_item(
            "", game.fonts.standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
        );
        info_box->add_child(difficulty_text);
        gui.add_item(difficulty_text, "difficulty");
        
        //Tags text.
        tags_text =
            new text_gui_item(
            "", game.fonts.standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
        );
        info_box->add_child(tags_text);
        gui.add_item(tags_text, "tags");
        
        //Maker text.
        maker_text =
            new text_gui_item(
            "", game.fonts.standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
        );
        info_box->add_child(maker_text);
        gui.add_item(maker_text, "maker");
        
        //Version text.
        version_text =
            new text_gui_item(
            "", game.fonts.standard, COLOR_WHITE, ALLEGRO_ALIGN_RIGHT
        );
        info_box->add_child(version_text);
        gui.add_item(version_text, "version");
        
    }
}


/* ----------------------------------------------------------------------------
 * Initializes the main GUI items.
 */
void area_menu_state::init_gui_main() {
    gui.register_coords("back",          12,  5, 20,  6);
    gui.register_coords("pick_text",     45,  5, 42,  6);
    gui.register_coords("list",          20, 51, 36, 82);
    gui.register_coords("list_scroll",   40, 51,  2, 82);
    gui.register_coords("view_toggle",   83,  5, 30,  6);
    gui.register_coords("info_box",      70, 51, 56, 82);
    gui.register_coords("specs_box",     70, 51, 56, 82);
    gui.register_coords("tooltip",       50, 96, 95,  4);
    gui.register_coords("no_areas_text", 50, 50, 96, 10);
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
        
        //Info box item.
        info_box = new gui_item();
        info_box->on_draw =
        [] (const point & center, const point & size) {
            draw_rounded_rectangle(
                center, size, 8.0f, al_map_rgba(255, 255, 255, 128), 1.0f
            );
        };
        gui.add_item(info_box, "info_box");
        
        if(area_type == AREA_TYPE_MISSION) {
            //View toggle button.
            button_gui_item* view_toggle_button =
                new button_gui_item(
                "Show mission specs",
                game.fonts.standard
            );
            view_toggle_button->on_activate =
            [this, view_toggle_button] (const point &) {
                gui_item* box_to_show = NULL;
                gui_item* box_to_hide = NULL;
                if(show_mission_specs) {
                    box_to_show = info_box;
                    box_to_hide = specs_box;
                    show_mission_specs = false;
                    view_toggle_button->text = "Show mission specs";
                } else {
                    box_to_show = specs_box;
                    box_to_hide = info_box;
                    show_mission_specs = true;
                    view_toggle_button->text = "Show standard info";
                }
                box_to_show->visible = true;
                box_to_show->responsive = true;
                box_to_hide->visible = false;
                box_to_hide->responsive = false;
                animate_info_and_specs();
            };
            view_toggle_button->on_get_tooltip =
            [] () {
                return "Toggles between basic area info and mission specs.";
            };
            gui.add_item(view_toggle_button, "view_toggle");
            
            //Specs box item.
            specs_box = new gui_item();
            specs_box->on_draw =
            [] (const point & center, const point & size) {
                draw_rounded_rectangle(
                    center, size, 8.0f, al_map_rgba(255, 255, 255, 128), 1.0f
                );
            };
            gui.add_item(specs_box, "specs_box");
            
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
    gui.register_coords("specs_name",     50,  5, 96,  6);
    gui.register_coords("goal_header",    50, 13, 96,  6);
    gui.register_coords("goal",           50, 21, 96,  6);
    gui.register_coords("fail_header",    50, 29, 96,  6);
    gui.register_coords("fail_list",      47, 48, 90, 28);
    gui.register_coords("fail_scroll",    96, 48,  4, 28);
    gui.register_coords("grading_header", 50, 67, 96,  6);
    gui.register_coords("grading_list",   47, 85, 90, 26);
    gui.register_coords("grading_scroll", 96, 85,  4, 26);
    gui.read_coords(
        data_node(AREA_MENU::SPECS_GUI_FILE_PATH).get_child_by_name("positions")
    );
    
    if(!areas_to_pick.empty()) {
    
        //Name text.
        specs_name_text =
            new text_gui_item("", game.fonts.area_name, COLOR_GOLD);
        specs_box->add_child(specs_name_text);
        gui.add_item(specs_name_text, "specs_name");
        
        //Goal header text.
        text_gui_item* goal_header_text =
            new text_gui_item("Goal", game.fonts.area_name);
        specs_box->add_child(goal_header_text);
        gui.add_item(goal_header_text, "goal_header");
        
        //Goal explanation text.
        goal_text =
            new text_gui_item("", game.fonts.standard);
        specs_box->add_child(goal_text);
        gui.add_item(goal_text, "goal");
        
        //Fail conditions header text.
        text_gui_item* fail_header_text =
            new text_gui_item("Fail conditions", game.fonts.area_name);
        specs_box->add_child(fail_header_text);
        gui.add_item(fail_header_text, "fail_header");
        
        //Fail condition explanation list.
        fail_list = new list_gui_item();
        specs_box->add_child(fail_list);
        gui.add_item(fail_list, "fail_list");
        
        //Fail condition explanation scrollbar.
        scroll_gui_item* fail_scroll = new scroll_gui_item();
        fail_scroll->list_item = fail_list;
        specs_box->add_child(fail_scroll);
        gui.add_item(fail_scroll, "fail_scroll");
        
        //Grading header text.
        text_gui_item* grading_header_text =
            new text_gui_item("Grading", game.fonts.area_name);
        specs_box->add_child(grading_header_text);
        gui.add_item(grading_header_text, "grading_header");
        
        //Grading explanation list.
        grading_list = new list_gui_item();
        specs_box->add_child(grading_list);
        gui.add_item(grading_list, "grading_list");
        
        //Grading explanation scrollbar.
        scroll_gui_item* grading_scroll = new scroll_gui_item();
        grading_scroll->list_item = grading_list;
        specs_box->add_child(grading_scroll);
        gui.add_item(grading_scroll, "grading_scroll");
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
    if(area_type == AREA_TYPE_MISSION) {
        init_gui_specs_page();
        specs_box->visible = false;
        specs_box->responsive = false;
    }
    
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
