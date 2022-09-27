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
const string HELP_GUI_FILE_PATH = GUI_FOLDER_PATH + "/Help.txt";
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
}


/* ----------------------------------------------------------------------------
 * Destroys a pause menu struct.
 */
pause_menu_struct::~pause_menu_struct() {
    gui.destroy();
    help_gui.destroy();
}


/* ----------------------------------------------------------------------------
 * Draws the pause menu.
 */
void pause_menu_struct::draw() {
    gui.draw();
    help_gui.draw();
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
 * Handles an Allegro event.
 * ev:
 *   Event to handle.
 */
void pause_menu_struct::handle_event(const ALLEGRO_EVENT &ev) {
    gui.handle_event(ev);
    help_gui.handle_event(ev);
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
    help_tidbit_list_box = new list_gui_item();
    help_gui.add_item(help_tidbit_list_box, "list");
    
    //Tidbit list scrollbar.
    scroll_gui_item* list_scroll = new scroll_gui_item();
    list_scroll->list_item = help_tidbit_list_box;
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
    gui.register_coords("header",   50, 12, 50, 10);
    gui.register_coords("continue", 50, 28, 50,  9);
    gui.register_coords("retry",    50, 39, 50,  9);
    gui.register_coords("end",      50, 50, 50,  9);
    gui.register_coords("help",     50, 61, 50,  9);
    gui.register_coords("quit",     50, 72, 50,  9);
    gui.register_coords("tooltip",  50, 95, 95,  8);
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
    
    //Continue button.
    gui.back_item =
        new button_gui_item("Continue", game.fonts.standard);
    gui.back_item->on_activate =
    [this] (const point &) {
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
            "Quit and return to the " +
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
    
    while(!help_tidbit_list_box->children.empty()) {
        gui_item* i_ptr = help_tidbit_list_box->children[0];
        help_tidbit_list_box->remove_child(i_ptr);
        help_gui.remove_item(i_ptr);
        delete i_ptr;
    }
    
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
        help_tidbit_list_box->add_child(tidbit_bullet);
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
    gui.start_animation(
        GUI_MANAGER_ANIM_CENTER_TO_UP,
        GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
    );
    game.states.gameplay->hud->gui.start_animation(
        GUI_MANAGER_ANIM_OUT_TO_IN,
        GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME
    );
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
