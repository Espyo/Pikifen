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


/* ----------------------------------------------------------------------------
 * Creates an "area menu" state.
 */
area_menu_state::area_menu_state() :
    game_state(),
    bmp_menu_bg(NULL),
    time_spent(0),
    cur_page_nr(0),
    cur_page_nr_widget(NULL) {
    
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
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->draw(time_spent);
    }
    
    game.fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Ticks one frame's worth of logic.
 */
void area_menu_state::do_logic() {
    game.fade_mgr.tick(game.delta_t);
    time_spent += game.delta_t;
    
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        menu_widgets[w]->tick(game.delta_t);
    }
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
    
    handle_widget_events(ev);
    
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
    selected_widget = NULL;
    bmp_menu_bg = NULL;
    time_spent = 0;
    cur_page_nr = 0;
    
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
    
    //Menu widgets.
    back_widget =
        new menu_button(
        point(game.win_w * 0.15, game.win_h * 0.10),
        point(game.win_w * 0.20, game.win_h * 0.06),
    [this] () {
        leave();
    },
    "Back", game.fonts.main
    );
    menu_widgets.push_back(back_widget);
    
    menu_widgets.push_back(
        new menu_text(
            point(game.win_w * 0.5, game.win_h * 0.1),
            point(game.win_w * 0.3, game.win_h * 0.1),
            "Pick an area:",
            game.fonts.main, al_map_rgb(255, 255, 255), ALLEGRO_ALIGN_CENTER
        )
    );
    
    for(size_t a = 0; a < 8; ++a) {
        menu_widgets.push_back(
            new menu_button(
                point(game.win_w * 0.5, game.win_h * (0.2 + 0.08 * a)),
                point(game.win_w * 0.8, game.win_h * 0.06),
        [] () {
        
        },
        "", game.fonts.area_name
            )
        );
        area_buttons.push_back(menu_widgets.back());
    }
    
    menu_widgets.push_back(
        new menu_text(
            point(game.win_w * 0.15, game.win_h * 0.9),
            point(game.win_w * 0.2, game.win_h * 0.1),
            "Page:", game.fonts.main
        )
    );
    menu_widgets.push_back(
        new menu_button(
            point(game.win_w * 0.3, game.win_h * 0.9),
            point(game.win_w * 0.15, game.win_h * 0.1),
    [this] () {
        cur_page_nr =
            sum_and_wrap(
                cur_page_nr, -1, ceil(areas_to_pick.size() / 8.0)
            );
        cur_page_nr_widget->start_juicy_grow();
        update();
    },
    "<", game.fonts.main
        )
    );
    cur_page_nr_widget =
        new menu_text(
        point(game.win_w * 0.4, game.win_h * 0.9),
        point(game.win_w * 0.1, game.win_h * 0.1),
        "", game.fonts.main
    );
    menu_widgets.push_back(cur_page_nr_widget);
    menu_widgets.push_back(
        new menu_button(
            point(game.win_w * 0.5, game.win_h * 0.9),
            point(game.win_w * 0.15, game.win_h * 0.1),
    [this] () {
        cur_page_nr =
            sum_and_wrap(
                cur_page_nr, 1, ceil(areas_to_pick.size() / 8.0)
            );
        cur_page_nr_widget->start_juicy_grow();
        update();
    },
    ">", game.fonts.main
        )
    );
    
    //Finishing touches.
    game.fade_mgr.start_fade(true, nullptr);
    update();
    if(menu_widgets.size() >= 3) {
        set_selected_widget(menu_widgets[2]);
    } else {
        set_selected_widget(menu_widgets[1]);
    }
    
}


/* ----------------------------------------------------------------------------
 * Unloads the area menu from memory.
 */
void area_menu_state::unload() {

    //Resources.
    al_destroy_bitmap(bmp_menu_bg);
    
    //Menu widgets.
    set_selected_widget(NULL);
    for(size_t w = 0; w < menu_widgets.size(); w++) {
        delete menu_widgets[w];
    }
    menu_widgets.clear();
    area_buttons.clear();
    areas_to_pick.clear();
    area_names.clear();
    cur_page_nr_widget = NULL;
    
}


/* ----------------------------------------------------------------------------
 * Updates the contents of the area menu.
 */
void area_menu_state::update() {
    cur_page_nr =
        std::min(cur_page_nr, (size_t) (ceil(areas_to_pick.size() / 8.0) - 1));
    cur_page_nr_widget->text = i2s(cur_page_nr + 1);
    
    for(size_t aw = 0; aw < area_buttons.size(); ++aw) {
        area_buttons[aw]->enabled = false;
    }
    
    size_t area_nr = cur_page_nr * 8;
    size_t list_nr = 0;
    for(; list_nr < 8 && area_nr < areas_to_pick.size(); ++area_nr, ++list_nr) {
        string area_name = area_names[area_nr];
        string area_folder = areas_to_pick[area_nr];
        
        ((menu_button*) area_buttons[list_nr])->click_handler =
        [area_name, area_folder] () {
            game.states.gameplay->area_to_load = area_folder;
            game.fade_mgr.start_fade(false, [] () {
                game.change_state(game.states.gameplay);
            });
        };
        ((menu_button*) area_buttons[list_nr])->text = area_name;
        area_buttons[list_nr]->enabled = true;
        
    }
}
