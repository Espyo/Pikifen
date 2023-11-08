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
#include "../utils/string_utils.h"


namespace CONTROLS_MENU {
//Height of each bind button.
const float BIND_BUTTON_HEIGHT = 0.07f;
//Padding between each bind button.
const float BIND_BUTTON_PADDING = 0.01f;
//Path to the GUI information file.
const string GUI_FILE_PATH = GUI_FOLDER_PATH + "/Controls_menu.txt";
}


/* ----------------------------------------------------------------------------
 * Creates a "controls menu" state.
 */
controls_menu_state::controls_menu_state() :
    game_state(),
    bmp_menu_bg(NULL),
    list_box(nullptr),
    capturing_input(false),
    showing_more(false),
    cur_action_type(PLAYER_ACTION_NONE),
    cur_bind_idx(0) {
    
}


/* ----------------------------------------------------------------------------
 * Chooses the input for a given action type's bind.
 * If the bind index is greater than the number of existing binds for this
 * action type, then a new one gets added.
 * action_type:
 *   Action type.
 * bind_idx:
 *   Index of that action type's bind.
 */
void controls_menu_state::choose_input(
    const PLAYER_ACTION_TYPES action_type, const size_t bind_idx
) {
    capturing_input = true;
    
    const vector<control_bind> &all_binds = game.controls.binds();
    size_t binds_counted = 0;
    cur_action_type = action_type;
    cur_bind_idx = all_binds.size();
    
    for(size_t b = 0; b < all_binds.size(); ++b) {
        if(all_binds[b].action_type_id != action_type) continue;
        if(binds_counted == bind_idx) {
            cur_bind_idx = b;
            break;
        } else {
            binds_counted++;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Deletes a bind from an action type.
 * action_type:
 *   Action type it belongs to.
 * bind_idx:
 *   Index number of the control.
 */
void controls_menu_state::delete_bind(
    const PLAYER_ACTION_TYPES action_type, const size_t bind_idx
) {
    vector<control_bind> &all_binds = game.controls.binds();
    size_t binds_counted = 0;
    
    for(size_t b = 0; b < all_binds.size(); ++b) {
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


/* ----------------------------------------------------------------------------
 * Draws the controls menu.
 */
void controls_menu_state::do_drawing() {
    al_clear_to_color(COLOR_BLACK);
    
    draw_bitmap(
        bmp_menu_bg, point(game.win_w * 0.5, game.win_h * 0.5),
        point(game.win_w, game.win_h), 0, map_gray(64)
    );
    
    gui.draw();
    
    if(capturing_input) {
        al_draw_filled_rectangle(
            0, 0, game.win_w, game.win_h,
            al_map_rgba(24, 24, 32, 192)
        );
        
        draw_text_lines(
            game.fonts.standard,
            COLOR_WHITE,
            point(game.win_w / 2.0f, game.win_h / 2.0f),
            ALLEGRO_ALIGN_CENTER,
            TEXT_VALIGN_CENTER,
            "Waiting for any input..."
        );
    }
    
    draw_mouse_cursor(GAME::CURSOR_STANDARD_COLOR);
    
    game.fade_mgr.draw();
    
    al_flip_display();
}


/* ----------------------------------------------------------------------------
 * Ticks time by one frame of logic.
 */
void controls_menu_state::do_logic() {
    vector<player_action> player_actions = game.controls.new_frame();
    for(size_t a = 0; a < player_actions.size(); ++a) {
        gui.handle_player_action(player_actions[a]);
    }
    
    gui.tick(game.delta_t);
    
    game.fade_mgr.tick(game.delta_t);
}


/* ----------------------------------------------------------------------------
 * Returns the name of this state.
 */
string controls_menu_state::get_name() const {
    return "controls menu";
}


/* ----------------------------------------------------------------------------
 * Handles Allegro events.
 * ev:
 *   Event to handle.
 */
void controls_menu_state::handle_allegro_event(ALLEGRO_EVENT &ev) {
    if(game.fade_mgr.is_fading()) return;
    
    if(capturing_input) {
    
        player_input input = game.controls.allegro_event_to_input(ev);
        if(input.value >= 0.5f) {
            vector<control_bind> &all_binds = game.controls.binds();
            if(cur_bind_idx >= all_binds.size()) {
                control_bind new_bind;
                new_bind.action_type_id = cur_action_type;
                new_bind.player_nr = cur_player_nr;
                new_bind.input = input;
                all_binds.push_back(new_bind);
            } else {
                game.controls.binds()[cur_bind_idx].input = input;
            }
            capturing_input = false;
            populate_binds();
        }
        
    } else {
    
        gui.handle_event(ev);
        game.controls.handle_allegro_event(ev);
        
    }
    
}


/* ----------------------------------------------------------------------------
 * Leaves the controls menu and goes to the options menu.
 */
void controls_menu_state::leave() {
    game.fade_mgr.start_fade(false, [] () {
        game.change_state(game.states.options_menu);
    });
    save_options();
}


/* ----------------------------------------------------------------------------
 * Loads the controls menu into memory.
 */
void controls_menu_state::load() {
    bmp_menu_bg = NULL;
    capturing_input = false;
    showing_more = false;
    cur_action_type = PLAYER_ACTION_NONE;
    cur_bind_idx = INVALID;
    
    //Resources.
    bmp_menu_bg = load_bmp(game.asset_file_names.main_menu);
    
    //Menu items.
    gui.register_coords("back",        12,  5, 20,  6);
    gui.register_coords("player",        12,  15, 20,  6);
    gui.register_coords("list",        50, 51, 88, 82);
    gui.register_coords("list_scroll", 97, 51,  2, 82);
    gui.register_coords("tooltip",     50, 96, 96,  4);
    gui.read_coords(
        data_node(CONTROLS_MENU::GUI_FILE_PATH).get_child_by_name("positions")
    );
    
    //Back button.
    gui.back_item =
        new button_gui_item("Back", game.fonts.standard);
    gui.back_item->on_activate =
    [this] (const point &) {
        leave();
    };
    gui.back_item->on_get_tooltip =
    [] () { return "Return to the options menu."; };
    gui.add_item(gui.back_item, "back");

    options_menu_picker_gui_item<size_t>* player_picker =
        new options_menu_picker_gui_item<size_t>(
        "Player: ",
        &game.states.controls_menu->cur_player_nr,
        0,
    {0, 1, 2, 3},
    {"1", "2", "3", "4"},
    "Player."
    );
    player_picker->value_to_string = [] (const float v) {
        return i2s(v);
    };
    player_picker->after_change = [this] () {
        this->populate_binds();
    };
    player_picker->init();
    gui.add_item(player_picker, "player");
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
    game.fade_mgr.start_fade(true, nullptr);
    gui.set_selected_item(gui.back_item);
    
    al_reconfigure_joysticks();
}


/* ----------------------------------------------------------------------------
 * Populates the list of binds.
 */
void controls_menu_state::populate_binds() {
    list_box->delete_all_children();
    
    const vector<player_action_type> &all_player_action_types =
        game.controls.get_all_player_action_types();
    vector<control_bind> &all_binds = game.controls.binds();
    
    binds_per_action_type.clear();
    binds_per_action_type.assign(N_PLAYER_ACTIONS, vector<control_bind>());
    
    //Read all binds and sort them by player action type.
    for(size_t b = 0; b < all_binds.size(); ++b) {
        const control_bind &bind = all_binds[b];
        if(bind.player_nr != cur_player_nr) continue;
        binds_per_action_type[bind.action_type_id].push_back(bind);
    }
    
    PLAYER_ACTION_CATEGORIES last_cat = PLAYER_ACTION_CAT_NONE;
    
    for(size_t a = 0; a < N_PLAYER_ACTIONS; ++a) {
        const player_action_type &action_type = all_player_action_types[a];
        
        if(action_type.internal_name.empty()) continue;
        
        float action_y =
            list_box->get_child_bottom() + CONTROLS_MENU::BIND_BUTTON_PADDING;
            
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
                new text_gui_item(section_name, game.fonts.area_name);
            section_text->center =
                point(
                    0.50f,
                    action_y + CONTROLS_MENU::BIND_BUTTON_HEIGHT / 2.0f
                );
            section_text->size =
                point(0.50f, CONTROLS_MENU::BIND_BUTTON_HEIGHT);
            list_box->add_child(section_text);
            gui.add_item(section_text);
            
            action_y =
                list_box->get_child_bottom() +
                CONTROLS_MENU::BIND_BUTTON_PADDING;
                
            last_cat = action_type.category;
            
        }
        
        float cur_y = action_y + CONTROLS_MENU::BIND_BUTTON_HEIGHT / 2.0f;
        
        //Action type name bullet.
        bullet_point_gui_item* name_bullet =
            new bullet_point_gui_item(action_type.name, game.fonts.standard);
        name_bullet->center = point(0.25f, cur_y);
        name_bullet->size = point(0.40f, CONTROLS_MENU::BIND_BUTTON_HEIGHT);
        name_bullet->on_get_tooltip =
        [action_type] () { return action_type.description; };
        list_box->add_child(name_bullet);
        gui.add_item(name_bullet);
        
        //More button.
        button_gui_item* more_button =
            new button_gui_item("...", game.fonts.standard);
        more_button->on_activate =
        [this, a] (const point &) {
            if(showing_more && a == cur_action_type) {
                showing_more = false;
            } else {
                cur_action_type = (PLAYER_ACTION_TYPES) a;
                showing_more = true;
            }
            populate_binds();
        };
        more_button->center = point(0.92f, cur_y);
        more_button->size = point(0.05f, CONTROLS_MENU::BIND_BUTTON_HEIGHT);
        string tooltip =
            (showing_more && a == cur_action_type) ?
            "Hide options." :
            "Show information and options for this action.";
        more_button->on_get_tooltip =
        [tooltip] () { return tooltip; };
        list_box->add_child(more_button);
        gui.add_item(more_button);
        if(a == cur_action_type) {
            gui.set_selected_item(more_button);
        }
        
        if(showing_more && a == cur_action_type) {
        
            //Default label.
            text_gui_item* default_label_text =
                new text_gui_item(
                "Default:", game.fonts.standard, COLOR_WHITE, ALLEGRO_ALIGN_LEFT
            );
            default_label_text->center =
                point(0.70f, cur_y);
            default_label_text->size =
                point(0.30f, CONTROLS_MENU::BIND_BUTTON_HEIGHT);
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
                point(0.75f, cur_y);
            default_icon->size =
                point(0.15f, CONTROLS_MENU::BIND_BUTTON_HEIGHT);
            default_icon->on_draw =
            [def_input] (const point & center, const point & size) {
                draw_player_input_icon(
                    game.fonts.slim, def_input, false, center, size
                );
            };
            list_box->add_child(default_icon);
            gui.add_item(default_icon);
            
            cur_y +=
                CONTROLS_MENU::BIND_BUTTON_HEIGHT +
                CONTROLS_MENU::BIND_BUTTON_PADDING;
                
            //Restore default button.
            button_gui_item* restore_button =
                new button_gui_item("Restore defaults", game.fonts.standard);
            restore_button->center =
                point(0.70f, cur_y);
            restore_button->size =
                point(0.30f, CONTROLS_MENU::BIND_BUTTON_HEIGHT);
            restore_button->on_activate =
            [this, a] (const point &) {
                restore_defaults((PLAYER_ACTION_TYPES) a);
            };
            restore_button->on_get_tooltip =
            [] () { return "Restore this action's default controls."; };
            list_box->add_child(restore_button);
            gui.add_item(restore_button);
            restore_button->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_MEDIUM
            );
            
            cur_y +=
                CONTROLS_MENU::BIND_BUTTON_HEIGHT +
                CONTROLS_MENU::BIND_BUTTON_PADDING;
                
        }
        
        vector<control_bind> a_binds = binds_per_action_type[a];
        for(size_t b = 0; b < a_binds.size(); ++b) {
        
            //Change/remove bind button.
            button_gui_item* bind_button =
                new button_gui_item("", game.fonts.standard);
            bind_button->on_activate =
            [this, a, b] (const point &) {
                if(showing_more && a == cur_action_type) {
                    delete_bind((PLAYER_ACTION_TYPES) a, b);
                } else {
                    choose_input((PLAYER_ACTION_TYPES) a, b);
                }
            };
            bind_button->on_draw =
                [this, a, b, a_binds, bind_button]
            (const point & center, const point & size) {
                point icon_center =
                    (showing_more && a == cur_action_type) ?
                    point(center.x + size.x * 0.25f, center.y) :
                    center;
                point icon_size =
                    (showing_more && a == cur_action_type) ?
                    point(size.x / 2.0f, size.y) :
                    size;
                    
                if(showing_more && a == cur_action_type) {
                    float juice = bind_button->get_juice_value();
                    draw_compressed_scaled_text(
                        game.fonts.standard, COLOR_WHITE,
                        point(center.x - size.x / 2.0f + 16.0f, center.y),
                        point(1.0 + juice, 1.0 + juice),
                        ALLEGRO_ALIGN_LEFT, TEXT_VALIGN_CENTER, size, true,
                        "Remove"
                    );
                }
                
                draw_player_input_icon(
                    game.fonts.slim, a_binds[b].input, false,
                    icon_center, icon_size * 0.8f
                );
                
                draw_button(
                    center, size,
                    "", game.fonts.standard, COLOR_WHITE,
                    bind_button->selected,
                    bind_button->get_juice_value()
                );
            };
            bind_button->center = point(0.70f, cur_y);
            bind_button->size = point(0.30f, CONTROLS_MENU::BIND_BUTTON_HEIGHT);
            string bind_button_tooltip =
                (showing_more && a == cur_action_type) ?
                "Remove this control from this action." :
                "Change the control for this action.";
            bind_button->on_get_tooltip =
            [bind_button_tooltip] () { return bind_button_tooltip; };
            list_box->add_child(bind_button);
            gui.add_item(bind_button);
            
            if(a == cur_action_type) {
                bind_button->start_juice_animation(
                    gui_item::JUICE_TYPE_GROW_TEXT_MEDIUM
                );
            }
            
            cur_y +=
                CONTROLS_MENU::BIND_BUTTON_HEIGHT +
                CONTROLS_MENU::BIND_BUTTON_PADDING;
                
        }
        
        if(showing_more && a == cur_action_type) {
        
            //Add button.
            button_gui_item* add_button =
                new button_gui_item("Add control", game.fonts.standard);
            add_button->center = point(0.70f, cur_y);
            add_button->size = point(0.30f, CONTROLS_MENU::BIND_BUTTON_HEIGHT);
            add_button->on_activate =
            [this, a, a_binds] (const point &) {
                choose_input((PLAYER_ACTION_TYPES) a, a_binds.size());
            };
            add_button->on_get_tooltip =
            [] () { return "Adds another control to this action."; };
            list_box->add_child(add_button);
            gui.add_item(add_button);
            add_button->start_juice_animation(
                gui_item::JUICE_TYPE_GROW_TEXT_MEDIUM
            );
            
            cur_y +=
                CONTROLS_MENU::BIND_BUTTON_HEIGHT +
                CONTROLS_MENU::BIND_BUTTON_PADDING;
                
        } else if(a_binds.empty()) {
        
            //Add first bind button.
            button_gui_item* bind_button =
                new button_gui_item("", game.fonts.standard);
            bind_button->on_activate =
            [this, a] (const point &) {
                choose_input((PLAYER_ACTION_TYPES) a, 0);
            };
            bind_button->on_draw =
                [this, a, bind_button]
            (const point & center, const point & size) {
                draw_button(
                    center, size, "", game.fonts.standard, COLOR_WHITE,
                    bind_button->selected,
                    bind_button->get_juice_value()
                );
            };
            bind_button->center = point(0.70f, cur_y);
            bind_button->size = point(0.30f, CONTROLS_MENU::BIND_BUTTON_HEIGHT);
            bind_button->on_get_tooltip =
            [] () { return "Choose an input for this action."; };
            list_box->add_child(bind_button);
            gui.add_item(bind_button);
            
        }
        
        if(a < N_PLAYER_ACTIONS - 1) {
            //Spacer line.
            gui_item* line = new gui_item();
            line->center =
                point(
                    0.50f, list_box->get_child_bottom() + 0.01f
                );
            line->size = point(0.90f, 0.01f);
            line->on_draw =
            [] (const point & center, const point & size) {
                al_draw_line(
                    center.x - size.x / 2.0f,
                    center.y,
                    center.x + size.x / 2.0f,
                    center.y,
                    al_map_rgba(255, 255, 255, 128),
                    1.0f
                );
            };
            list_box->add_child(line);
            gui.add_item(line);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Restores the default binds for a given player action.
 * action_type_id:
 *   Action type ID of the action to restore.
 */
void controls_menu_state::restore_defaults(
    const PLAYER_ACTION_TYPES action_type_id
) {
    const player_action_type &action_type =
        game.controls.get_all_player_action_types()[action_type_id];
    vector<control_bind> &all_binds =
        game.controls.binds();
        
    for(size_t b = 0; b < all_binds.size();) {
        if(
            all_binds[b].player_nr == cur_player_nr &&
            all_binds[b].action_type_id == action_type_id
        ) {
            all_binds.erase(all_binds.begin() + b);
        } else {
            ++b;
        }
    }
    
    player_input def_input =
        game.controls.str_to_input(action_type.default_bind_str);
    control_bind new_bind;
    
    if(def_input.type != INPUT_TYPE_NONE) {
        new_bind.action_type_id = action_type_id;
        new_bind.player_nr = cur_player_nr;
        new_bind.input = def_input;
        all_binds.push_back(new_bind);
    }
    
    showing_more = false;
    populate_binds();
}


/* ----------------------------------------------------------------------------
 * Unloads the controls menu from memory.
 */
void controls_menu_state::unload() {

    //Resources.
    al_destroy_bitmap(bmp_menu_bg);
    
    //Menu items.
    gui.destroy();
    
}
