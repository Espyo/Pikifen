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
    bmp_menu_bg(NULL) {
    
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
    game.fade_mgr.tick(game.delta_t);
    
    gui.tick(game.delta_t);
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
        game.change_state(game.states.main_menu);
    });
}


/* ----------------------------------------------------------------------------
 * Loads the area menu into memory.
 */
void area_menu_state::load() {
    bmp_menu_bg = NULL;
    
    //Areas.
    areas_to_pick = folder_to_vector(AREAS_FOLDER_PATH, true);
    
    for(size_t a = 0; a < areas_to_pick.size(); ++a) {
        string actual_name = areas_to_pick[a];
        data_node data(
            AREAS_FOLDER_PATH + "/" + actual_name + "/Data.txt"
        );
        if(data.file_was_opened) {
            string s = data.get_child_by_name("name")->value;
            if(!s.empty()) {
                actual_name = s;
            }
        }
        
        area_names.push_back(actual_name);
    }
    
    //Resources.
    bmp_menu_bg = load_bmp(game.asset_file_names.main_menu);
    
    //Menu items.
    gui.register_coords("back",        15, 10, 20,  6);
    gui.register_coords("pick_text",   50, 10, 30, 10);
    gui.register_coords("list",        49, 55, 77, 70);
    gui.register_coords("list_scroll", 90, 55,  2, 70);
    gui.register_coords("tooltip",     50, 95, 95,  8);
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
        new text_gui_item("Pick an area:", game.fonts.standard);
    gui.add_item(pick_text, "pick_text");
    
    //Area list box.
    list_gui_item* list_box = new list_gui_item();
    gui.add_item(list_box, "list");
    
    //Area list scrollbar.
    scroll_gui_item* list_scroll = new scroll_gui_item();
    list_scroll->list_item = list_box;
    gui.add_item(list_scroll, "list_scroll");
    
    //Items for the various areas.
    button_gui_item* first_area_button = NULL;
    for(size_t a = 0; a < areas_to_pick.size(); ++a) {
        string area_name = area_names[a];
        string area_folder = areas_to_pick[a];
        
        //Area button.
        button_gui_item* area_button =
            new button_gui_item(area_name, game.fonts.area_name);
        area_button->center = point(0.50f, 0.045f + a * 0.10f);
        area_button->size = point(1.0f, 0.09f);
        area_button->on_activate =
        [this, area_folder] (const point &) {
            game.states.gameplay->area_to_load = area_folder;
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
    
}
