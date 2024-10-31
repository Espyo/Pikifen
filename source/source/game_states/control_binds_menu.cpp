/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Controls menu state class and controls menu state-related functions.
 */

#include <algorithm>

#include "menus.h"

#include "../drawing.h"
#include "../functions.h"
#include "../game.h"
#include "../load.h"
#include "../utils/allegro_utils.h"
#include "../utils/string_utils.h"


namespace CONTROL_BINDS_MENU {

//Height of each bind button.
const float BIND_BUTTON_HEIGHT = 0.07f;

//Padding between each bind button.
const float BIND_BUTTON_PADDING = 0.01f;

//Timeout before the input capturing cancels.
const float CAPTURE_TIMEOUT_DURATION = 5.0f;

//Name of the GUI information file.
const string GUI_FILE_NAME = "control_binds_menu.txt";

//Name of the song to play in this state.
const string SONG_NAME = "menus";

}


/**
 * @brief Chooses the input for a given action type's bind.
 * If the bind index is greater than the number of existing binds for this
 * action type, then a new one gets added.
 *
 * @param action_type Action type.
 * @param bind_idx Index of that action type's bind.
 */
void control_binds_menu_state::choose_input(
    const PLAYER_ACTION_TYPE action_type, size_t bind_idx
) {
    capturing_input = 1;
    capturing_input_timeout = CONTROL_BINDS_MENU::CAPTURE_TIMEOUT_DURATION;
    
    const vector<control_bind> &all_binds = game.controls.binds();
    size_t binds_counted = 0;
    cur_action_type = action_type;
    cur_bind_idx = all_binds.size();
    
    for(size_t b = 0; b < all_binds.size(); b++) {
        if(all_binds[b].action_type_id != action_type) continue;
        if(binds_counted == bind_idx) {
            cur_bind_idx = b;
            break;
        } else {
            binds_counted++;
        }
    }
}


/**
 * @brief Deletes a bind from an action type.
 *
 * @param action_type Action type it belongs to.
 * @param bind_idx Index number of the control.
 */
void control_binds_menu_state::delete_bind(
    const PLAYER_ACTION_TYPE action_type, size_t bind_idx
) {
    vector<control_bind> &all_binds = game.controls.binds();
    size_t binds_counted = 0;
    
    for(size_t b = 0; b < all_binds.size(); b++) {
        if(all_binds[b].action_type_id != action_type) continue;
        if(binds_counted == bind_idx) {
            all_binds.erase(all_binds.begin() + b);
            break;
        } else {
            binds_counted++;
        }
    }
    
    populate_binds();
}


/**
 * @brief Draws the controls menu.
 */
void control_binds_menu_state::do_drawing() {
    al_clear_to_color(COLOR_BLACK);
    
    draw_bitmap(
        bmp_menu_bg, point(game.win_w * 0.5, game.win_h * 0.5),
        point(game.win_w, game.win_h), 0, map_gray(64)
    );
    
    gui.draw();
    
    if(capturing_input == 1) {
        al_draw_filled_rectangle(
            0, 0, game.win_w, game.win_h,
            al_map_rgba(24, 24, 32, 192)
        );
        
        draw_text_lines(
            "Please perform the new input for \n" +
            game.controls.get_player_action_type(cur_action_type).name + "\n"
            "\n"
            "(Or wait " + i2s(capturing_input_timeout + 1) + "s to cancel...)",
            game.sys_assets.fnt_standard,
            point(game.win_w / 2.0f, game.win_h / 2.0f),
            point(LARGE_FLOAT, LARGE_FLOAT),
            COLOR_WHITE, ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER,
            TEXT_SETTING_FLAG_CANT_GROW
        );
    }
    
    draw_mouse_cursor(GAME::CURSOR_STANDARD_COLOR);
}


/**
 * @brief Ticks time by one frame of logic.
 */
void control_binds_menu_state::do_logic() {
    vector<player_action> player_actions = game.controls.new_frame();
    
    if(capturing_input == 0) {
        for(size_t a = 0; a < player_actions.size(); a++) {
            gui.handle_player_action(player_actions[a]);
        }
    }
    
    gui.tick(game.delta_t);
    
    if(capturing_input == 1) {
        capturing_input_timeout -= game.delta_t;
        if(capturing_input_timeout <= 0.0f) {
            //Timed out. Cancel.
            capturing_input = 0;
        }
    } else if(capturing_input == 2) {
        //A frame has passed in the post-capture cooldown. Finish the cooldown.
        capturing_input = 0;
    }
    
    game.fade_mgr.tick(game.delta_t);
}


/**
 * @brief Returns the name of this state.
 *
 * @return The name.
 */
string control_binds_menu_state::get_name() const {
    return "controls menu";
}


/**
 * @brief Handles Allegro events.
 *
 * @param ev Event to handle.
 */
void control_binds_menu_state::handle_allegro_event(ALLEGRO_EVENT &ev) {
    if(game.fade_mgr.is_fading()) return;
    
    switch(capturing_input) {
    case 0: {
        //Not capturing.
        gui.handle_event(ev);
        
        break;
    } case 1: {
        //Actively capturing.
        player_input input = game.controls.allegro_event_to_input(ev);
        if(input.value >= 0.5f) {
            vector<control_bind> &all_binds = game.controls.binds();
            if(cur_bind_idx >= all_binds.size()) {
                control_bind new_bind;
                new_bind.action_type_id = cur_action_type;
                new_bind.player_nr = 0;
                new_bind.input = input;
                all_binds.push_back(new_bind);
            } else {
                game.controls.binds()[cur_bind_idx].input = input;
            }
            capturing_input = 2;
            populate_binds();
        }
        break;
    } case 2: {
        //One frame of cooldown, so that we don't accidentally feed the
        //input meant for the capture to the GUI.
        break;
    }
    }
    
}


/**
 * @brief Leaves the controls menu and goes to the options menu.
 */
void control_binds_menu_state::leave() {
    game.fade_mgr.start_fade(false, [] () {
        game.states.options_menu->page_to_load = OPTIONS_MENU_PAGE_CONTROLS;
        game.change_state(game.states.options_menu);
    });
    save_options();
}


/**
 * @brief Loads the controls menu into memory.
 */
void control_binds_menu_state::load() {
    bmp_menu_bg = nullptr;
    capturing_input = 0;
    capturing_input_timeout = 0.0f;
    showing_more = false;
    cur_action_type = PLAYER_ACTION_TYPE_NONE;
    cur_bind_idx = INVALID;
    
    //Resources.
    bmp_menu_bg = game.content.bitmaps.list.get(game.asset_file_names.bmp_main_menu);
    
    //Menu items.
    gui.register_coords("back",        12,  5, 20,  6);
    gui.register_coords("header",      50,  5, 50,  6);
    gui.register_coords("list",        50, 51, 88, 82);
    gui.register_coords("list_scroll", 97, 51,  2, 82);
    gui.register_coords("tooltip",     50, 96, 96,  4);
    gui.read_coords(
        game.content.gui.list[CONTROL_BINDS_MENU::GUI_FILE_NAME].
        get_child_by_name("positions")
    );
    
    //Back button.
    gui.back_item =
        new button_gui_item("Back", game.sys_assets.fnt_standard);
    gui.back_item->on_activate =
    [this] (const point &) {
        leave();
    };
    gui.back_item->on_get_tooltip =
    [] () { return "Return to the options menu."; };
    gui.add_item(gui.back_item, "back");
    
    //Header text.
    text_gui_item* header_text =
        new text_gui_item(
        "CONTROL BINDS",
        game.sys_assets.fnt_area_name, COLOR_TRANSPARENT_WHITE, ALLEGRO_ALIGN_CENTER
    );
    gui.add_item(header_text, "header");
    
    //Controls list box.
    list_box = new list_gui_item();
    gui.add_item(list_box, "list");
    
    //Controls list scrollbar.
    scroll_gui_item* list_scroll = new scroll_gui_item();
    list_scroll->list_item = list_box;
    gui.add_item(list_scroll, "list_scroll");
    
    //Tooltip text.
    tooltip_gui_item* tooltip_text =
        new tooltip_gui_item(&gui);
    gui.add_item(tooltip_text, "tooltip");
    
    //Populate the list of binds.
    populate_binds();
    
    //Finishing touches.
    game.audio.set_current_song(CONTROL_BINDS_MENU::SONG_NAME);
    game.fade_mgr.start_fade(true, nullptr);
    gui.set_selected_item(gui.back_item, true);
    
    al_reconfigure_joysticks();
}


/**
 * @brief Populates the list of binds.
 */
void control_binds_menu_state::populate_binds() {
    list_box->delete_all_children();
    
    const vector<player_action_type> &all_player_action_types =
        game.controls.get_all_player_action_types();
    vector<control_bind> &all_binds = game.controls.binds();
    
    binds_per_action_type.clear();
    binds_per_action_type.assign(all_player_action_types.size(), vector<control_bind>());
    
    //Read all binds and sort them by player action type.
    for(size_t b = 0; b < all_binds.size(); b++) {
        const control_bind &bind = all_binds[b];
        if(bind.player_nr != 0) continue;
        binds_per_action_type[bind.action_type_id].push_back(bind);
    }
    
    PLAYER_ACTION_CAT last_cat = PLAYER_ACTION_CAT_NONE;
    
    for(size_t a = 0; a < all_player_action_types.size(); a++) {
        const player_action_type &action_type = all_player_action_types[a];
        
        if(action_type.internal_name.empty()) continue;
        
        float action_y =
            list_box->get_child_bottom() +
            CONTROL_BINDS_MENU::BIND_BUTTON_PADDING;
            
        if(action_type.category != last_cat) {
        
            //Section header text.
            string section_name;
            switch(action_type.category) {
            case PLAYER_ACTION_CAT_NONE: {
                break;
            } case PLAYER_ACTION_CAT_MAIN: {
                section_name = "Main";
                break;
            } case PLAYER_ACTION_CAT_MENUS: {
                section_name = "Menus";
                break;
            } case PLAYER_ACTION_CAT_ADVANCED: {
                section_name = "Advanced";
                break;
            }
            }
            text_gui_item* section_text =
                new text_gui_item(section_name, game.sys_assets.fnt_area_name);
            section_text->center =
                point(
                    0.50f,
                    action_y + CONTROL_BINDS_MENU::BIND_BUTTON_HEIGHT / 2.0f
                );
            section_text->size =
                point(0.50f, CONTROL_BINDS_MENU::BIND_BUTTON_HEIGHT);
            list_box->add_child(section_text);
            gui.add_item(section_text);
            
            action_y =
                list_box->get_child_bottom() +
                CONTROL_BINDS_MENU::BIND_BUTTON_PADDING;
                
            last_cat = action_type.category;
            
        }
        
        float cur_y = action_y + CONTROL_BINDS_MENU::BIND_BUTTON_HEIGHT / 2.0f;
        
        //Action type name bullet.
        bullet_point_gui_item* name_bullet =
            new bullet_point_gui_item(action_type.name, game.sys_assets.fnt_standard);
        name_bullet->center =
            point(0.22f, cur_y);
        name_bullet->size =
            point(0.34f, CONTROL_BINDS_MENU::BIND_BUTTON_HEIGHT);
        name_bullet->on_get_tooltip =
        [action_type] () { return action_type.description; };
        list_box->add_child(name_bullet);
        gui.add_item(name_bullet);
        
        //More button.
        button_gui_item* more_button =
            new button_gui_item("...", game.sys_assets.fnt_standard);
        more_button->on_activate =
        [this, action_type] (const point &) {
            if(showing_more && action_type.id == cur_action_type) {
                showing_more = false;
            } else {
                cur_action_type = action_type.id;
                showing_more = true;
            }
            populate_binds();
        };
        more_button->center =
            point(0.92f, cur_y);
        more_button->size =
            point(0.05f, CONTROL_BINDS_MENU::BIND_BUTTON_HEIGHT);
        string tooltip =
            (showing_more && action_type.id == cur_action_type) ?
            "Hide options." :
            "Show information and options for this action.";
        more_button->on_get_tooltip =
        [tooltip] () { return tooltip; };
        list_box->add_child(more_button);
        gui.add_item(more_button);
        if(action_type.id == cur_action_type) {
            gui.set_selected_item(more_button, true);
        }
        
        vector<control_bind> a_binds = binds_per_action_type[action_type.id];
        for(size_t b = 0; b < a_binds.size(); b++) {
        
            //Change bind button.
            button_gui_item* bind_button =
                new button_gui_item("", game.sys_assets.fnt_standard);
            bind_button->on_activate =
            [this, action_type, b] (const point &) {
                choose_input(action_type.id, b);
            };
            bind_button->on_draw =
                [this, b, a_binds, bind_button]
            (const point & center, const point & size) {
                draw_player_input_icon(
                    game.sys_assets.fnt_slim, a_binds[b].input, false,
                    center, size * 0.8f
                );
                
                draw_button(
                    center, size,
                    "", game.sys_assets.fnt_standard, COLOR_WHITE,
                    bind_button->selected,
                    bind_button->get_juice_value()
                );
            };
            bind_button->center =
                point(0.63f, cur_y);
            bind_button->size =
                point(0.34f, CONTROL_BINDS_MENU::BIND_BUTTON_HEIGHT);
            bind_button->on_get_tooltip =
            [] () { return "Change the input for this action."; };
            list_box->add_child(bind_button);
            gui.add_item(bind_button);
            
            if(showing_more && action_type.id == cur_action_type) {
                //Remove bind button.
                button_gui_item* remove_bind_button =
                    new button_gui_item("", game.sys_assets.fnt_standard);
                remove_bind_button->on_activate =
                [this, action_type, b] (const point &) {
                    delete_bind(action_type.id, b);
                };
                remove_bind_button->on_draw =
                    [this, remove_bind_button]
                (const point & center, const point & size) {
                    draw_button(
                        center, size, "X", game.sys_assets.fnt_standard, COLOR_WHITE,
                        remove_bind_button->selected,
                        remove_bind_button->get_juice_value()
                    );
                };
                remove_bind_button->center =
                    point(0.85f, cur_y);
                remove_bind_button->size =
                    point(0.05f, CONTROL_BINDS_MENU::BIND_BUTTON_HEIGHT);
                remove_bind_button->on_get_tooltip =
                [] () { return "Remove this input from this action."; };
                list_box->add_child(remove_bind_button);
                gui.add_item(remove_bind_button);
                remove_bind_button->start_juice_animation(
                    gui_item::JUICE_TYPE_GROW_TEXT_HIGH
                );
            }
            
            if(action_type.id == cur_action_type) {
                bind_button->start_juice_animation(
                    gui_item::JUICE_TYPE_GROW_TEXT_MEDIUM
                );
            }
            
            cur_y +=
                CONTROL_BINDS_MENU::BIND_BUTTON_HEIGHT +
                CONTROL_BINDS_MENU::BIND_BUTTON_PADDING;
                
        }
        
        if(a_binds.empty()) {
        
            //Add first bind button.
            button_gui_item* bind_button =
                new button_gui_item("", game.sys_assets.fnt_standard);
            bind_button->on_activate =
            [this, action_type] (const point &) {
                choose_input(action_type.id, 0);
            };
            bind_button->on_draw =
                [this, bind_button]
            (const point & center, const point & size) {
                draw_button(
                    center, size, "", game.sys_assets.fnt_standard, COLOR_WHITE,
                    bind_button->selected,
                    bind_button->get_juice_value()
                );
            };
            bind_button->center =
                point(0.63f, cur_y);
            bind_button->size =
                point(0.34f, CONTROL_BINDS_MENU::BIND_BUTTON_HEIGHT);
            bind_button->on_get_tooltip =
            [] () { return "Choose an input for this action."; };
            list_box->add_child(bind_button);
            gui.add_item(bind_button);
            bind_button->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_MEDIUM
            );
            
            cur_y +=
                CONTROL_BINDS_MENU::BIND_BUTTON_HEIGHT +
                CONTROL_BINDS_MENU::BIND_BUTTON_PADDING;
                
        } else if(showing_more && action_type.id == cur_action_type) {
        
            //Add button.
            button_gui_item* add_button =
                new button_gui_item("Add...", game.sys_assets.fnt_standard);
            add_button->center =
                point(0.63f, cur_y);
            add_button->size =
                point(0.34f, CONTROL_BINDS_MENU::BIND_BUTTON_HEIGHT);
            add_button->on_activate =
            [this, action_type, a_binds] (const point &) {
                choose_input(action_type.id, a_binds.size());
            };
            add_button->on_get_tooltip =
            [] () { return "Add another input to this action."; };
            list_box->add_child(add_button);
            gui.add_item(add_button);
            add_button->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_HIGH
            );
            
            cur_y +=
                CONTROL_BINDS_MENU::BIND_BUTTON_HEIGHT +
                CONTROL_BINDS_MENU::BIND_BUTTON_PADDING;
                
        }
        
        if(showing_more && action_type.id == cur_action_type) {
        
            //Restore default button.
            button_gui_item* restore_button =
                new button_gui_item("Restore defaults", game.sys_assets.fnt_standard);
            restore_button->center =
                point(0.63f, cur_y);
            restore_button->size =
                point(0.34f, CONTROL_BINDS_MENU::BIND_BUTTON_HEIGHT);
            restore_button->on_activate =
            [this, action_type] (const point &) {
                restore_defaults(action_type.id);
            };
            restore_button->on_get_tooltip =
            [] () { return "Restore this action's default inputs."; };
            list_box->add_child(restore_button);
            gui.add_item(restore_button);
            restore_button->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_MEDIUM
            );
            
            cur_y +=
                CONTROL_BINDS_MENU::BIND_BUTTON_HEIGHT +
                CONTROL_BINDS_MENU::BIND_BUTTON_PADDING;
                
            //Default label.
            text_gui_item* default_label_text =
                new text_gui_item(
                "Default:", game.sys_assets.fnt_standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
            );
            default_label_text->center =
                point(0.63f, cur_y);
            default_label_text->size =
                point(0.30f, CONTROL_BINDS_MENU::BIND_BUTTON_HEIGHT);
            list_box->add_child(default_label_text);
            gui.add_item(default_label_text);
            default_label_text->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_MEDIUM
            );
            
            //Default icon.
            player_input def_input =
                game.controls.str_to_input(action_type.default_bind_str);
            gui_item* default_icon = new gui_item();
            default_icon->center =
                point(0.68f, cur_y);
            default_icon->size =
                point(0.17f, CONTROL_BINDS_MENU::BIND_BUTTON_HEIGHT);
            default_icon->on_draw =
            [def_input] (const point & center, const point & size) {
                draw_player_input_icon(
                    game.sys_assets.fnt_slim, def_input, false, center, size
                );
            };
            list_box->add_child(default_icon);
            gui.add_item(default_icon);
            
        }
        
        if(a < all_player_action_types.size() - 1) {
            //Spacer line.
            gui_item* line = new gui_item();
            line->center =
                point(
                    0.50f, list_box->get_child_bottom() + 0.02f
                );
            line->size = point(0.90f, 0.02f);
            line->on_draw =
            [] (const point & center, const point & size) {
                al_draw_line(
                    center.x - size.x / 2.0f,
                    center.y,
                    center.x + size.x / 2.0f,
                    center.y,
                    COLOR_TRANSPARENT_WHITE,
                    1.0f
                );
            };
            list_box->add_child(line);
            gui.add_item(line);
        }
    }
}


/**
 * @brief Restores the default binds for a given player action.
 *
 * @param action_type_id Action type ID of the action to restore.
 */
void control_binds_menu_state::restore_defaults(
    const PLAYER_ACTION_TYPE action_type_id
) {
    const player_action_type &action_type =
        game.controls.get_player_action_type(action_type_id);
    vector<control_bind> &all_binds =
        game.controls.binds();
        
    for(size_t b = 0; b < all_binds.size();) {
        if(
            all_binds[b].player_nr == 0 &&
            all_binds[b].action_type_id == action_type_id
        ) {
            all_binds.erase(all_binds.begin() + b);
        } else {
            b++;
        }
    }
    
    player_input def_input =
        game.controls.str_to_input(action_type.default_bind_str);
    control_bind new_bind;
    
    if(def_input.type != INPUT_TYPE_NONE) {
        new_bind.action_type_id = action_type_id;
        new_bind.player_nr = 0;
        new_bind.input = def_input;
        all_binds.push_back(new_bind);
    }
    
    showing_more = false;
    populate_binds();
}


/**
 * @brief Unloads the controls menu from memory.
 */
void control_binds_menu_state::unload() {

    //Resources.
    game.content.bitmaps.list.free(bmp_menu_bg);
    bmp_menu_bg = nullptr;
    
    //Menu items.
    gui.destroy();
    
}
