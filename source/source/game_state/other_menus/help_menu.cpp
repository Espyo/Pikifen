/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Help menu structs and functions.
 */

#include "help_menu.h"

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../util/string_utils.h"


namespace HELP_MENU {

//Name of the help menu GUI information file.
const string GUI_FILE_NAME = "help";

}


/**
 * @brief Constructs a new help menu object.
 */
help_menu_t::help_menu_t() {
    const vector<string> category_node_names {
        "gameplay_basics", "advanced_gameplay", "controls", "", "objects"
    };
    data_node* gui_file = &game.content.gui_defs.list[HELP_MENU::GUI_FILE_NAME];
    
    //Load the tidbits.
    data_node* tidbits_node = gui_file->get_child_by_name("tidbits");
    
    for(size_t c = 0; c < N_HELP_CATEGORIES; c++) {
        if(category_node_names[c].empty()) continue;
        data_node* category_node =
            tidbits_node->get_child_by_name(category_node_names[c]);
        size_t n_tidbits = category_node->get_nr_of_children();
        vector<tidbit> &category_tidbits = tidbits[(HELP_CATEGORY) c];
        category_tidbits.reserve(n_tidbits);
        for(size_t t = 0; t < n_tidbits; t++) {
            vector<string> parts =
                split(category_node->get_child(t)->name, ";");
            tidbit new_t;
            new_t.name = parts.size() > 0 ? parts[0] : "";
            new_t.description = parts.size() > 1 ? parts[1] : "";
            new_t.image = parts.size() > 2 ? game.content.bitmaps.list.get(parts[2]) : nullptr;
            category_tidbits.push_back(new_t);
        }
    }
    for(size_t p = 0; p < game.config.pikmin_order.size(); p++) {
        tidbit new_t;
        new_t.name = game.config.pikmin_order[p]->name;
        new_t.description = game.config.pikmin_order[p]->description;
        new_t.image = game.config.pikmin_order[p]->bmp_icon;
        tidbits[HELP_CATEGORY_PIKMIN].push_back(new_t);
    }
    
    //Menu items.
    gui.register_coords("back",        12,  5, 20,  6);
    gui.register_coords("back_input",   3,  7,  4,  4);
    gui.register_coords("gameplay1",   22, 15, 36,  6);
    gui.register_coords("gameplay2",   22, 23, 36,  6);
    gui.register_coords("controls",    22, 31, 36,  6);
    gui.register_coords("pikmin",      22, 39, 36,  6);
    gui.register_coords("objects",     22, 47, 36,  6);
    gui.register_coords("manual",      22, 54, 36,  4);
    gui.register_coords("category",    71,  5, 54,  6);
    gui.register_coords("list",        69, 39, 50, 54);
    gui.register_coords("list_scroll", 96, 39,  2, 54);
    gui.register_coords("image",       16, 83, 28, 30);
    gui.register_coords("tooltip",     65, 83, 66, 30);
    gui.read_coords(gui_file->get_child_by_name("positions"));
    
    //Back button.
    gui.back_item =
        new button_gui_item(
        "Back", game.sys_content.fnt_standard
    );
    gui.back_item->on_activate =
    [this] (const point &) {
        start_closing();
        if(back_callback) back_callback();
    };
    gui.back_item->on_get_tooltip =
    [] () { return "Return to the previous menu."; };
    gui.add_item(gui.back_item, "back");
    
    //Back input icon.
    gui_add_back_input_icon(&gui);
    
    //Gameplay basics button.
    button_gui_item* gameplay1_button =
        new button_gui_item("Gameplay basics", game.sys_content.fnt_standard);
    gameplay1_button->on_activate =
    [this] (const point &) {
        populate_tidbits(HELP_CATEGORY_GAMEPLAY1);
    };
    gameplay1_button->on_get_tooltip =
    [] () {
        return "Show help about basic gameplay features.";
    };
    gui.add_item(gameplay1_button, "gameplay1");
    
    //Gameplay advanced button.
    button_gui_item* gameplay2_button =
        new button_gui_item("Advanced gameplay", game.sys_content.fnt_standard);
    gameplay2_button->on_activate =
    [this] (const point &) {
        populate_tidbits(HELP_CATEGORY_GAMEPLAY2);
    };
    gameplay2_button->on_get_tooltip =
    [] () {
        return "Show advanced gameplay tips.";
    };
    gui.add_item(gameplay2_button, "gameplay2");
    
    //Controls button.
    button_gui_item* controls_button =
        new button_gui_item("Controls", game.sys_content.fnt_standard);
    controls_button->on_activate =
    [this] (const point &) {
        populate_tidbits(HELP_CATEGORY_CONTROLS);
    };
    controls_button->on_get_tooltip =
    [] () {
        return "Show game controls and certain actions you can perform.";
    };
    gui.add_item(controls_button, "controls");
    
    //Pikmin button.
    button_gui_item* pikmin_button =
        new button_gui_item("Pikmin types", game.sys_content.fnt_standard);
    pikmin_button->on_activate =
    [this] (const point &) {
        populate_tidbits(HELP_CATEGORY_PIKMIN);
    };
    pikmin_button->on_get_tooltip =
    [] () {
        return "Show a description of each Pikmin type.";
    };
    gui.add_item(pikmin_button, "pikmin");
    
    //Objects button.
    button_gui_item* objects_button =
        new button_gui_item("Objects", game.sys_content.fnt_standard);
    objects_button->on_activate =
    [this] (const point &) {
        populate_tidbits(HELP_CATEGORY_OBJECTS);
    };
    objects_button->on_get_tooltip =
    [] () {
        return "Show help about some noteworthy objects you'll find.";
    };
    gui.add_item(objects_button, "objects");
    
    //Manual text.
    bullet_gui_item* manual_bullet =
        new bullet_gui_item("More help...", game.sys_content.fnt_standard);
    manual_bullet->on_activate =
    [] (const point &) {
        open_manual("home.html");
    };
    manual_bullet->on_get_tooltip = [] () {
        return
            "Click to open the manual (in the game's folder) for more help.";
    };
    gui.add_item(manual_bullet, "manual");
    
    //Category text.
    category_text = new text_gui_item("Help", game.sys_content.fnt_standard);
    gui.add_item(category_text, "category");
    
    //Tidbit list box.
    tidbit_list = new list_gui_item();
    gui.add_item(tidbit_list, "list");
    
    //Tidbit list scrollbar.
    scroll_gui_item* list_scroll = new scroll_gui_item();
    list_scroll->list_item = tidbit_list;
    gui.add_item(list_scroll, "list_scroll");
    
    //Image item.
    gui_item* image_item = new gui_item();
    image_item->on_draw =
    [this] (const point & center, const point & size) {
        if(cur_tidbit == nullptr) return;
        if(cur_tidbit->image == nullptr) return;
        draw_bitmap_in_box(
            cur_tidbit->image,
            center, size, false
        );
    };
    gui.add_item(image_item, "image");
    
    //Tooltip text.
    text_gui_item* tooltip_text =
        new text_gui_item("", game.sys_content.fnt_standard);
    tooltip_text->on_draw =
        [this]
    (const point & center, const point & size) {
        draw_tidbit(
            game.sys_content.fnt_standard, center, size,
            gui.get_current_tooltip()
        );
    };
    gui.add_item(tooltip_text, "tooltip");
    
    //Finishing touches.
    gui.set_selected_item(gui.back_item, true);
    gui.on_selection_changed =
    [this] () {
        cur_tidbit = nullptr;
    };
}


/**
 * @brief Destroys the help menu object.
 */
help_menu_t::~help_menu_t() {
    for(size_t c = 0; c < N_HELP_CATEGORIES; c++) {
        if(c == HELP_CATEGORY_PIKMIN) continue;
        for(size_t t = 0; t < tidbits[(HELP_CATEGORY) c].size(); t++) {
            if(tidbits[(HELP_CATEGORY) c][t].image) {
                game.content.bitmaps.list.free(tidbits[(HELP_CATEGORY) c][t].image);
            }
        }
    }
    tidbits.clear();
    
    gui.destroy();
}


/**
 * @brief Draws the help menu.
 */
void help_menu_t::draw() {
    gui.draw();
}


/**
 * @brief Draws some help tidbit's text.
 *
 * @param font Font to use.
 * @param where Coordinates to draw the text on.
 * @param max_size Maximum width or height the text can occupy.
 * A value of zero in one of these coordinates makes it not have a
 * limit in that dimension.
 * @param text Text to draw.
 */
void help_menu_t::draw_tidbit(
    const ALLEGRO_FONT* const font, const point &where,
    const point &max_size, const string &text
) {
    //Get the tokens that make up the tidbit.
    vector<string_token> tokens = tokenize_string(text);
    if(tokens.empty()) return;
    
    int line_height = al_get_font_line_height(font);
    
    set_string_token_widths(tokens, font, game.sys_content.fnt_slim, line_height, true);
    
    //Split long lines.
    vector<vector<string_token> > tokens_per_line =
        split_long_string_with_tokens(tokens, max_size.x);
        
    if(tokens_per_line.empty()) return;
    
    //Figure out if we need to scale things vertically.
    //Control bind icons that are bitmaps will have their width unchanged,
    //otherwise this would turn into a cat-and-mouse game of the Y scale
    //shrinking causing a token width to shrink, which could cause the
    //Y scale to grow, ad infinitum.
    float y_scale = 1.0f;
    if(tokens_per_line.size() * line_height > max_size.y) {
        y_scale = max_size.y / (tokens_per_line.size() * (line_height + 4));
    }
    
    //Draw!
    for(size_t l = 0; l < tokens_per_line.size(); l++) {
        draw_string_tokens(
            tokens_per_line[l], game.sys_content.fnt_standard, game.sys_content.fnt_slim,
            true,
            point(
                where.x,
                where.y + l * (line_height + 4) * y_scale -
                (tokens_per_line.size() * line_height * y_scale / 2.0f)
            ),
            ALLEGRO_ALIGN_CENTER, point(max_size.x, line_height * y_scale)
        );
    }
}


/**
 * @brief Handles an Allegro event.
 *
 * @param ev The event.
 */
void help_menu_t::handle_event(const ALLEGRO_EVENT &ev) {
    if(!closing) gui.handle_event(ev);
}

/**
 * @brief Handles a player action.
 *
 * @param action Data about the player action.
 */
void help_menu_t::handle_player_action(const player_action &action) {
    gui.handle_player_action(action);
}


/**
 * @brief Populates the help menu's list of tidbits.
 *
 * @param category Category of tidbits to use.
 */
void help_menu_t::populate_tidbits(const HELP_CATEGORY category) {
    vector<tidbit> &category_tidbits = tidbits[category];
    
    switch(category) {
    case HELP_CATEGORY_GAMEPLAY1: {
        category_text->text = "Gameplay basics";
        break;
    } case HELP_CATEGORY_GAMEPLAY2: {
        category_text->text = "Advanced gameplay";
        break;
    } case HELP_CATEGORY_CONTROLS: {
        category_text->text = "Controls";
        break;
    } case HELP_CATEGORY_PIKMIN: {
        category_text->text = "Pikmin";
        break;
    } case HELP_CATEGORY_OBJECTS: {
        category_text->text = "Objects";
        break;
    } default: {
        category_text->text = "Help";
        break;
    }
    }
    
    tidbit_list->delete_all_children();
    
    for(size_t t = 0; t < category_tidbits.size(); t++) {
        tidbit* t_ptr = &category_tidbits[t];
        bullet_gui_item* tidbit_bullet =
            new bullet_gui_item(
            t_ptr->name,
            game.sys_content.fnt_standard
        );
        tidbit_bullet->center = point(0.50f, 0.045f + t * 0.10f);
        tidbit_bullet->size = point(1.0f, 0.09f);
        tidbit_bullet->on_get_tooltip = [this, t_ptr] () {
            return t_ptr->description;
        };
        tidbit_bullet->on_selected = [this, t_ptr] () {
            cur_tidbit = t_ptr;
        };
        tidbit_bullet->start_juice_animation(
            gui_item::JUICE_TYPE_GROW_TEXT_MEDIUM
        );
        tidbit_list->add_child(tidbit_bullet);
        gui.add_item(tidbit_bullet);
    }
    
    category_text->start_juice_animation(
        gui_item::JUICE_TYPE_GROW_TEXT_HIGH
    );
}


/**
 * @brief Starts the closing process.
 */
void help_menu_t::start_closing() {
    closing = true;
    closing_timer = GAMEPLAY::MENU_EXIT_HUD_MOVE_TIME;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void help_menu_t::tick(float delta_t) {
    //Tick the GUI.
    gui.tick(delta_t);
    
    //Tick the menu closing.
    if(closing) {
        closing_timer -= delta_t;
        if(closing_timer <= 0.0f) {
            to_delete = true;
        }
    }
}
