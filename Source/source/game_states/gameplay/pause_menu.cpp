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
        help_category_text->text = "Gameplay";
    };
    gameplay_button->on_get_tooltip =
    [] () { return "Show help about gameplay features."; };
    help_gui.add_item(gameplay_button, "gameplay");
    
    //Controls button.
    button_gui_item* controls_button =
        new button_gui_item("Controls", game.fonts.standard);
    controls_button->on_activate =
    [this] (const point &) {
        help_category_text->text = "Controls";
    };
    controls_button->on_get_tooltip =
    [] () { return "Show help about game controls."; };
    help_gui.add_item(controls_button, "controls");
    
    //Pikmin button.
    button_gui_item* pikmin_button =
        new button_gui_item("Pikmin", game.fonts.standard);
    pikmin_button->on_activate =
    [this] (const point &) {
        help_category_text->text = "Pikmin";
    };
    pikmin_button->on_get_tooltip =
    [] () { return "Show help about the different Pikmin types."; };
    help_gui.add_item(pikmin_button, "pikmin");
    
    //Objects button.
    button_gui_item* objects_button =
        new button_gui_item("Objects", game.fonts.standard);
    objects_button->on_activate =
    [this] (const point &) {
        help_category_text->text = "Objects";
    };
    objects_button->on_get_tooltip =
    [] () { return "Show help about some objects you'll find."; };
    help_gui.add_item(objects_button, "objects");
    
    //Category text.
    help_category_text = new text_gui_item("", game.fonts.standard);
    help_gui.add_item(help_category_text, "category");
    
    //Tidbit list box.
    list_gui_item* list_box = new list_gui_item();
    help_gui.add_item(list_box, "list");
    
    //Tidbit list scrollbar.
    scroll_gui_item* list_scroll = new scroll_gui_item();
    list_scroll->list_item = list_box;
    list_box->scroll_item = list_scroll;
    help_gui.add_item(list_scroll, "list_scroll");
    
    //Tooltip text.
    text_gui_item* tooltip_text =
        new text_gui_item("", game.fonts.standard);
    tooltip_text->on_draw =
        [this]
    (const point & center, const point & size) {
        draw_compressed_scaled_text(
            game.fonts.standard, COLOR_WHITE,
            center, point(1.0f, 1.0f), ALLEGRO_ALIGN_CENTER, 1, size,
            false,
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
    [] () { return "Some quick help about how gameplay works."; };
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
