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

#include "../../game.h"


//Path to the GUI information file.
const string gameplay_state::pause_menu_struct::GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Pause_menu.txt";


/* ----------------------------------------------------------------------------
 * Creates a pause menu struct.
 */
gameplay_state::pause_menu_struct::pause_menu_struct() :
    to_delete(false) {
    
    //Menu items.
    gui.register_coords("continue", 50, 32, 50, 10);
    gui.register_coords("retry",    50, 44, 50, 10);
    gui.register_coords("finish",   50, 56, 50, 10);
    gui.register_coords("quit",     50, 68, 50, 10);
    gui.read_coords(
        data_node(GUI_FILE_PATH).get_child_by_name("positions")
    );
    
    //Continue button.
    gui.back_item =
        new button_gui_item("Continue", game.fonts.standard);
    gui.back_item->on_activate =
    [this] (const point &) {
        to_delete = true;
    };
    gui.add_item(gui.back_item, "continue");
    
    //Retry button.
    button_gui_item* retry_button =
        new button_gui_item("Retry day", game.fonts.standard);
    retry_button->on_activate =
    [this] (const point &) {
        game.states.gameplay->leave(LEAVE_TO_RETRY);
    };
    gui.add_item(retry_button, "retry");
    
    //Finish button.
    button_gui_item* finish_button =
        new button_gui_item("Finish day", game.fonts.standard);
    finish_button->on_activate =
    [this] (const point &) {
        game.states.gameplay->leave(LEAVE_TO_FINISH);
    };
    gui.add_item(finish_button, "finish");
    
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
        game.states.gameplay->leave(LEAVE_TO_AREA_SELECT);
    };
    gui.add_item(quit_button, "quit");
    
    //Finishing touches.
    gui.set_selected_item(gui.back_item);
}


/* ----------------------------------------------------------------------------
 * Destroys a pause menu struct.
 */
gameplay_state::pause_menu_struct::~pause_menu_struct() {
    gui.destroy();
}


/* ----------------------------------------------------------------------------
 * Ticks the pause menu by one frame.
 * time:
 *   How many seconds to tick by.
 */
void gameplay_state::pause_menu_struct::tick(const float delta_t) {
    //Tick the GUI.
    gui.tick(delta_t);
}
