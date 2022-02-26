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


//Path to the GUI information file.
const string gameplay_state::pause_menu_struct::GUI_FILE_PATH =
    GUI_FOLDER_PATH + "/Pause_menu.txt";


/* ----------------------------------------------------------------------------
 * Creates a pause menu struct.
 */
gameplay_state::pause_menu_struct::pause_menu_struct() :
    bg_alpha_mult(0.0f),
    closing_timer(0.0f),
    to_delete(false),
    closing(false) {
    
    //Menu items.
    gui.register_coords("header",   50, 15, 50, 10);
    gui.register_coords("continue", 50, 32, 50, 10);
    gui.register_coords("retry",    50, 44, 50, 10);
    gui.register_coords("finish",   50, 56, 50, 10);
    gui.register_coords("quit",     50, 68, 50, 10);
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
        game.states.gameplay->start_leaving(LEAVE_TO_RETRY);
    };
    retry_button->on_get_tooltip =
    [] () { return "Retry this day from the start."; };
    gui.add_item(retry_button, "retry");
    
    //Finish button.
    button_gui_item* finish_button =
        new button_gui_item("Finish day", game.fonts.standard);
    finish_button->on_activate =
    [this] (const point &) {
        game.states.gameplay->start_leaving(LEAVE_TO_FINISH);
    };
    finish_button->on_get_tooltip =
    [] () { return "Finish playing this day."; };
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
        game.states.gameplay->start_leaving(LEAVE_TO_AREA_SELECT);
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
        GUI_MANAGER_ANIM_UP_TO_CENTER, MENU_ENTRY_HUD_MOVE_TIME
    );
}


/* ----------------------------------------------------------------------------
 * Destroys a pause menu struct.
 */
gameplay_state::pause_menu_struct::~pause_menu_struct() {
    gui.destroy();
}


/* ----------------------------------------------------------------------------
 * Handles an Allegro event.
 */
void gameplay_state::pause_menu_struct::handle_event(const ALLEGRO_EVENT &ev) {
    if(!closing) gui.handle_event(ev);
}


/* ----------------------------------------------------------------------------
 * Starts the closing process.
 */
void gameplay_state::pause_menu_struct::start_closing() {
    closing = true;
    closing_timer = MENU_EXIT_HUD_MOVE_TIME;
    gui.start_animation(GUI_MANAGER_ANIM_CENTER_TO_UP, MENU_EXIT_HUD_MOVE_TIME);
    game.states.gameplay->hud->gui.start_animation(
        GUI_MANAGER_ANIM_OUT_TO_IN,
        MENU_EXIT_HUD_MOVE_TIME
    );
}


/* ----------------------------------------------------------------------------
 * Ticks time by one frame of logic.
 * delta_t:
 *   How long the frame's tick is, in seconds.
 */
void gameplay_state::pause_menu_struct::tick(const float delta_t) {
    //Tick the GUI.
    gui.tick(delta_t);
    
    //Tick the background.
    const float bg_alpha_mult_speed = 1.0f / MENU_ENTRY_HUD_MOVE_TIME;
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
