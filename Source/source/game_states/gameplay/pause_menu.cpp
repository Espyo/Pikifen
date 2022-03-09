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
#include "../../game.h"
#include "../../utils/string_utils.h"


//Path to the GUI information file.
const string pause_menu_struct::GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Pause_menu.txt";
//Path to the help page GUI information file.
const string pause_menu_struct::HELP_GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Help.txt";


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
 */
void pause_menu_struct::draw_tidbit(
    const ALLEGRO_FONT* const font, const ALLEGRO_COLOR &color,
    const point &where, const point &scale,
    const int flags, const unsigned char valign,
    const point &max_size, const string &text
) {
    enum TOKEN_TYPES {
        TOKEN_WORD,
        TOKEN_CONTROL
    };
    struct token {
        TOKEN_TYPES type;
        string content;
        int width;
        token() : type(TOKEN_WORD), width(0) {}
    };
    vector<token> tokens;
    token cur_token;
    bool inside_control = false;
    
    //First, figure out which tokens exist.
    for(size_t t = 0; t < text.size(); ++t) {
        if(!inside_control && str_peek(text, t, " ")) {
            if(!cur_token.content.empty()) tokens.push_back(cur_token);
            cur_token.content.clear();
            cur_token.type = TOKEN_WORD;
            
        } else if(str_peek(text, t, "\\k")) {
            if(!cur_token.content.empty()) tokens.push_back(cur_token);
            cur_token.content.clear();
            if(!inside_control) {
                inside_control = true;
                cur_token.type = TOKEN_CONTROL;
            } else {
                inside_control = false;
                cur_token.type = TOKEN_WORD;
            }
            t++;
            
        } else {
            cur_token.content.push_back(text[t]);
            
        }
    }
    if(!cur_token.content.empty()) tokens.push_back(cur_token);
    
    //Get their widths.
    int space_char_width = al_get_text_width(font, " ");
    for(size_t t = 0; t < tokens.size(); ++t) {
        switch(tokens[t].type) {
        case TOKEN_WORD: {
            tokens[t].width =
                al_get_text_width(font, tokens[t].content.c_str());
            break;
        } case TOKEN_CONTROL: {
            tokens[t].content = trim_spaces(tokens[t].content);
            tokens[t].width =
                get_control_icon_width(font, find_control(tokens[t].content));
        }
        }
    }
    
    //Figure out how many lines will be needed.
    vector<int> line_widths;
    int cur_line_width = 0;
    for(size_t t = 0; t < tokens.size(); ++t) {
        int line_width_after_token =
            cur_line_width + tokens[t].width;
        if(cur_line_width > 0) {
            line_width_after_token += space_char_width;
        }
        
        if(cur_line_width > 0 && line_width_after_token >= max_size.x) {
            //Must go to the next line.
            line_widths.push_back(cur_line_width);
            cur_line_width = 0;
        }
        
        if(cur_line_width > 0) {
            cur_line_width += space_char_width;
        }
        cur_line_width += tokens[t].width;
        
        if(
            cur_line_width > 0 &&
            (cur_line_width >= max_size.x || t == tokens.size() - 1)
        ) {
            //We have to finish the current line.
            line_widths.push_back(cur_line_width);
            cur_line_width = 0;
        }
    }
    
    if(line_widths.empty()) return;
    
    //Figure out scales for each line, and for the whole piece.
    float y_scale = 1.0f;
    int line_height = al_get_font_line_height(font);
    if(line_widths.size() * line_height > max_size.y) {
        y_scale = max_size.y / line_widths.size() * line_height;
    }
    
    vector<float> line_scales;
    line_scales.reserve(line_widths.size());
    for(size_t l = 0; l < line_widths.size(); ++l) {
        float line_scale = 1.0f;
        if(line_widths[l] > max_size.x) {
            line_scale = max_size.x / line_widths[l];
        }
        line_scales.push_back(line_scale);
    }
    
    //Draw!
    int cur_line_idx = 0;
    cur_line_width = 0;
    for(size_t t = 0; t < tokens.size(); ++t) {
        int line_width_after_token =
            cur_line_width + tokens[t].width;
        if(cur_line_width > 0) {
            line_width_after_token += space_char_width;
        }
        
        if(cur_line_width > 0 && line_width_after_token >= max_size.x) {
            //Must go to the next line.
            cur_line_idx++;
            cur_line_width = 0;
        }
        
        float x = where.x + cur_line_width * line_scales[cur_line_idx];
        if(cur_line_width > 0) {
            x += space_char_width * line_scales[cur_line_idx];
        }
        if(flags & ALLEGRO_ALIGN_CENTER) {
            x -= (line_widths[cur_line_idx] * line_scales[cur_line_idx]) / 2.0f;
        } else if(flags & ALLEGRO_ALIGN_RIGHT) {
            x -= line_widths[cur_line_idx] * line_scales[cur_line_idx];
        }
        float y = where.y + cur_line_idx * line_height * y_scale;
        if(valign == 1) {
            y -= (line_widths.size() * line_height * y_scale) / 2.0f;
        } else if(valign == 2) {
            y -= line_widths.size() * line_height * y_scale;
        }
        
        switch(tokens[t].type) {
        case TOKEN_WORD: {
            draw_scaled_text(
                font, color,
                point(x, y),
                point(line_scales[cur_line_idx], y_scale),
                ALLEGRO_ALIGN_LEFT, 0, tokens[t].content
            );
            break;
        }
        case TOKEN_CONTROL: {
            draw_control_icon(
                game.fonts.slim,
                find_control(tokens[t].content),
                point(
                    x + (tokens[t].width * line_scales[cur_line_idx]) / 2.0f,
                    y + (line_height * y_scale) / 2.0f
                ),
                point(
                    tokens[t].width * line_scales[cur_line_idx],
                    line_height * y_scale
                )
            );
            break;
        }
        }
        
        if(cur_line_width > 0) {
            cur_line_width += space_char_width;
        }
        cur_line_width += tokens[t].width;
        
        if(cur_line_width >= max_size.x) {
            //We have to finish the current line.
            cur_line_idx++;
            cur_line_width = 0;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Handles an Allegro event.
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
    data_node gui_file(HELP_GUI_FILE_PATH);
    
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
            gameplay_state::MENU_EXIT_HUD_MOVE_TIME
        );
        gui.responsive = true;
        gui.start_animation(
            GUI_MANAGER_ANIM_UP_TO_CENTER,
            gameplay_state::MENU_EXIT_HUD_MOVE_TIME
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
            game.fonts.standard, COLOR_WHITE,
            center, point(1.0f, 1.0f), ALLEGRO_ALIGN_CENTER, 1, size,
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
    gui.register_coords("finish",   50, 50, 50,  9);
    gui.register_coords("help",     50, 61, 50,  9);
    gui.register_coords("quit",     50, 72, 50,  9);
    gui.register_coords("tooltip",  50, 95, 95,  8);
    gui.read_coords(
        data_node(GUI_FILE_PATH).get_child_by_name("positions")
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
        new button_gui_item("Retry day", game.fonts.standard);
    retry_button->on_activate =
    [this] (const point &) {
        game.states.gameplay->start_leaving(gameplay_state::LEAVE_TO_RETRY);
    };
    retry_button->on_get_tooltip =
    [] () { return "Retry this day from the start."; };
    gui.add_item(retry_button, "retry");
    
    //Finish button.
    button_gui_item* finish_button =
        new button_gui_item("Finish day", game.fonts.standard);
    finish_button->on_activate =
    [this] (const point &) {
        game.states.gameplay->start_leaving(gameplay_state::LEAVE_TO_FINISH);
    };
    finish_button->on_get_tooltip =
    [] () { return "Finish playing this day."; };
    gui.add_item(finish_button, "finish");
    
    //Help button.
    button_gui_item* help_button =
        new button_gui_item("Help", game.fonts.standard);
    help_button->on_activate =
    [this] (const point &) {
        gui.responsive = false;
        gui.start_animation(
            GUI_MANAGER_ANIM_CENTER_TO_UP,
            gameplay_state::MENU_EXIT_HUD_MOVE_TIME
        );
        help_gui.responsive = true;
        help_gui.start_animation(
            GUI_MANAGER_ANIM_UP_TO_CENTER,
            gameplay_state::MENU_EXIT_HUD_MOVE_TIME
        );
    };
    help_button->on_get_tooltip =
    [] () { return "Some quick help and tips about how to play."; };
    gui.add_item(help_button, "help");
    
    //Quit button.
    button_gui_item* quit_button =
        new button_gui_item(
        game.states.area_ed->quick_play_area.empty() ?
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
                game.states.area_ed->quick_play_area.empty() ?
                "area selection menu" :
                "area editor"
            ) + ".";
    };
    gui.add_item(quit_button, "quit");
    
    //Tooltip text.
    text_gui_item* tooltip_text =
        new text_gui_item("", game.fonts.standard);
    tooltip_text->on_draw =
        [this]
    (const point & center, const point & size) {
        draw_compressed_scaled_text(
            game.fonts.standard, COLOR_WHITE,
            center, point(0.7f, 0.7f), ALLEGRO_ALIGN_CENTER, 1, size, false,
            gui.get_current_tooltip()
        );
    };
    gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    gui.set_selected_item(gui.back_item);
    gui.start_animation(
        GUI_MANAGER_ANIM_UP_TO_CENTER, gameplay_state::MENU_ENTRY_HUD_MOVE_TIME
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
        help_tidbit_list_box->add_child(tidbit_bullet);
        help_gui.add_item(tidbit_bullet);
    }
}


/* ----------------------------------------------------------------------------
 * Starts the closing process.
 */
void pause_menu_struct::start_closing() {
    closing = true;
    closing_timer = gameplay_state::MENU_EXIT_HUD_MOVE_TIME;
    gui.start_animation(
        GUI_MANAGER_ANIM_CENTER_TO_UP,
        gameplay_state::MENU_EXIT_HUD_MOVE_TIME
    );
    game.states.gameplay->hud->gui.start_animation(
        GUI_MANAGER_ANIM_OUT_TO_IN,
        gameplay_state::MENU_EXIT_HUD_MOVE_TIME
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
    const float bg_alpha_mult_speed = 1.0f / gameplay_state::MENU_ENTRY_HUD_MOVE_TIME;
    const float diff = closing ? -bg_alpha_mult_speed : bg_alpha_mult_speed;
    bg_alpha_mult = clamp(bg_alpha_mult + diff * delta_t, 0.0f, 1.0f);
    
    //Tick the menu closing.
    if(closing) {
        closing_timer -= delta_t;
        if(closing_timer <= 0.0f) {
            to_delete = true;
        }
    }
}
