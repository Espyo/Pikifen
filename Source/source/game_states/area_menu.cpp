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
//Path to the GUI information file.
const string GUI_FILE_PATH = GUI_FOLDER_PATH + "/Area_menu.txt";
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
    cur_thumb(nullptr) {
    
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
    if(!areas_to_pick.empty() && prev_selected_item != gui.selected_item) {
    
        size_t area_idx = INVALID;
        
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
                area_subtitles[area_idx];
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
            
        } else {
            name_text->text.clear();
            subtitle_text->text.clear();
            description_text->text.clear();
            difficulty_text->text.clear();
            tags_text->text.clear();
            maker_text->text.clear();
            version_text->text.clear();
            cur_thumb = NULL;
            
        }
        
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
    }
    
    //Resources.
    bmp_menu_bg = load_bmp(game.asset_file_names.main_menu);
    
    //Menu items.
    gui.register_coords("back",          14,    7, 20,  6);
    gui.register_coords("pick_text",     45.5,  7, 39,  6);
    gui.register_coords("mission_specs", 81,    7, 30,  6);
    gui.register_coords("list",          19,   51, 30, 78);
    gui.register_coords("list_scroll",   36,   51,  2, 78);
    gui.register_coords("info_box",      67,   51, 58, 78);
    gui.register_coords("name",          56,   18, 32,  8);
    gui.register_coords("subtitle",      56,   28, 32,  8);
    gui.register_coords("thumbnail",     84,   24, 20, 20);
    gui.register_coords("description",   67,   45, 54, 18);
    gui.register_coords("high_scores",   67,   63, 54, 14);
    gui.register_coords("difficulty",    67,   74, 54,  4);
    gui.register_coords("tags",          67,   80, 54,  4);
    gui.register_coords("maker",         54,   86, 28,  4);
    gui.register_coords("version",       82,   86, 24,  4);
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
        
        //Area info box.
        list_gui_item* info_box = new list_gui_item();
        gui.add_item(info_box, "info_box");
        
        //Name text.
        name_text = new text_gui_item("", game.fonts.area_name);
        gui.add_item(name_text, "name");
        
        //Subtitle text.
        subtitle_text = new text_gui_item("", game.fonts.area_name);
        gui.add_item(subtitle_text, "subtitle");
        
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
        gui.add_item(thumb_item, "thumbnail");
        
        //Description text.
        description_text =
            new text_gui_item(
            "", game.fonts.standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
        );
        description_text->on_draw =
        [this] (const point & center, const point & size) {
            int text_x = center.x;
            switch(description_text->flags) {
            case ALLEGRO_ALIGN_LEFT: {
                text_x = center.x - size.x * 0.5;
                break;
            } case ALLEGRO_ALIGN_RIGHT: {
                text_x = center.x + size.x * 0.5;
                break;
            }
            }
            int text_y = center.y - size.y / 2.0f;
            int line_height = al_get_font_line_height(game.fonts.standard);
            float juicy_grow_amount = description_text->get_juice_value();
            
            vector<string_token> tokens =
                tokenize_string(description_text->text);
            set_string_token_widths(
                tokens, game.fonts.standard, game.fonts.slim, line_height
            );
            vector<vector<string_token> > tokens_per_line =
                split_long_string_with_tokens(tokens, size.x);
                
            for(size_t l = 0; l < tokens_per_line.size(); ++l) {
                draw_string_tokens(
                    tokens_per_line[l], game.fonts.standard, game.fonts.slim,
                    point(
                        text_x,
                        text_y + l * line_height
                    ),
                    description_text->flags,
                    point(size.x, line_height),
                    point(1.0f + juicy_grow_amount, 1.0f + juicy_grow_amount)
                );
            }
        };
        gui.add_item(description_text, "description");
        
        //TODO high scores
        
        //Difficulty text.
        difficulty_text =
            new text_gui_item(
            "", game.fonts.standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
        );
        gui.add_item(difficulty_text, "difficulty");
        
        //Tags text.
        tags_text =
            new text_gui_item(
            "", game.fonts.standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
        );
        gui.add_item(tags_text, "tags");
        
        //Maker text.
        maker_text =
            new text_gui_item(
            "", game.fonts.standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
        );
        gui.add_item(maker_text, "maker");
        
        //Version text.
        version_text =
            new text_gui_item(
            "", game.fonts.standard, COLOR_WHITE, ALLEGRO_ALIGN_RIGHT
        );
        gui.add_item(version_text, "version");
        
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
    game.fade_mgr.start_fade(true, nullptr);
    if(areas_to_pick.size() > 1) {
        gui.set_selected_item(first_area_button);
    }
    
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
    
    cur_thumb = NULL;
    for(size_t a = 0; a < area_thumbs.size(); ++a) {
        if(area_thumbs[a]) {
            al_destroy_bitmap(area_thumbs[a]);
        }
    }
    area_thumbs.clear();
    
}
