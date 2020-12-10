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


const string area_menu_state::GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Area_menu.txt";


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
    al_clear_to_color(al_map_rgb(0, 0, 0));
    draw_bitmap(
        bmp_menu_bg, point(game.win_w * 0.5, game.win_h * 0.5),
        point(game.win_w, game.win_h), 0, map_gray(64)
    );
    
    gui.draw();
    
    game.fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Ticks one frame's worth of logic.
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
    
    //If there's only one area, go there right away.
    if(areas_to_pick.size() == 1) {
        game.states.gameplay->area_to_load =
            areas_to_pick[0];
        game.change_state(game.states.gameplay);
        return;
    }
    
    for(size_t a = 0; a < areas_to_pick.size(); ++a) {
        string actual_name = areas_to_pick[a];
        data_node data(AREAS_FOLDER_PATH + "/" + actual_name + "/Data.txt");
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
    gui.read_coords(
        data_node(GUI_FILE_PATH).get_child_by_name("positions")
    );
    
    gui.back_item = new gui_item(true);
    gui.back_item->on_draw =
    [this] (const point & center, const point & size) {
        draw_button(
            center, size, "Back", game.fonts.main,
            map_gray(255), gui.back_item->selected
        );
    };
    gui.back_item->on_activate =
    [this] () {
        leave();
    };
    gui.add_item(gui.back_item, "back");
    
    gui_item* pick_text = new gui_item();
    pick_text->on_draw =
    [this] (const point & center, const point & size) {
        draw_compressed_text(
            game.fonts.main, map_gray(255),
            center, ALLEGRO_ALIGN_CENTER, 1, size,
            "Pick an area:"
        );
    };
    gui.add_item(pick_text, "pick_text");
    
    gui_item* list_box = new gui_item();
    list_box->on_draw =
    [this] (const point & center, const point & size) {
        al_draw_rounded_rectangle(
            center.x - size.x * 0.5,
            center.y - size.y * 0.5,
            center.x + size.x * 0.5,
            center.y + size.y * 0.5,
            8.0f, 8.0f, al_map_rgba(255, 255, 255, 128), 1.0f
        );
    };
    list_box->padding = 8.0f;
    gui.add_item(list_box, "list");
    
    gui_item* list_scroll = new gui_item();
    list_scroll->on_draw =
    [this] (const point & center, const point & size) {
        al_draw_rounded_rectangle(
            center.x - size.x * 0.5,
            center.y - size.y * 0.5,
            center.x + size.x * 0.5,
            center.y + size.y * 0.5,
            8.0f, 8.0f, al_map_rgba(255, 255, 255, 128), 1.0f
        );
    };
    gui.add_item(list_scroll, "list_scroll");
    
    gui_item* first_area_button = NULL;
    for(size_t a = 0; a < areas_to_pick.size(); ++a) {
        string area_name = area_names[a];
        string area_folder = areas_to_pick[a];
        
        gui_item* area_button = new gui_item(true);
        area_button->center = point(0.50f, 0.045f + a * 0.10f);
        area_button->size = point(1.0f, 0.09f);
        area_button->parent = list_box;
        area_button->on_draw =
            [this, area_button, area_name] (
                const point & center, const point & size
        ) {
            draw_button(
                center, size, area_name,
                game.fonts.area_name, map_gray(255),
                area_button->selected
            );
        };
        area_button->on_activate =
        [this, area_folder] () {
            game.states.gameplay->area_to_load = area_folder;
            game.fade_mgr.start_fade(false, [] () {
                game.change_state(game.states.gameplay);
            });
        };
        gui.add_item(area_button);
        if(!first_area_button) {
            first_area_button = area_button;
        }
    }
    
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
