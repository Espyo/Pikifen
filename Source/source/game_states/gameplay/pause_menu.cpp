/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pause menu classes and functions.
 */

#include "gameplay.h"

#include "../../drawing.h"
#include "../../functions.h"
#include "../../game.h"
#include "../../utils/string_utils.h"


namespace PAUSE_MENU {
//Path to the GUI information file.
const string GUI_FILE_PATH = GUI_FOLDER_PATH + "/Pause_menu.txt";
//Path to the help page GUI information file.
const string HELP_GUI_FILE_PATH = GUI_FOLDER_PATH + "/Pause_help.txt";
//Path to the mission page GUI information file.
const string MISSION_GUI_FILE_PATH = GUI_FOLDER_PATH + "/Pause_mission.txt";
}


/* ----------------------------------------------------------------------------
 * Creates a pause menu struct.
 */
pause_menu_struct::pause_menu_struct() :
    bg_alpha_mult(0.0f),
    closing_timer(0.0f),
    to_delete(false),
    closing(false) {
    
    init_main_pause_menu();
    init_help_page();
    init_mission_page();
}


/* ----------------------------------------------------------------------------
 * Destroys a pause menu struct.
 */
pause_menu_struct::~pause_menu_struct() {
    gui.destroy();
    help_gui.destroy();
    mission_gui.destroy();
}


/* ----------------------------------------------------------------------------
 * Adds a new bullet point to either the failure condition list, or the
 * grading explanation list.
 * list:
 *   List to add to.
 * text:
 *   Text.
 * color:
 *   Text color.
 */
void pause_menu_struct::add_bullet(
    list_gui_item* list, const string &text,
    const ALLEGRO_COLOR &color
) {
    size_t bullet_idx = list->children.size();
    const float BULLET_HEIGHT = 0.18f;
    const float BULLET_PADDING = 0.01f;
    const float BULLETS_OFFSET = 0.01f;
    const float bullet_center_y =
        (BULLETS_OFFSET + BULLET_HEIGHT / 2.0f) +
        ((BULLET_HEIGHT + BULLET_PADDING) * bullet_idx);
        
    bullet_point_gui_item* bullet =
        new bullet_point_gui_item(
        text, game.fonts.standard, color
    );
    bullet->center = point(0.50f, bullet_center_y);
    bullet->size = point(0.96f, BULLET_HEIGHT);
    list->add_child(bullet);
    mission_gui.add_item(bullet);
}


/* ----------------------------------------------------------------------------
 * Draws the pause menu.
 */
void pause_menu_struct::draw() {
    gui.draw();
    help_gui.draw();
    mission_gui.draw();
}


/* ----------------------------------------------------------------------------
 * Draws some help page tidbit's text.
 * font:
 *   Font to use.
 * where:
 *   Coordinates to draw the text on.
 * max_size:
 *   Maximum width or height the text can occupy. A value of zero in
 *   one of these coordinates makes it not have a limit in that dimension.
 * text:
 *   Text to draw.
 */
void pause_menu_struct::draw_tidbit(
    const ALLEGRO_FONT* const font, const point &where,
    const point &max_size, const string &text
) {
    //Get the tokens that make up the tidbit.
    vector<string_token> tokens = tokenize_string(text);
    if(tokens.empty()) return;
    
    int line_height = al_get_font_line_height(font);
    
    set_string_token_widths(tokens, font, game.fonts.slim, line_height);
    
    //Split long lines.
    vector<vector<string_token> > tokens_per_line =
        split_long_string_with_tokens(tokens, max_size.x);
        
    if(tokens_per_line.empty()) return;
    
    //Figure out if we need to scale things vertically.
    //Control icons that are bitmaps will have their width unchanged, otherwise
    //this would turn into a cat-and-mouse game of the Y scale shrinking causing
    //a token width to shrink, which could cause the Y scale to grow,
    //ad infinitum.
    float y_scale = 1.0f;
    if(tokens_per_line.size() * line_height > max_size.y) {
        y_scale = max_size.y / (tokens_per_line.size() * line_height);
    }
    
    //Draw!
    for(size_t l = 0; l < tokens_per_line.size(); ++l) {
        draw_string_tokens(
            tokens_per_line[l], game.fonts.standard, game.fonts.slim,
            point(
                where.x,
                where.y + l * line_height * y_scale -
                (tokens_per_line.size() * line_height * y_scale / 2.0f)
            ),
            ALLEGRO_ALIGN_CENTER, point(max_size.x, line_height * y_scale)
        );
    }
}


/* ----------------------------------------------------------------------------
 * Fills the list of mission fail conditions.
 * list:
 *   List item to fill.
 */
void pause_menu_struct::fill_mission_fail_list(list_gui_item* list) {
    if(
        has_flag(
            game.cur_area_data.mission.fail_conditions,
            MISSION_FAIL_COND_TIME_LIMIT
        )
    ) {
        add_bullet(
            list,
            game.cur_area_data.mission.get_fail_description(
                MISSION_FAIL_COND_TIME_LIMIT
            ),
            al_map_rgb(255, 200, 200)
        );
        float percentage = 0.0f;
        if(game.cur_area_data.mission.fail_time_limit != 0) {
            percentage =
                game.states.gameplay->area_time_passed /
                (float) game.cur_area_data.mission.fail_time_limit;
            percentage *= 100.0f;
        }
        add_bullet(
            list,
            "    " +
            time_to_str(game.states.gameplay->area_time_passed, "m", "s") +
            " have passed so far. (" + i2s(percentage) + "%)"
        );
    }
    if(
        has_flag(
            game.cur_area_data.mission.fail_conditions,
            MISSION_FAIL_COND_PIKMIN_AMOUNT
        )
    ) {
        add_bullet(
            list,
            game.cur_area_data.mission.get_fail_description(
                MISSION_FAIL_COND_PIKMIN_AMOUNT
            ),
            al_map_rgb(255, 200, 200)
        );
        float current =
            game.states.gameplay->get_total_pikmin_amount();
        if(game.cur_area_data.mission.fail_pik_higher_than) {
            float percentage = 0.0f;
            if(game.cur_area_data.mission.fail_pik_amount != 0.0f) {
                percentage =
                    current /
                    (float) game.cur_area_data.mission.fail_pik_amount;
                percentage *= 100;
            }
            add_bullet(
                list,
                "    You have " +
                i2s(current) + "/" +
                i2s(game.cur_area_data.mission.fail_pik_amount) +
                " Pikmin. (" + i2s(percentage) + "%)"
            );
        } else {
            add_bullet(
                list,
                "    You have " + i2s(current) + " Pikmin."
            );
        }
    }
    if(
        has_flag(
            game.cur_area_data.mission.fail_conditions,
            MISSION_FAIL_COND_LOSE_PIKMIN
        )
    ) {
        add_bullet(
            list,
            game.cur_area_data.mission.get_fail_description(
                MISSION_FAIL_COND_LOSE_PIKMIN
            ),
            al_map_rgb(255, 200, 200)
        );
        float percentage = 0.0f;
        if(game.cur_area_data.mission.fail_pik_killed != 0) {
            percentage =
                game.states.gameplay->pikmin_deaths /
                (float) game.cur_area_data.mission.fail_pik_killed;
            percentage *= 100.0f;
        }
        add_bullet(
            list,
            "    You have lost " +
            i2s(game.states.gameplay->pikmin_deaths) +
            "/" +
            i2s(game.cur_area_data.mission.fail_pik_killed) +
            " Pikmin. (" + i2s(percentage) + "%)"
        );
    }
    if(
        has_flag(
            game.cur_area_data.mission.fail_conditions,
            MISSION_FAIL_COND_TAKE_DAMAGE
        )
    ) {
        add_bullet(
            list,
            game.cur_area_data.mission.get_fail_description(
                MISSION_FAIL_COND_TAKE_DAMAGE
            ),
            al_map_rgb(255, 200, 200)
        );
    }
    if(
        has_flag(
            game.cur_area_data.mission.fail_conditions,
            MISSION_FAIL_COND_LOSE_LEADERS
        )
    ) {
        add_bullet(
            list,
            game.cur_area_data.mission.get_fail_description(
                MISSION_FAIL_COND_LOSE_LEADERS
            ),
            al_map_rgb(255, 200, 200)
        );
        float percentage = 0.0f;
        if(game.cur_area_data.mission.fail_leaders_kod != 0) {
            percentage =
                game.states.gameplay->leaders_kod /
                (float) game.cur_area_data.mission.fail_leaders_kod;
            percentage *= 100.0f;
        }
        add_bullet(
            list,
            "    You have lost " +
            i2s(game.states.gameplay->leaders_kod) +
            "/" +
            i2s(game.cur_area_data.mission.fail_leaders_kod) +
            " leaders. (" + i2s(percentage) + "%)"
        );
    }
    if(
        has_flag(
            game.cur_area_data.mission.fail_conditions,
            MISSION_FAIL_COND_KILL_ENEMIES
        )
    ) {
        add_bullet(
            list,
            game.cur_area_data.mission.get_fail_description(
                MISSION_FAIL_COND_KILL_ENEMIES
            ),
            al_map_rgb(255, 200, 200)
        );
        float percentage = 0.0f;
        if(game.cur_area_data.mission.fail_enemies_killed != 0) {
            percentage =
                game.states.gameplay->enemy_deaths /
                (float) game.cur_area_data.mission.fail_enemies_killed;
            percentage *= 100.0f;
        }
        add_bullet(
            list,
            "    You have killed " +
            i2s(game.states.gameplay->enemy_deaths) +
            "/" +
            i2s(game.cur_area_data.mission.fail_enemies_killed) +
            " enemies. (" + i2s(percentage) + "%)"
        );
    }
    if(
        game.cur_area_data.mission.goal !=
        MISSION_GOAL_END_MANUALLY
    ) {
        add_bullet(
            list,
            game.cur_area_data.mission.get_fail_description(
                MISSION_FAIL_COND_PAUSE_MENU
            ),
            al_map_rgb(255, 200, 200)
        );
    }
}


/* ----------------------------------------------------------------------------
 * Fills the list of mission grading information.
 * list:
 *   List item to fill.
 */
void pause_menu_struct::fill_mission_grading_list(list_gui_item* list) {
    switch(game.cur_area_data.mission.grading_mode) {
    case MISSION_GRADING_POINTS: {
        add_bullet(
            list,
            "Your medal depends on your score:"
        );
        add_bullet(
            list,
            "    Platinum: " +
            i2s(game.cur_area_data.mission.platinum_req) + "+ points.",
            al_map_rgb(255, 255, 200)
        );
        add_bullet(
            list,
            "    Gold: " +
            i2s(game.cur_area_data.mission.gold_req) + "+ points.",
            al_map_rgb(255, 255, 200)
        );
        add_bullet(
            list,
            "    Silver: " +
            i2s(game.cur_area_data.mission.silver_req) + "+ points.",
            al_map_rgb(255, 255, 200)
        );
        add_bullet(
            list,
            "    Bronze: " +
            i2s(game.cur_area_data.mission.bronze_req) + "+ points.",
            al_map_rgb(255, 255, 200)
        );
        add_bullet(
            list,
            "Your score is calculated like so:"
        );
        if(game.cur_area_data.mission.points_per_pikmin_born != 0) {
            add_bullet(
                list,
                "    Pikmin born x " +
                i2s(game.cur_area_data.mission.points_per_pikmin_born) + "."
            );
        }
        if(game.cur_area_data.mission.points_per_pikmin_death != 0) {
            add_bullet(
                list,
                "    Pikmin deaths x " +
                i2s(game.cur_area_data.mission.points_per_pikmin_death) + "."
            );
        }
        if(game.cur_area_data.mission.points_per_sec_left != 0) {
            add_bullet(
                list,
                "    Seconds left x " +
                i2s(game.cur_area_data.mission.points_per_sec_left) + "."
            );
        }
        if(game.cur_area_data.mission.points_per_sec_passed != 0) {
            add_bullet(
                list,
                "    Seconds passed x " +
                i2s(game.cur_area_data.mission.points_per_sec_passed) + "."
            );
        }
        if(game.cur_area_data.mission.points_per_treasure_point != 0) {
            add_bullet(
                list,
                "    Treasure points x " +
                i2s(game.cur_area_data.mission.points_per_treasure_point) + "."
            );
        }
        if(game.cur_area_data.mission.points_per_enemy_point != 0) {
            add_bullet(
                list,
                "    Enemy points x " +
                i2s(game.cur_area_data.mission.points_per_enemy_point) + "."
            );
        }
        vector<string> loss_notes;
        if(
            has_flag(
                game.cur_area_data.mission.point_loss_data,
                MISSION_POINT_CRITERIA_PIKMIN_BORN
            )
        ) {
            loss_notes.push_back("    Pikmin born");
        }
        if(
            has_flag(
                game.cur_area_data.mission.point_loss_data,
                MISSION_POINT_CRITERIA_PIKMIN_DEATH
            )
        ) {
            loss_notes.push_back("    Pikmin deaths");
        }
        if(
            has_flag(
                game.cur_area_data.mission.point_loss_data,
                MISSION_POINT_CRITERIA_SEC_LEFT
            )
        ) {
            loss_notes.push_back("    Seconds left");
        }
        if(
            has_flag(
                game.cur_area_data.mission.point_loss_data,
                MISSION_POINT_CRITERIA_SEC_PASSED
            )
        ) {
            loss_notes.push_back("    Seconds passed");
        }
        if(
            has_flag(
                game.cur_area_data.mission.point_loss_data,
                MISSION_POINT_CRITERIA_TREASURE_POINTS
            )
        ) {
            loss_notes.push_back("    Treasure points");
        }
        if(
            has_flag(
                game.cur_area_data.mission.point_loss_data,
                MISSION_POINT_CRITERIA_ENEMY_POINTS
            )
        ) {
            loss_notes.push_back("    Enemy points");
        }
        if(!loss_notes.empty()) {
            add_bullet(
                list,
                "If you fail, you'll lose your score for:"
            );
            for(size_t l = 0; l < loss_notes.size(); ++l) {
                add_bullet(list, loss_notes[l]);
            }
        }
        break;
    } case MISSION_GRADING_GOAL: {
        add_bullet(
            list,
            "You get a platinum medal if you clear the goal."
        );
        add_bullet(
            list,
            "You get no medal if you fail."
        );
        break;
    } case MISSION_GRADING_PARTICIPATION: {
        add_bullet(
            list,
            "You get a platinum medal just by playing the mission."
        );
        break;
    }
    }
}


/* ----------------------------------------------------------------------------
 * Returns a string representing the player's status towards the mission goal.
 */
string pause_menu_struct::get_mission_goal_status() {
    float percentage = 0.0f;
    int cur = game.states.gameplay->goal_cur_amount;
    int req = game.states.gameplay->goal_req_amount;
    if(req != 0.0f) {
        percentage = cur / (float) req;
    }
    percentage *= 100;
    switch(game.cur_area_data.mission.goal) {
    case MISSION_GOAL_END_MANUALLY: {
        return "";
        break;
        
    } case MISSION_GOAL_COLLECT_TREASURE: {
        return
            "You have collected " + i2s(cur) + "/" + i2s(req) +
            " treasures. (" + i2s(percentage) + "%)";
        break;
        
    } case MISSION_GOAL_BATTLE_ENEMIES: {
        return
            "You have killed " + i2s(cur) + "/" + i2s(req) +
            " enemies. (" + i2s(percentage) + "%)";
        break;
        
    } case MISSION_GOAL_TIMED_SURVIVAL: {
        return
            "You have survived for " +
            time_to_str(cur, "m", "s") +
            " so far. (" + i2s(percentage) + "%)";
        break;
        
    } case MISSION_GOAL_GET_TO_EXIT: {
        return
            "You have " + i2s(cur) + "/" + i2s(req) +
            " leaders in the exit. (" + i2s(percentage) + "%)";
        break;
        
    } case MISSION_GOAL_REACH_PIKMIN_AMOUNT: {

        if(game.cur_area_data.mission.goal_higher_than) {
            return
                "You have " + i2s(cur) + "/" + i2s(req) +
                " Pikmin. (" + i2s(percentage) + "%)";
        } else {
            return
                "You have " + i2s(cur) + " Pikmin.";
        }
        break;
        
    }
    }
    return "";
}


/* ----------------------------------------------------------------------------
 * Handles an Allegro event.
 * ev:
 *   Event to handle.
 */
void pause_menu_struct::handle_event(const ALLEGRO_EVENT &ev) {
    gui.handle_event(ev);
    help_gui.handle_event(ev);
    mission_gui.handle_event(ev);
}


/* ----------------------------------------------------------------------------
 * Initializes the help page.
 */
void pause_menu_struct::init_help_page() {
    const vector<string> category_node_names {
        "gameplay", "controls", "", "objects"
    };
    data_node gui_file(PAUSE_MENU::HELP_GUI_FILE_PATH);
    
    //Load the tidbits.
    data_node* tidbits_node = gui_file.get_child_by_name("tidbits");
    
    for(size_t c = 0; c < N_HELP_CATEGORIES; ++c) {
        if(category_node_names[c].empty()) continue;
        data_node* category_node =
            tidbits_node->get_child_by_name(category_node_names[c]);
        size_t n_tidbits = category_node->get_nr_of_children();
        vector<string> &category_tidbits = tidbits[(HELP_CATEGORIES) c];
        category_tidbits.reserve(n_tidbits);
        for(size_t t = 0; t < n_tidbits; ++t) {
            category_tidbits.push_back(category_node->get_child(t)->name);
        }
    }
    for(size_t p = 0; p < game.config.pikmin_order.size(); ++p) {
        tidbits[HELP_CATEGORY_PIKMIN].push_back(
            game.config.pikmin_order[p]->name + ";" +
            game.config.pikmin_order[p]->description
        );
    }
    
    //Menu items.
    help_gui.register_coords("back",        15,  8, 20, 8);
    help_gui.register_coords("gameplay",    22, 25, 35, 10);
    help_gui.register_coords("controls",    22, 37, 35, 10);
    help_gui.register_coords("pikmin",      22, 49, 35, 10);
    help_gui.register_coords("objects",     22, 61, 35, 10);
    help_gui.register_coords("manual",      22, 73, 35, 10);
    help_gui.register_coords("category",    69, 15, 50,  8);
    help_gui.register_coords("list",        69, 50, 50, 60);
    help_gui.register_coords("list_scroll", 96, 50,  2, 60);
    help_gui.register_coords("tooltip",     50, 90, 95, 15);
    help_gui.read_coords(gui_file.get_child_by_name("positions"));
    
    //Back button.
    help_gui.back_item =
        new button_gui_item(
        "Back", game.fonts.standard
    );
    help_gui.back_item->on_activate =
    [this] (const point &) {
        help_gui.responsive = false;
        help_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_UP,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
        gui.responsive = true;
        gui.start_animation(
            GUI_MANAGER_ANIM_UP_TO_CENTER,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
    };
    help_gui.back_item->on_get_tooltip =
    [] () { return "Return to the pause menu."; };
    help_gui.add_item(help_gui.back_item, "back");
    
    //Gameplay button.
    button_gui_item* gameplay_button =
        new button_gui_item("Gameplay", game.fonts.standard);
    gameplay_button->on_activate =
    [this] (const point &) {
        this->populate_help_tidbits(HELP_CATEGORY_GAMEPLAY);
    };
    gameplay_button->on_get_tooltip =
    [] () {
        return "Show help about gameplay features, along with some tips.";
    };
    help_gui.add_item(gameplay_button, "gameplay");
    
    //Controls button.
    button_gui_item* controls_button =
        new button_gui_item("Controls", game.fonts.standard);
    controls_button->on_activate =
    [this] (const point &) {
        this->populate_help_tidbits(HELP_CATEGORY_CONTROLS);
    };
    controls_button->on_get_tooltip =
    [] () {
        return "Show game controls and certain actions you can perform.";
    };
    help_gui.add_item(controls_button, "controls");
    
    //Pikmin button.
    button_gui_item* pikmin_button =
        new button_gui_item("Pikmin types", game.fonts.standard);
    pikmin_button->on_activate =
    [this] (const point &) {
        this->populate_help_tidbits(HELP_CATEGORY_PIKMIN);
    };
    pikmin_button->on_get_tooltip =
    [] () {
        return "Show a description of each Pikmin type.";
    };
    help_gui.add_item(pikmin_button, "pikmin");
    
    //Objects button.
    button_gui_item* objects_button =
        new button_gui_item("Objects", game.fonts.standard);
    objects_button->on_activate =
    [this] (const point &) {
        this->populate_help_tidbits(HELP_CATEGORY_OBJECTS);
    };
    objects_button->on_get_tooltip =
    [] () {
        return "Show help about some noteworthy objects you'll find.";
    };
    help_gui.add_item(objects_button, "objects");
    
    //Manual text.
    bullet_point_gui_item* manual_bullet =
        new bullet_point_gui_item("More help...", game.fonts.standard);
    manual_bullet->on_get_tooltip = [] () {
        return
            "For more help on other subjects, check out the "
            "manual in the game's folder.";
    };
    help_gui.add_item(manual_bullet, "manual");
    
    //Category text.
    help_category_text = new text_gui_item("", game.fonts.standard);
    help_gui.add_item(help_category_text, "category");
    
    //Tidbit list box.
    help_tidbit_list = new list_gui_item();
    help_gui.add_item(help_tidbit_list, "list");
    
    //Tidbit list scrollbar.
    scroll_gui_item* list_scroll = new scroll_gui_item();
    list_scroll->list_item = help_tidbit_list;
    help_gui.add_item(list_scroll, "list_scroll");
    
    //Tooltip text.
    text_gui_item* tooltip_text =
        new text_gui_item("", game.fonts.standard);
    tooltip_text->on_draw =
        [this]
    (const point & center, const point & size) {
        draw_tidbit(
            game.fonts.standard, center, size,
            help_gui.get_current_tooltip()
        );
    };
    help_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    help_gui.set_selected_item(help_gui.back_item);
    help_gui.responsive = false;
    help_gui.hide_items();
}


/* ----------------------------------------------------------------------------
 * Initializes the pause menu's main menu.
 */
void pause_menu_struct::init_main_pause_menu() {
    //Menu items.
    gui.register_coords("header",        50,  5, 52,  6);
    gui.register_coords("left_page",     12,  5, 20,  6);
    gui.register_coords("right_page",    88,  5, 20,  6);
    gui.register_coords("line",          50, 11, 96,  2);
    gui.register_coords("area_name",     50, 20, 96,  8);
    gui.register_coords("area_subtitle", 50, 27, 88,  6);
    gui.register_coords("continue",      13, 88, 22,  8);
    gui.register_coords("retry",         50, 41, 52, 10);
    gui.register_coords("end",           50, 53, 52, 10);
    gui.register_coords("help",          50, 65, 52, 10);
    gui.register_coords("quit",          87, 88, 22,  8);
    gui.register_coords("tooltip",       50, 96, 96,  4);
    gui.read_coords(
        data_node(PAUSE_MENU::GUI_FILE_PATH).get_child_by_name("positions")
    );
    
    //Header.
    text_gui_item* header_text =
        new text_gui_item(
        "PAUSED", game.fonts.area_name,
        al_map_rgba(255, 255, 255, 128)
    );
    gui.add_item(header_text, "header");
    
    if(game.cur_area_data.type == AREA_TYPE_MISSION) {
        //Left page button.
        button_gui_item* left_page_button =
            new button_gui_item(
            "< Mission", game.fonts.standard
        );
        left_page_button->on_activate =
        [this] (const point &) {
            gui.responsive = false;
            gui.start_animation(
                GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
                GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
            );
            mission_gui.responsive = true;
            mission_gui.start_animation(
                GUI_MANAGER_ANIM_LEFT_TO_CENTER,
                GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
            );
        };
        left_page_button->on_get_tooltip =
        [] () { return "Go to the pause menu's mission page."; };
        gui.add_item(left_page_button, "left_page");
        
        //Right page button.
        button_gui_item* right_page_button =
            new button_gui_item(
            "Mission >", game.fonts.standard
        );
        right_page_button->on_activate =
        [this] (const point &) {
            gui.responsive = false;
            gui.start_animation(
                GUI_MANAGER_ANIM_CENTER_TO_LEFT,
                GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
            );
            mission_gui.responsive = true;
            mission_gui.start_animation(
                GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
                GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
            );
        };
        right_page_button->on_get_tooltip =
        [] () { return "Go to the pause menu's mission page."; };
        gui.add_item(right_page_button, "right_page");
    }
    
    //Line.
    gui_item* line = new gui_item();
    line->on_draw =
    [] (const point & center, const point & size) {
        draw_filled_rounded_rectangle(
            center,
            point(size.x, 3.0f),
            2.0f,
            al_map_rgba(255, 255, 255, 128)
        );
    };
    gui.add_item(line, "line");
    
    //Area name.
    text_gui_item* area_name_text =
        new text_gui_item(
        game.cur_area_data.name, game.fonts.area_name,
        change_alpha(COLOR_GOLD, 192)
    );
    gui.add_item(area_name_text, "area_name");
    
    //Area name.
    text_gui_item* area_subtitle_text =
        new text_gui_item(
        get_subtitle_or_mission_goal(
            game.cur_area_data.subtitle, game.cur_area_data.type,
            game.cur_area_data.mission.goal
        ),
        game.fonts.area_name,
        change_alpha(COLOR_WHITE, 192)
    );
    gui.add_item(area_subtitle_text, "area_subtitle");
    
    //Continue button.
    gui.back_item =
        new button_gui_item("Continue", game.fonts.standard);
    gui.back_item->on_activate =
    [this] (const point &) {
        gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_UP,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
        game.states.gameplay->hud->gui.start_animation(
            GUI_MANAGER_ANIM_OUT_TO_IN,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
        start_closing();
    };
    gui.back_item->on_get_tooltip =
    [] () { return "Unpause and continue playing."; };
    gui.add_item(gui.back_item, "continue");
    
    //Retry button.
    button_gui_item* retry_button =
        new button_gui_item(
        game.cur_area_data.type == AREA_TYPE_SIMPLE ?
        "Restart exploration" :
        "Retry mission",
        game.fonts.standard
    );
    retry_button->on_activate =
    [this] (const point &) {
        game.states.gameplay->start_leaving(gameplay_state::LEAVE_TO_RETRY);
    };
    retry_button->on_get_tooltip =
    [] () {
        return
            game.cur_area_data.type == AREA_TYPE_SIMPLE ?
            "Restart this area's exploration." :
            "Retry the mission from the start.";
    };
    gui.add_item(retry_button, "retry");
    
    //End button.
    button_gui_item* end_button =
        new button_gui_item(
        game.cur_area_data.type == AREA_TYPE_SIMPLE ?
        "End exploration" :
        "End mission",
        game.fonts.standard
    );
    end_button->on_activate =
    [this] (const point &) {
        if(game.cur_area_data.mission.goal != MISSION_GOAL_END_MANUALLY) {
            game.states.gameplay->mission_fail_reason =
                MISSION_FAIL_COND_PAUSE_MENU;
        }
        game.states.gameplay->start_leaving(gameplay_state::LEAVE_TO_END);
    };
    end_button->on_get_tooltip =
    [] () {
        return
            game.cur_area_data.type == AREA_TYPE_SIMPLE ?
            "End this area's exploration." :
            game.cur_area_data.mission.goal == MISSION_GOAL_END_MANUALLY ?
            "End this mission successfully." :
            "End this mission as a failure.";
    };
    gui.add_item(end_button, "end");
    
    //Help button.
    button_gui_item* help_button =
        new button_gui_item("Help", game.fonts.standard);
    help_button->on_activate =
    [this] (const point &) {
        gui.responsive = false;
        gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_UP,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
        help_gui.responsive = true;
        help_gui.start_animation(
            GUI_MANAGER_ANIM_UP_TO_CENTER,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
    };
    help_button->on_get_tooltip =
    [] () { return "Some quick help and tips about how to play."; };
    gui.add_item(help_button, "help");
    
    //Quit button.
    button_gui_item* quit_button =
        new button_gui_item(
        game.states.area_ed->quick_play_area_path.empty() ?
        "Quit" :
        "Back to editor",
        game.fonts.standard
    );
    quit_button->on_activate =
    [this] (const point &) {
        game.states.gameplay->start_leaving(
            gameplay_state::LEAVE_TO_AREA_SELECT
        );
    };
    quit_button->on_get_tooltip =
    [] () {
        return
            "Lose your progress and return to the " +
            string(
                game.states.area_ed->quick_play_area_path.empty() ?
                "area selection menu" :
                "area editor"
            ) + ".";
    };
    gui.add_item(quit_button, "quit");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&gui);
    gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    gui.set_selected_item(gui.back_item);
    gui.start_animation(
        GUI_MANAGER_ANIM_UP_TO_CENTER, GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME
    );
}


/* ----------------------------------------------------------------------------
 * Initializes the mission page.
 */
void pause_menu_struct::init_mission_page() {
    data_node gui_file(PAUSE_MENU::MISSION_GUI_FILE_PATH);
    
    //Menu items.
    mission_gui.register_coords("header",         50,  5, 52,  6);
    mission_gui.register_coords("left_page",      12,  5, 20,  6);
    mission_gui.register_coords("right_page",     88,  5, 20,  6);
    mission_gui.register_coords("line",           50, 11, 96,  2);
    mission_gui.register_coords("continue",       10, 16, 16,  4);
    mission_gui.register_coords("goal_header",    50, 16, 60,  4);
    mission_gui.register_coords("goal",           50, 22, 96,  4);
    mission_gui.register_coords("goal_status",    50, 26, 96,  4);
    mission_gui.register_coords("fail_header",    50, 32, 96,  4);
    mission_gui.register_coords("fail_list",      48, 48, 92, 24);
    mission_gui.register_coords("fail_scroll",    97, 48,  2, 24);
    mission_gui.register_coords("grading_header", 50, 64, 96,  4);
    mission_gui.register_coords("grading_list",   48, 80, 92, 24);
    mission_gui.register_coords("grading_scroll", 97, 80,  2, 24);
    mission_gui.register_coords("tooltip",        50, 96, 96,  4);
    mission_gui.read_coords(gui_file.get_child_by_name("positions"));
    
    //Header.
    text_gui_item* header_text =
        new text_gui_item(
        "MISSION", game.fonts.area_name,
        al_map_rgba(255, 255, 255, 128)
    );
    mission_gui.add_item(header_text, "header");
    
    //Left page button.
    button_gui_item* left_page_button =
        new button_gui_item(
        "< System", game.fonts.standard
    );
    left_page_button->on_activate =
    [this] (const point &) {
        mission_gui.responsive = false;
        mission_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_RIGHT,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
        gui.responsive = true;
        gui.start_animation(
            GUI_MANAGER_ANIM_LEFT_TO_CENTER,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
    };
    left_page_button->on_get_tooltip =
    [] () { return "Go to the pause menu's system page."; };
    mission_gui.add_item(left_page_button, "left_page");
    
    //Right page button.
    button_gui_item* right_page_button =
        new button_gui_item(
        "System >", game.fonts.standard
    );
    right_page_button->on_activate =
    [this] (const point &) {
        mission_gui.responsive = false;
        mission_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_LEFT,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
        gui.responsive = true;
        gui.start_animation(
            GUI_MANAGER_ANIM_RIGHT_TO_CENTER,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
    };
    right_page_button->on_get_tooltip =
    [] () { return "Go to the pause menu's system page."; };
    mission_gui.add_item(right_page_button, "right_page");
    
    //Line.
    gui_item* line = new gui_item();
    line->on_draw =
    [] (const point & center, const point & size) {
        draw_filled_rounded_rectangle(
            center,
            point(size.x, 3.0f),
            2.0f,
            al_map_rgba(255, 255, 255, 128)
        );
    };
    mission_gui.add_item(line, "line");
    
    //Continue button.
    mission_gui.back_item =
        new button_gui_item("Continue", game.fonts.standard);
    mission_gui.back_item->on_activate =
    [this] (const point &) {
        mission_gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_UP,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
        game.states.gameplay->hud->gui.start_animation(
            GUI_MANAGER_ANIM_OUT_TO_IN,
            GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
        );
        start_closing();
    };
    mission_gui.back_item->on_get_tooltip =
    [] () { return "Unpause and continue playing."; };
    mission_gui.add_item(mission_gui.back_item, "continue");
    
    //Goal header text.
    text_gui_item* goal_header_text =
        new text_gui_item("Goal", game.fonts.area_name);
    mission_gui.add_item(goal_header_text, "goal_header");
    
    //Goal explanation text.
    text_gui_item* goal_text =
        new text_gui_item(
        game.cur_area_data.mission.get_goal_description(),
        game.fonts.standard,
        al_map_rgb(255, 255, 200)
    );
    mission_gui.add_item(goal_text, "goal");
    
    //Goal status text.
    text_gui_item* goal_status_text =
        new text_gui_item(
        get_mission_goal_status(),
        game.fonts.standard
    );
    mission_gui.add_item(goal_status_text, "goal_status");
    
    //Fail conditions header text.
    text_gui_item* fail_header_text =
        new text_gui_item("Fail conditions", game.fonts.area_name);
    mission_gui.add_item(fail_header_text, "fail_header");
    
    //Fail condition explanation list.
    list_gui_item* mission_fail_list = new list_gui_item();
    mission_gui.add_item(mission_fail_list, "fail_list");
    fill_mission_fail_list(mission_fail_list);
    
    //Fail condition explanation scrollbar.
    scroll_gui_item* fail_scroll = new scroll_gui_item();
    fail_scroll->list_item = mission_fail_list;
    mission_gui.add_item(fail_scroll, "fail_scroll");
    
    //Grading header text.
    text_gui_item* grading_header_text =
        new text_gui_item("Grading", game.fonts.area_name);
    mission_gui.add_item(grading_header_text, "grading_header");
    
    //Grading explanation list.
    list_gui_item* mission_grading_list = new list_gui_item();
    mission_gui.add_item(mission_grading_list, "grading_list");
    fill_mission_grading_list(mission_grading_list);
    
    //Grading explanation scrollbar.
    scroll_gui_item* grading_scroll = new scroll_gui_item();
    grading_scroll->list_item = mission_grading_list;
    mission_gui.add_item(grading_scroll, "grading_scroll");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&mission_gui);
    mission_gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    //TODO mission_gui.set_selected_item(mission_gui.back_item);
    mission_gui.responsive = false;
    mission_gui.hide_items();
}


/* ----------------------------------------------------------------------------
 * Populates the help page's list of tidbits.
 * category:
 *   Category of tidbits to use.
 */
void pause_menu_struct::populate_help_tidbits(const HELP_CATEGORIES category) {
    vector<string> &tidbit_list = tidbits[category];
    
    switch(category) {
    case HELP_CATEGORY_GAMEPLAY: {
        help_category_text->text = "Gameplay";
        break;
    } case HELP_CATEGORY_CONTROLS: {
        help_category_text->text = "Controls";
        break;
    } case HELP_CATEGORY_PIKMIN: {
        help_category_text->text = "Pikmin";
        break;
    } case HELP_CATEGORY_OBJECTS: {
        help_category_text->text = "Objects";
        break;
    } case N_HELP_CATEGORIES: {
        break;
    }
    }
    
    help_tidbit_list->delete_all_children();
    
    for(size_t t = 0; t < tidbit_list.size(); ++t) {
        vector<string> parts = split(tidbit_list[t], ";");
        bullet_point_gui_item* tidbit_bullet =
            new bullet_point_gui_item(
            parts.empty() ? "" : parts[0],
            game.fonts.standard
        );
        tidbit_bullet->center = point(0.50f, 0.045f + t * 0.10f);
        tidbit_bullet->size = point(1.0f, 0.09f);
        tidbit_bullet->on_get_tooltip = [this, parts] () {
            return parts.size() < 2 ? "" : parts[1];
        };
        tidbit_bullet->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_MEDIUM
        );
        help_tidbit_list->add_child(tidbit_bullet);
        help_gui.add_item(tidbit_bullet);
    }
    
    help_category_text->start_juice_animation(
        gui_item::JUICE_TYPE_GROW_TEXT_HIGH
    );
}


/* ----------------------------------------------------------------------------
 * Starts the closing process.
 */
void pause_menu_struct::start_closing() {
    closing = true;
    closing_timer = GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME;
}


/* ----------------------------------------------------------------------------
 * Ticks time by one frame of logic.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 */
void pause_menu_struct::tick(const float delta_t) {
    //Tick the GUI.
    gui.tick(delta_t);
    help_gui.tick(delta_t);
    mission_gui.tick(delta_t);
    
    //Tick the background.
    const float bg_alpha_mult_speed =
        1.0f / GAMEPLAY::MENU_ENTRY_HUD_MOVE_TIME;
    const float diff =
        closing ? -bg_alpha_mult_speed : bg_alpha_mult_speed;
    bg_alpha_mult = clamp(bg_alpha_mult + diff * delta_t, 0.0f, 1.0f);
    
    //Tick the menu closing.
    if(closing) {
        closing_timer -= delta_t;
        if(closing_timer <= 0.0f) {
            to_delete = true;
        }
    }
}
