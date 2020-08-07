/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Editor-related functions.
 */

#include "editor.h"

#include "../drawing.h"
#include "../game.h"
#include "../functions.h"
#include "../imgui/imgui_impl_allegro5.h"
#include "../imgui/imgui_stdlib.h"
#include "../load.h"
#include "../mob_categories/mob_category.h"
#include "../mob_types/mob_type.h"
#include "../utils/imgui_utils.h"
#include "../utils/string_utils.h"


//Every icon in the icon bitmap file is these many pixels from the previous.
const int editor::EDITOR_ICON_BMP_PADDING = 1;
//Every icon in the icon bitmap file has this size.
const int editor::EDITOR_ICON_BMP_SIZE = 24;
//Time until the next click is no longer considered a double-click.
const float editor::DOUBLE_CLICK_TIMEOUT = 0.5f;
//How much to zoom in/out with the keyboard keys.
const float editor::KEYBOARD_CAM_ZOOM = 0.25f;
//How long the unsaved changes warning stays on-screen for.
const float editor::UNSAVED_CHANGES_WARNING_DURATION = 3.0f;
//Height of the unsaved changes warning, sans spike.
const int editor::UNSAVED_CHANGES_WARNING_HEIGHT = 30;
//Width and height of the unsaved changes warning's spike.
const int editor::UNSAVED_CHANGES_WARNING_SPIKE_SIZE = 16;
//Width of the unsaved changes warning, sans spike.
const int editor::UNSAVED_CHANGES_WARNING_WIDTH = 150;


/* ----------------------------------------------------------------------------
 * Creates a new dialog info.
 */
editor::dialog_info::dialog_info() :
    close_callback(nullptr),
    process_callback(nullptr),
    is_open(true) {
    
}


/* ----------------------------------------------------------------------------
 * Processes the dialog for this frame.
 */
void editor::dialog_info::process() {
    if(!is_open) return;
    
    ImGui::SetNextWindowPos(
        ImVec2(game.win_w / 2.0f, game.win_h / 2.0f),
        ImGuiCond_Always, ImVec2(0.5f, 0.5f)
    );
    ImGui::SetNextWindowSize(
        ImVec2(game.win_w * 0.8, game.win_h * 0.8), ImGuiCond_Once
    );
    ImGui::OpenPopup((title + "##dialog").c_str());
    
    if(
        ImGui::BeginPopupModal(
            (title + "##dialog").c_str(), &is_open
        )
    ) {
    
        process_callback();
        
        ImGui::EndPopup();
    }
}


/* ----------------------------------------------------------------------------
 * Initializes editor class stuff.
 */
editor::editor() :
    canvas_separator_x(-1),
    double_click_time(0),
    is_ctrl_pressed(false),
    is_m1_pressed(false),
    is_m2_pressed(false),
    is_m3_pressed(false),
    is_shift_pressed(false),
    last_mouse_click(INVALID),
    loaded_content_yet(false),
    made_new_changes(false),
    mouse_drag_confirmed(false),
    unsaved_changes_warning_timer(UNSAVED_CHANGES_WARNING_DURATION),
    special_input_focus_controller(0),
    state(0),
    sub_state(0),
    zoom_max_level(0),
    zoom_min_level(0) {
    
    editor_icons.reserve(N_EDITOR_ICONS);
    for(size_t i = 0; i < N_EDITOR_ICONS; ++i) {
        editor_icons.push_back(NULL);
    }
}


/* ----------------------------------------------------------------------------
 * Centers the camera so that these four points are in view.
 * A bit of padding is added, so that, for instance, the top-left
 * point isn't exactly on the top-left of the screen,
 * where it's hard to see.
 */
void editor::center_camera(
    const point &min_coords, const point &max_coords
) {
    point min_c = min_coords;
    point max_c = max_coords;
    if(min_c == max_c) {
        min_c = min_c - 2.0;
        max_c = max_c + 2.0;
    }
    
    float width = max_c.x - min_c.x;
    float height = max_c.y - min_c.y;
    
    game.cam.target_pos.x = floor(min_c.x + width  / 2);
    game.cam.target_pos.y = floor(min_c.y + height / 2);
    
    float z;
    if(width > height) z = (canvas_br.x - canvas_tl.x) / width;
    else z = (canvas_br.y - canvas_tl.y) / height;
    z -= z * 0.1;
    
    game.cam.target_zoom = z;
    update_transformations();
}


/* ----------------------------------------------------------------------------
 * Checks if there are any unsaved changes that have not been notified yet.
 * Returns true if there are, and also sets up the unsaved changes warning.
 * Returns false if everything is okay to continue.
 */
bool editor::check_new_unsaved_changes(const point &pos) {
    unsaved_changes_warning_timer.stop();
    
    if(!made_new_changes) return false;
    made_new_changes = false;
    
    if(pos.x == 0 && pos.y == 0) {
        unsaved_changes_warning_pos = get_last_widget_pos();
    } else {
        unsaved_changes_warning_pos = pos;
    }
    unsaved_changes_warning_timer.start();
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Closes the topmost dialog.
 */
void editor::close_top_dialog() {
    if(dialogs.empty()) return;
    dialogs.back()->is_open = false;
}


/* ----------------------------------------------------------------------------
 * Handles the logic part of the main loop of the area editor. This is meant to
 * be run after the editor's own logic code.
 */
void editor::do_logic_post() {
    game.fade_mgr.tick(game.delta_t);
}


/* ----------------------------------------------------------------------------
 * Handles the logic part of the main loop of the area editor. This is meant to
 * be run before the editor's own logic code.
 */
void editor::do_logic_pre() {
    if(double_click_time > 0) {
        double_click_time -= game.delta_t;
        if(double_click_time < 0) double_click_time = 0;
    }
    
    game.cam.tick(game.delta_t);
    
    unsaved_changes_warning_timer.tick(game.delta_t);
    
    update_transformations();
}


/* ----------------------------------------------------------------------------
 * Draws the grid, using the current game camera.
 * interval:
 *   Interval between grid lines.
 * major_color:
 *   Color to use for major lines. These are lines that happen at major
 *   milestones (i.e. twice the interval).
 * minor_color:
 *   Color to use for minor lines. These are lines that aren't major.
 */
void editor::draw_grid(
    const float interval,
    const ALLEGRO_COLOR &major_color, const ALLEGRO_COLOR &minor_color
) {
    point cam_top_left_corner(0, 0);
    point cam_bottom_right_corner(canvas_br.x, canvas_br.y);
    al_transform_coordinates(
        &game.screen_to_world_transform,
        &cam_top_left_corner.x, &cam_top_left_corner.y
    );
    al_transform_coordinates(
        &game.screen_to_world_transform,
        &cam_bottom_right_corner.x, &cam_bottom_right_corner.y
    );
    
    float x = floor(cam_top_left_corner.x / interval) * interval;
    while(x < cam_bottom_right_corner.x + interval) {
        ALLEGRO_COLOR c = minor_color;
        bool draw_line = true;
        
        if(fmod(x, interval * 2) == 0) {
            c = major_color;
            if((interval * 2) * game.cam.zoom <= 6) {
                draw_line = false;
            }
        } else {
            if(interval * game.cam.zoom <= 6) {
                draw_line = false;
            }
        }
        
        if(draw_line) {
            al_draw_line(
                x, cam_top_left_corner.y,
                x, cam_bottom_right_corner.y + interval,
                c, 1.0f / game.cam.zoom
            );
        }
        x += interval;
    }
    
    float y = floor(cam_top_left_corner.y / interval) * interval;
    while(y < cam_bottom_right_corner.y + interval) {
        ALLEGRO_COLOR c = minor_color;
        bool draw_line = true;
        
        if(fmod(y, interval * 2) == 0) {
            c = major_color;
            if((interval * 2) * game.cam.zoom <= 6) {
                draw_line = false;
            }
        } else {
            if(interval * game.cam.zoom <= 6) {
                draw_line = false;
            }
        }
        
        if(draw_line) {
            al_draw_line(
                cam_top_left_corner.x, y,
                cam_bottom_right_corner.x + interval, y,
                c, 1.0f / game.cam.zoom
            );
        }
        y += interval;
    }
}


/* ----------------------------------------------------------------------------
 * Draws the unsaved changes warning, if it is visible.
 */
void editor::draw_unsaved_changes_warning() {
    float r = unsaved_changes_warning_timer.get_ratio_left();
    if(r == 0) return;
    
    ALLEGRO_COLOR back_color = al_map_rgba(192, 192, 64, r * 255);
    ALLEGRO_COLOR outline_color = al_map_rgba(80, 80, 16, r * 255);
    ALLEGRO_COLOR text_color = al_map_rgba(0, 0, 0, r * 255);
    bool spike_up = unsaved_changes_warning_pos.y < game.win_h / 2.0;
    
    point box_center = unsaved_changes_warning_pos;
    if(unsaved_changes_warning_pos.x < UNSAVED_CHANGES_WARNING_WIDTH / 2.0) {
        box_center.x +=
            UNSAVED_CHANGES_WARNING_WIDTH / 2.0 - unsaved_changes_warning_pos.x;
    } else if(
        unsaved_changes_warning_pos.x >
        game.win_w - UNSAVED_CHANGES_WARNING_WIDTH / 2.0
    ) {
        box_center.x -=
            unsaved_changes_warning_pos.x -
            (game.win_w - UNSAVED_CHANGES_WARNING_WIDTH / 2.0);
    }
    if(spike_up) {
        box_center.y += UNSAVED_CHANGES_WARNING_HEIGHT / 2.0;
        box_center.y += UNSAVED_CHANGES_WARNING_SPIKE_SIZE;
    } else {
        box_center.y -= UNSAVED_CHANGES_WARNING_HEIGHT / 2.0;
        box_center.y -= UNSAVED_CHANGES_WARNING_SPIKE_SIZE;
    }
    
    point box_tl(
        box_center.x - UNSAVED_CHANGES_WARNING_WIDTH / 2.0,
        box_center.y - UNSAVED_CHANGES_WARNING_HEIGHT / 2.0
    );
    point box_br(
        box_center.x + UNSAVED_CHANGES_WARNING_WIDTH / 2.0,
        box_center.y + UNSAVED_CHANGES_WARNING_HEIGHT / 2.0
    );
    point spike_p1(
        unsaved_changes_warning_pos.x,
        unsaved_changes_warning_pos.y
    );
    point spike_p2(
        unsaved_changes_warning_pos.x -
        UNSAVED_CHANGES_WARNING_SPIKE_SIZE / 2.0,
        unsaved_changes_warning_pos.y +
        UNSAVED_CHANGES_WARNING_SPIKE_SIZE * (spike_up ? 1 : -1)
    );
    point spike_p3(
        unsaved_changes_warning_pos.x +
        UNSAVED_CHANGES_WARNING_SPIKE_SIZE / 2.0,
        unsaved_changes_warning_pos.y +
        UNSAVED_CHANGES_WARNING_SPIKE_SIZE * (spike_up ? 1 : -1)
    );
    
    al_draw_filled_rectangle(
        box_tl.x, box_tl.y, box_br.x, box_br.y, back_color
    );
    al_draw_filled_triangle(
        spike_p1.x, spike_p1.y, spike_p2.x, spike_p2.y, spike_p3.x, spike_p3.y,
        back_color
    );
    al_draw_rectangle(
        box_tl.x, box_tl.y, box_br.x, box_br.y, outline_color, 2
    );
    al_draw_line(
        spike_p2.x, spike_p2.y, spike_p3.x, spike_p3.y,
        back_color, 2
    );
    al_draw_line(
        spike_p1.x, spike_p1.y, spike_p2.x, spike_p2.y,
        outline_color, 2
    );
    al_draw_line(
        spike_p1.x, spike_p1.y, spike_p3.x, spike_p3.y,
        outline_color, 2
    );
    draw_text_lines(
        game.fonts.builtin, text_color,
        box_center, ALLEGRO_ALIGN_CENTER, 1,
        "You have\nunsaved changes!"
    );
}


/* ----------------------------------------------------------------------------
 * Sets up logic to focus on the next input widget, if it is one of the
 * input widgets with logic to focus on it.
 */
void editor::focus_next_special_input() {
    special_input_focus_controller = 2;
}


/* ----------------------------------------------------------------------------
 * Returns the position of the last widget, in screen coordinates.
 */
point editor::get_last_widget_pos() {
    return
        point(
            ImGui::GetItemRectMin().x + ImGui::GetItemRectSize().x / 2.0,
            ImGui::GetItemRectMin().y + ImGui::GetItemRectSize().y / 2.0
        );
}


/* ----------------------------------------------------------------------------
 * Handles an Allegro event for control-related things.
 */
void editor::handle_allegro_event(ALLEGRO_EVENT &ev) {
    if(game.fade_mgr.is_fading()) return;
    
    ImGui_ImplAllegro5_ProcessEvent(&ev);
    
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_WARPED ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN ||
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        handle_mouse_update(ev);
    }
    
    if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
        !is_mouse_in_gui
    ) {
    
        switch (ev.mouse.button) {
        case 1: {
            is_m1_pressed = true;
            break;
        } case 2: {
            is_m2_pressed = true;
            break;
        } case 3: {
            is_m3_pressed = true;
            break;
        }
        }
        
        mouse_drag_start = point(ev.mouse.x, ev.mouse.y);
        mouse_drag_confirmed = false;
        
        if(ev.mouse.button == 1) {
            is_gui_focused = false;
        }
        
        if(ev.mouse.button == last_mouse_click && double_click_time > 0) {
            switch(ev.mouse.button) {
            case 1: {
        
                handle_lmb_double_click(ev);
                break;
            } case 2: {
                handle_rmb_double_click(ev);
                break;
            } case 3: {
                handle_mmb_double_click(ev);
                break;
            }
            }
            
            double_click_time = 0;
            
        } else {
            switch(ev.mouse.button) {
            case 1: {
                handle_lmb_down(ev);
                break;
            } case 2: {
                handle_rmb_down(ev);
                break;
            } case 3: {
                handle_mmb_down(ev);
                break;
            }
            }
            
            last_mouse_click = ev.mouse.button;
            double_click_time = DOUBLE_CLICK_TIMEOUT;
        }
        
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN &&
        is_mouse_in_gui
    ) {
        is_gui_focused = true;
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP
    ) {
        switch(ev.mouse.button) {
        case 1: {
            is_m1_pressed = false;
            handle_lmb_up(ev);
            break;
        } case 2: {
            is_m2_pressed = false;
            handle_rmb_up(ev);
            break;
        } case 3: {
            is_m3_pressed = false;
            handle_mmb_up(ev);
            break;
        }
        }
        
    } else if(
        ev.type == ALLEGRO_EVENT_MOUSE_AXES ||
        ev.type == ALLEGRO_EVENT_MOUSE_WARPED
    ) {
        if(
            fabs(ev.mouse.x - mouse_drag_start.x) >=
            game.options.editor_mouse_drag_threshold ||
            fabs(ev.mouse.y - mouse_drag_start.y) >=
            game.options.editor_mouse_drag_threshold
        ) {
            mouse_drag_confirmed = true;
        }
        
        if(mouse_drag_confirmed) {
            if(is_m1_pressed) {
                handle_lmb_drag(ev);
            }
            if(is_m2_pressed) {
                handle_rmb_drag(ev);
            }
            if(is_m3_pressed) {
                handle_mmb_drag(ev);
            }
        }
        if(
            (ev.mouse.dz != 0 || ev.mouse.dw != 0) &&
            !is_mouse_in_gui
        ) {
            handle_mouse_wheel(ev);
        }
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_LSHIFT ||
            ev.keyboard.keycode == ALLEGRO_KEY_RSHIFT
        ) {
            is_shift_pressed = true;
            
        } else if(
            ev.keyboard.keycode == ALLEGRO_KEY_LCTRL ||
            ev.keyboard.keycode == ALLEGRO_KEY_RCTRL ||
            ev.keyboard.keycode == ALLEGRO_KEY_COMMAND
        ) {
            is_ctrl_pressed = true;
            
        }
        
        handle_key_down_anywhere(ev);
        if(!is_gui_focused) {
            handle_key_down_canvas(ev);
        }
        
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE &&
            !dialogs.empty()
        ) {
            close_top_dialog();
        }
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_UP) {
        if(
            ev.keyboard.keycode == ALLEGRO_KEY_LSHIFT ||
            ev.keyboard.keycode == ALLEGRO_KEY_RSHIFT
        ) {
            is_shift_pressed = false;
            
        } else if(
            ev.keyboard.keycode == ALLEGRO_KEY_LCTRL ||
            ev.keyboard.keycode == ALLEGRO_KEY_RCTRL ||
            ev.keyboard.keycode == ALLEGRO_KEY_COMMAND
        ) {
            is_ctrl_pressed = false;
            
        }
        
        handle_key_up_anywhere(ev);
        if(!is_gui_focused) {
            handle_key_up_canvas(ev);
        }
        
    } else if(ev.type == ALLEGRO_EVENT_KEY_CHAR) {
        handle_key_char_anywhere(ev);
        if(!is_gui_focused) {
            handle_key_char_canvas(ev);
        }
        
    }
}


//Input handler functions.
void editor::handle_key_char_anywhere(const ALLEGRO_EVENT &ev) {}
void editor::handle_key_char_canvas(const ALLEGRO_EVENT &ev) {}
void editor::handle_key_down_anywhere(const ALLEGRO_EVENT &ev) {}
void editor::handle_key_down_canvas(const ALLEGRO_EVENT &ev) {}
void editor::handle_key_up_anywhere(const ALLEGRO_EVENT &ev) {}
void editor::handle_key_up_canvas(const ALLEGRO_EVENT &ev) {}
void editor::handle_lmb_double_click(const ALLEGRO_EVENT &ev) {}
void editor::handle_lmb_down(const ALLEGRO_EVENT &ev) {}
void editor::handle_lmb_drag(const ALLEGRO_EVENT &ev) {}
void editor::handle_lmb_up(const ALLEGRO_EVENT &ev) {}
void editor::handle_mmb_double_click(const ALLEGRO_EVENT &ev) {}
void editor::handle_mmb_down(const ALLEGRO_EVENT &ev) {}
void editor::handle_mmb_drag(const ALLEGRO_EVENT &ev) {}
void editor::handle_mmb_up(const ALLEGRO_EVENT &ev) {}
void editor::handle_mouse_update(const ALLEGRO_EVENT &ev) {}
void editor::handle_mouse_wheel(const ALLEGRO_EVENT &ev) {}
void editor::handle_rmb_double_click(const ALLEGRO_EVENT &ev) {}
void editor::handle_rmb_down(const ALLEGRO_EVENT &ev) {}
void editor::handle_rmb_drag(const ALLEGRO_EVENT &ev) {}
void editor::handle_rmb_up(const ALLEGRO_EVENT &ev) {}


/* ----------------------------------------------------------------------------
 * Displays a popup, if applicable, and fills it with a text input for the
 * user to type something in.
 * Returns true if the user pressed Return or the Ok button.
 * label:
 *   Name of the popup.
 * prompt:
 *   What to prompt to the user. e.g.: "New name:"
 * text:
 *   Pointer to the starting text, as well as the user's final text.
 */
bool editor::input_popup(
    const char* label, const char* prompt, string* text
) {
    bool ret = false;
    if(ImGui::BeginPopup(label)) {
        ImGui::Text("%s", prompt);
        next_input_needs_special_focus();
        if(
            ImGui::InputText(
                "##inputPopupText", text,
                ImGuiInputTextFlags_EnterReturnsTrue |
                ImGuiInputTextFlags_AutoSelectAll
            )
        ) {
            ret = true;
            ImGui::CloseCurrentPopup();
        }
        if(ImGui::Button("Ok")) {
            ret = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if(ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    return ret;
}


/* ----------------------------------------------------------------------------
 * Returns whether or not the pressed key corresponds to the specified
 * key combination. Used for keyboard shortcuts.
 */
bool editor::key_check(
    const int pressed_key, const int match_key,
    const bool needs_ctrl, const int needs_shift
) {

    if(pressed_key != match_key) {
        return false;
    }
    if(needs_ctrl != is_ctrl_pressed) {
        return false;
    }
    if(needs_shift != is_shift_pressed) {
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Displays a popup, if applicable, and fills it with selectable items
 * from a list. Returns true if one of the items was clicked on,
 * false otherwise.
 * label:
 *   Name of the popup.
 * items:
 *   List of items.
 * picked_item:
 *   If an item was picked, set this to its name.
 */
bool editor::list_popup(
    const char* label, const vector<string> &items, string* picked_item
) {
    bool ret = false;
    if(ImGui::BeginPopup(label)) {
        for(size_t i = 0; i < items.size(); ++i) {
            string name = items[i];
            if(ImGui::Selectable(name.c_str())) {
                *picked_item = name;
                ret = true;
            }
        }
        ImGui::EndPopup();
    }
    return ret;
}


/* ----------------------------------------------------------------------------
 * Exits out of the editor, with a fade.
 */
void editor::leave() {
    //Save the user's preferred tree node open states.
    save_options();
    
    game.fade_mgr.start_fade(false, [] () {
        if(game.states.area_editor_st->quick_play_area.empty()) {
            game.change_state(game.states.main_menu_st);
        } else {
            game.states.gameplay_st->area_to_load =
                game.states.area_editor_st->quick_play_area;
            game.change_state(game.states.gameplay_st);
        }
    });
}


/* ----------------------------------------------------------------------------
 * Loads content common for all editors.
 */
void editor::load() {
    bmp_editor_icons =
        load_bmp(game.asset_file_names.editor_icons, NULL, true, false);
    if(bmp_editor_icons) {
        for(size_t i = 0; i < N_EDITOR_ICONS; ++i) {
            editor_icons[i] =
                al_create_sub_bitmap(
                    bmp_editor_icons,
                    EDITOR_ICON_BMP_SIZE * i + EDITOR_ICON_BMP_PADDING * i,
                    0, EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE
                );
        }
    }
    
    game.fade_mgr.start_fade(true, nullptr);
    
    //Load the user's preferred tree node open states.
    load_options();
}


/* ----------------------------------------------------------------------------
 * The next input widget requires special focus logic.
 * It will only be focused on if focus_next_special_input() was called.
 */
void editor::next_input_needs_special_focus() {
    //In order to focus on the InputText when the input popup is
    //opened, we need to call SetKeyboardFocusHere two frames in
    //a row. I'm not quite sure why, but it's likely the same as
    //this: https://github.com/ocornut/imgui/issues/343
    if(special_input_focus_controller > 0) {
        ImGui::SetKeyboardFocusHere();
        special_input_focus_controller--;
    }
}


/* ----------------------------------------------------------------------------
 * Opens a dialog.
 * title:
 *   Title of the dialog window. This is normally a request to the user,
 *   like "Pick an area.".
 * process_callback:
 *   A function to call when it's time to process the contents inside
 *   the dialog.
 */
void editor::open_dialog(
    const string &title,
    const std::function<void()> &process_callback
) {
    dialog_info* new_dialog = new dialog_info();
    
    new_dialog->title = title;
    new_dialog->process_callback = process_callback;
    
    dialogs.push_back(new_dialog);
}


/* ----------------------------------------------------------------------------
 * Opens a picker dialog with the given content.
 * title:
 *   Title of the picker's dialog window. This is normally
 *   a request to the user, like "Pick an area.".
 * items:
 *   List of items to populate the picker with.
 * pick_callback:
 *   A function to call when the user clicks an item or enters a new one.
 *   This function's first argument is the name of the item.
 *   Its second argument is the category of the item, or an empty string.
 *   Its third argument is whether it's a new item or not.
 * list_header:
 *   If not-empty, display this text above the list.
 * can_make_new:
 *   If true, the user can create a new element, by writing its
 *   name on the textbox, and pressing the "+" button.
 * filter:
 *   Filter of names. Only items that match this will appear.
 */
void editor::open_picker(
    const string &title,
    const vector<picker_item> &items,
    const std::function <void(
        const string &, const string &, const bool
    )> &pick_callback,
    const string &list_header,
    const bool can_make_new,
    const string &filter
) {
    picker_info* new_picker = new picker_info(this);
    
    new_picker->title = title;
    new_picker->process_callback =
        std::bind(&editor::picker_info::process, new_picker);
        
    new_picker->items = items;
    new_picker->list_header = list_header;
    new_picker->pick_callback = pick_callback;
    new_picker->can_make_new = can_make_new;
    new_picker->filter = filter;
    
    focus_next_special_input();
    
    dialogs.push_back(new_picker);
}


/* ----------------------------------------------------------------------------
 * Creates widgets with the goal of placing a disabled text widget to the
 * right side of the panel.
 * title: Title to write.
 * width: Width to reserve for it.
 */
void editor::panel_title(const char* title, const float width) {
    //Spacer dummy widget.
    ImGui::SameLine();
    float size =
        game.win_w - canvas_separator_x - ImGui::GetItemRectSize().x -
        width;
    ImGui::Dummy(ImVec2(size, 0));
    
    //Text widget.
    ImGui::SameLine();
    ImGui::TextDisabled("%s", title);
}


/* ----------------------------------------------------------------------------
 * Processes all currently open dialogs for this frame.
 */
void editor::process_dialogs() {
    //Delete closed ones.
    for(size_t d = 0; d < dialogs.size();) {
        dialog_info* d_ptr = dialogs[d];
        
        if(!d_ptr->is_open) {
            if(d_ptr->close_callback) {
                d_ptr->close_callback();
            }
            delete d_ptr;
            dialogs.erase(dialogs.begin() + d);
        } else {
            ++d;
        }
    }
    
    //Process the latest one.
    if(!dialogs.empty()) {
        dialogs.back()->process();
    }
}


/* ----------------------------------------------------------------------------
 * Processes the category and type widgets that allow a user to select a mob
 * type.
 * cat:
 *   Pointer to the category reflected in the combo box.
 * typ:
 *   Pointer to the type reflected in the combo box.
 * only_show_area_editor_types:
 *   If true, object types that cannot appear in the area editor will not
 *   be included.
 * category_change_callback:
 *   If not NULL, this is called as soon as the category combobox is changed.
 * type_change_callback:
 *   If not NULL, this is called as soon as the type combobox is changed.
 */
void editor::process_mob_type_widgets(
    mob_category** cat, mob_type** typ,
    const bool only_show_area_editor_types,
    const std::function<void()> &category_change_callback,
    const std::function<void()> &type_change_callback
) {
    //Column setup.
    ImGui::Columns(2, NULL, false);
    ImGui::SetColumnWidth(-1, 51.0f);
    
    //Search button.
    if(
        ImGui::ImageButton(
            editor_icons[ICON_SEARCH],
            ImVec2(EDITOR_ICON_BMP_SIZE, EDITOR_ICON_BMP_SIZE),
            ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f),
            9.0f
        )
    ) {
        vector<picker_item> items;
        for(unsigned char c = 0; c < N_MOB_CATEGORIES; ++c) {
            if(c == MOB_CATEGORY_NONE) continue;
            
            vector<string> names;
            mob_category* c_ptr = game.mob_categories.get(c);
            c_ptr->get_type_names(names);
            string cat_name = game.mob_categories.get(c)->plural_name;
            
            for(size_t n = 0; n < names.size(); ++n) {
                if(
                    only_show_area_editor_types &&
                    !c_ptr->get_type(names[n])->appears_in_area_editor
                ) {
                    continue;
                }
                items.push_back(picker_item(names[n], cat_name));
            }
        }
        open_picker(
            "Pick an object type", items,
            [cat, typ, category_change_callback, type_change_callback]
        (const string & n, const string & c, const bool) {
            if(type_change_callback) {
                type_change_callback();
            }
            (*cat) = game.mob_categories.get_from_pname(c);
            (*typ) = (*cat)->get_type(n);
        },
        "", false
        );
    }
    set_tooltip(
        "Search for an object type from the entire list."
    );
    
    ImGui::NextColumn();
    
    //Object category combobox.
    if(!(*cat)) {
        *cat = game.mob_categories.get(MOB_CATEGORY_NONE);
    }
    
    vector<string> categories;
    for(size_t c = 0; c < N_MOB_CATEGORIES; ++c) {
        categories.push_back(game.mob_categories.get(c)->plural_name);
    }
    int selected_category_nr = (*cat)->id;
    
    if(ImGui::Combo("Category", &selected_category_nr, categories)) {
        if(category_change_callback) {
            category_change_callback();
        }
        *cat = game.mob_categories.get(selected_category_nr);
        
        vector<string> type_names;
        (*cat)->get_type_names(type_names);
        
        *typ = NULL;
        if(!type_names.empty()) {
            *typ = (*cat)->get_type(type_names[0]);
        }
    }
    set_tooltip(
        "What category this object belongs to: a Pikmin, a leader, etc."
    );
    
    if((*cat)->id != MOB_CATEGORY_NONE) {
    
        //Object type combobox.
        vector<string> types;
        (*cat)->get_type_names(types);
        for(size_t t = 0; t < types.size(); ) {
            mob_type* t_ptr = (*cat)->get_type(types[t]);
            if(only_show_area_editor_types && !t_ptr->appears_in_area_editor) {
                types.erase(types.begin() + t);
            } else {
                ++t;
            }
        }
        
        string selected_type_name;
        if(*typ) {
            selected_type_name = (*typ)->name;
        }
        if(ImGui::Combo("Type", &selected_type_name, types)) {
            if(type_change_callback) {
                type_change_callback();
            }
            *typ = (*cat)->get_type(selected_type_name);
        }
        set_tooltip(
            "The specific type of object this is, from the chosen category."
        );
    }
    
    ImGui::Columns();
}


/* ----------------------------------------------------------------------------
 * Process the width and height widgets that allow a user to
 * specify the size of something.
 * Returns true if the user changed one of the values.
 * label:
 *   Label for the widgets.
 * size:
 *   Size variable to alter.
 * v_speed:
 *   Variable change speed. Same value you'd pass to ImGui::DragFloat2.
 *   1.0f for default.
 * keep_aspect_ratio:
 *   If true, changing one will change the other in the same ratio.
 * min_size:
 *   Minimum value that either width or height is allowed to have.
 *   Use -FLT_MAX for none.
 * pre_change_callback:
 *   Callback to call when the width or height is changed, before it actually
 *   changes.
 */
bool editor::process_size_widgets(
    const char* label, point &size, const float v_speed,
    const bool keep_aspect_ratio,
    const float min_size,
    const std::function<void()> &pre_change_callback
) {
    bool ret = false;
    point new_size = size;
    if(
        ImGui::DragFloat2(label, (float*) &new_size, v_speed)
    ) {
        if(pre_change_callback) {
            pre_change_callback();
        }
        
        if(
            !keep_aspect_ratio ||
            size.x == 0.0f || size.y == 0.0f ||
            new_size.x == 0.0f || new_size.y == 0.0f
        ) {
            new_size.x = std::max(min_size, new_size.x);
            new_size.y = std::max(min_size, new_size.y);
        } else {
            float ratio = size.x / size.y;
            if(new_size.x != size.x) {
                new_size.x = std::max(min_size * ratio, new_size.x);
                new_size.x = std::max(min_size, new_size.x);
                new_size.y = new_size.x / ratio;
            } else {
                new_size.x = std::max(min_size / ratio, new_size.y);
                new_size.y = std::max(min_size, new_size.y);
                new_size.x = new_size.y * ratio;
            }
        }
        size = new_size;
        
        ret = true;
    }
    
    return ret;
}


/* ----------------------------------------------------------------------------
 * Processes an ImGui::TreeNode, except it pre-emptively opens it or closes it
 * based on the user's preferences. It also saves the user's preferences as
 * they open and close the node.
 * In order for these preferences to be saved onto disk, save_options must
 * be called.
 * category: Category this node belongs to. This is just a generic term, and
 *   you likely want to use the panel this node belongs to.
 * label:    Label to give to Dear ImGui.
 */
bool editor::saveable_tree_node(const string &category, const string &label) {
    string node_name = get_name() + "/" + category + "/" + label;
    ImGui::SetNextTreeNodeOpen(game.options.editor_open_nodes[node_name]);
    bool is_open = ImGui::TreeNode(label.c_str());
    game.options.editor_open_nodes[node_name] = is_open;
    return is_open;
}


/* ----------------------------------------------------------------------------
 * Sets the tooltip of the previous widget.
 * explanation:
 *   Text explaining the widget.
 * shortcut:
 *   If the widget has a shortcut key, specify its name here.
 */
void editor::set_tooltip(const string &explanation, const string &shortcut) {
    if(!game.options.editor_show_tooltips) {
        return;
    }
    
    if(ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("%s", explanation.c_str());
        if(!shortcut.empty()) {
            ImGui::TextColored(
                ImVec4(0.66f, 0.66f, 0.66f, 1.0f),
                "Shortcut key: %s", shortcut.c_str()
            );
        }
        ImGui::EndTooltip();
    }
}


/* ----------------------------------------------------------------------------
 * Unloads loaded editor-related content.
 */
void editor::unload() {
    if(bmp_editor_icons) {
        for(size_t i = 0; i < N_EDITOR_ICONS; ++i) {
            al_destroy_bitmap(editor_icons[i]);
            editor_icons[i] = NULL;
        }
        al_destroy_bitmap(bmp_editor_icons);
        bmp_editor_icons = NULL;
    }
}


/* ----------------------------------------------------------------------------
 * Updates the transformations, with the current camera coordinates, zoom, etc.
 */
void editor::update_transformations() {
    //World coordinates to screen coordinates.
    point canvas_center(
        (canvas_tl.x + canvas_br.x) / 2.0,
        (canvas_tl.y + canvas_br.y) / 2.0
    );
    game.world_to_screen_transform = game.identity_transform;
    al_translate_transform(
        &game.world_to_screen_transform,
        -game.cam.pos.x + canvas_center.x / game.cam.zoom,
        -game.cam.pos.y + canvas_center.y / game.cam.zoom
    );
    al_scale_transform(&game.world_to_screen_transform, game.cam.zoom, game.cam.zoom);
    
    //Screen coordinates to world coordinates.
    game.screen_to_world_transform = game.world_to_screen_transform;
    al_invert_transform(&game.screen_to_world_transform);
}


/* ----------------------------------------------------------------------------
 * Zooms to the specified level, keeping the mouse cursor in the same spot.
 */
void editor::zoom_with_cursor(const float new_zoom) {
    //Keep a backup of the old mouse coordinates.
    point old_mouse_pos = game.mouse_cursor_w;
    
    //Do the zoom.
    game.cam.set_zoom(clamp(new_zoom, zoom_min_level, zoom_max_level));
    update_transformations();
    
    //Figure out where the mouse will be after the zoom.
    game.mouse_cursor_w = game.mouse_cursor_s;
    al_transform_coordinates(
        &game.screen_to_world_transform,
        &game.mouse_cursor_w.x, &game.mouse_cursor_w.y
    );
    
    //Readjust the transformation by shifting the camera
    //so that the cursor ends up where it was before.
    game.cam.set_pos(
        point(
            game.cam.pos.x += (old_mouse_pos.x - game.mouse_cursor_w.x),
            game.cam.pos.y += (old_mouse_pos.y - game.mouse_cursor_w.y)
        )
    );
    
    //Update the mouse coordinates again.
    update_transformations();
    game.mouse_cursor_w = game.mouse_cursor_s;
    al_transform_coordinates(
        &game.screen_to_world_transform,
        &game.mouse_cursor_w.x, &game.mouse_cursor_w.y
    );
}


/* ----------------------------------------------------------------------------
 * Creates a new picker info.
 */
editor::picker_info::picker_info(editor* editor_ptr) :
    editor_ptr(editor_ptr),
    can_make_new(false),
    pick_callback(nullptr) {
}


/* ----------------------------------------------------------------------------
 * Processes the picker dialog for this frame.
 */
void editor::picker_info::process() {
    vector<string> category_names;
    vector<vector<picker_item> > final_items;
    string filter_lower = str_to_lower(filter);
    
    for(size_t i = 0; i < items.size(); ++i) {
        if(!filter.empty()) {
            string name_lower = str_to_lower(items[i].name);
            if(name_lower.find(filter_lower) == string::npos) {
                continue;
            }
        }
        
        size_t cat_index = INVALID;
        for(size_t c = 0; c < category_names.size(); ++c) {
            if(category_names[c] == items[i].category) {
                cat_index = c;
                break;
            }
        }
        
        if(cat_index == INVALID) {
            category_names.push_back(items[i].category);
            final_items.push_back(vector<picker_item>());
            cat_index = category_names.size() - 1;
        }
        
        final_items[cat_index].push_back(items[i]);
    }
    
    auto try_make_new = [this] () {
        if(filter.empty()) return;
        
        bool is_really_new = true;
        for(size_t i = 0; i < items.size(); ++i) {
            if(filter == items[i].name) {
                is_really_new = false;
                break;
            }
        }
        
        pick_callback(filter, "", is_really_new);
        is_open = false;
    };
    
    if(can_make_new) {
        ImGui::PushStyleColor(
            ImGuiCol_Button, (ImVec4) ImColor(192, 32, 32)
        );
        ImGui::PushStyleColor(
            ImGuiCol_ButtonHovered, (ImVec4) ImColor(208, 48, 48)
        );
        ImGui::PushStyleColor
        (ImGuiCol_ButtonActive, (ImVec4) ImColor(208, 32, 32)
        );
        if(ImGui::Button("+", ImVec2(64.0f, 32.0f))) {
            try_make_new();
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();
    }
    
    string filter_widget_hint =
        can_make_new ?
        "Search filter or new item name" :
        "Search filter";
        
    editor_ptr->next_input_needs_special_focus();
    if(
        ImGui::InputTextWithHint(
            "##filter", filter_widget_hint.c_str(), &filter,
            ImGuiInputTextFlags_EnterReturnsTrue
        )
    ) {
        if(can_make_new) {
            try_make_new();
        }
    }
    
    if(!list_header.empty()) {
        ImGui::Text("%s", list_header.c_str());
    }
    
    ImGuiStyle &style = ImGui::GetStyle();
    float picker_x2 =
        ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
        
    for(size_t c = 0; c < final_items.size(); ++c) {
    
        bool show = true;
        if(!category_names[c].empty()) {
            ImGui::SetNextTreeNodeOpen(true, ImGuiCond_Once);
            show = ImGui::TreeNode(category_names[c].c_str());
        }
        
        if(show) {
            for(size_t i = 0; i < final_items[c].size(); ++i) {
                picker_item* i_ptr = &final_items[c][i];
                ImGui::PushID((i2s(c) + "-" + i2s(i)).c_str());
                
                ImVec2 button_size;
                
                if(i_ptr->bitmap) {
                
                    ImGui::BeginGroup();
                    button_size = ImVec2(160.0f, 160.0f);
                    if(
                        ImGui::ImageButton(
                            (void*) i_ptr->bitmap,
                            button_size,
                            ImVec2(0.0f, 0.0f),
                            ImVec2(1.0f, 1.0f),
                            4.0f
                        )
                    ) {
                        pick_callback(
                            i_ptr->name, i_ptr->category, false
                        );
                        is_open = false;
                    }
                    ImGui::SetNextItemWidth(20.0f);
                    ImGui::TextWrapped("%s", i_ptr->name.c_str());
                    ImGui::Dummy(ImVec2(0.0f, 8.0f));
                    ImGui::EndGroup();
                    
                } else {
                
                    button_size = ImVec2(168.0f, 32.0f);
                    if(ImGui::Button(i_ptr->name.c_str(), button_size)) {
                        pick_callback(
                            i_ptr->name, i_ptr->category, false
                        );
                        is_open = false;
                    }
                    
                }
                
                float last_x2 = ImGui::GetItemRectMax().x;
                float next_x2 = last_x2 + style.ItemSpacing.x + button_size.x;
                if(i + 1 < final_items[c].size() && next_x2 < picker_x2) {
                    ImGui::SameLine();
                }
                ImGui::PopID();
            }
            
            if(!category_names[c].empty()) {
                ImGui::TreePop();
            }
            
        }
    }
}


/* ----------------------------------------------------------------------------
 * Creates a picker item.
 */
editor::picker_item::picker_item(
    const string &name, const string &category, ALLEGRO_BITMAP* bitmap
) :
    name(name),
    category(category),
    bitmap(bitmap) {
    
}


const float editor::transformation_widget::DEF_SIZE = 32.0f;
const float editor::transformation_widget::HANDLE_RADIUS = 6.0f;
const float editor::transformation_widget::OUTLINE_THICKNESS = 2.0f;
const float editor::transformation_widget::ROTATION_HANDLE_THICKNESS = 8.0f;

/* ----------------------------------------------------------------------------
 * Creates a new transformation widget.
 */
editor::transformation_widget::transformation_widget() :
    moving_handle(-1),
    old_angle(0.0f),
    old_mouse_angle(0.0f) {
    
}


/* ----------------------------------------------------------------------------
 * Draws the widget on-screen.
 * center:
 *   Center point.
 * size:
 *   Width and height. If NULL, no scale handles will be drawn.
 * angle:
 *   Angle. If NULL, the rotation handle will not be drawn.
 * zoom:
 *   Zoom the widget's components by this much.
 */
void editor::transformation_widget::draw(
    const point* const center, const point* const size,
    const float* const angle, const float zoom
) const {
    if(!center) return;
    
    point handles[9];
    float radius;
    get_locations(center, size, angle, handles, &radius, NULL);
    
    //Draw the rotation handle.
    if(angle && !isnanf(radius)) {
        al_draw_circle(
            center->x, center->y, radius,
            al_map_rgb(64, 64, 192), ROTATION_HANDLE_THICKNESS * zoom
        );
    }
    
    //Draw the outline.
    point corners[4] = {
        handles[0],
        handles[2],
        handles[8],
        handles[6],
    };
    for(unsigned char c = 0; c < 4; ++c) {
        size_t c2 = sum_and_wrap(c, 1, 4);
        al_draw_line(
            corners[c].x, corners[c].y,
            corners[c2].x, corners[c2].y,
            al_map_rgb(32, 32, 160), OUTLINE_THICKNESS * zoom
        );
    }
    
    //Draw the translation and scale handles.
    for(unsigned char h = 0; h < 9; ++h) {
        if(!size && h != 4) continue;
        al_draw_filled_circle(
            handles[h].x, handles[h].y,
            HANDLE_RADIUS * zoom, al_map_rgb(96, 96, 224)
        );
    }
}


/* ----------------------------------------------------------------------------
 * Returns the location of all handles, based on the information it
 * was fed.
 * center:
 *   Center point.
 * size:
 *   Width and height. If NULL, the default size is used.
 * angle:
 *   Angle. If NULL, zero is used.
 * handles:
 *   Return the location of all nine translation and scale handles here.
 * radius:
 *   Return the angle handle's radius here.
 * transform:
 *   If not NULL, return the transformation used here.
 *   The transformation will only rotate and translate, not scale.
 */
void editor::transformation_widget::get_locations(
    const point* const center, const point* const size,
    const float* const angle, point* handles, float* radius,
    ALLEGRO_TRANSFORM* transform
) const {
    point size_to_use(DEF_SIZE, DEF_SIZE);
    if(size) size_to_use = *size;
    
    //First, the Allegro transformation.
    ALLEGRO_TRANSFORM transform_to_use;
    al_identity_transform(&transform_to_use);
    if(angle) {
        al_rotate_transform(&transform_to_use, *angle);
    }
    al_translate_transform(&transform_to_use, center->x, center->y);
    
    //Get the coordinates of all translation and scale handles.
    handles[0] = { -size_to_use.x / 2.0f, -size_to_use.y / 2.0f };
    handles[1] = { 0.0f,                  -size_to_use.y / 2.0f };
    handles[2] = { size_to_use.x / 2.0f,  -size_to_use.y / 2.0f };
    handles[3] = { -size_to_use.x / 2.0f, 0.0f                  };
    handles[4] = { 0.0f,                  0.0f                  };
    handles[5] = { size_to_use.x / 2.0f,  0.0f                  };
    handles[6] = { -size_to_use.x / 2.0f, size_to_use.y / 2.0f  };
    handles[7] = { 0.0f,                  size_to_use.y / 2.0f  };
    handles[8] = { size_to_use.x / 2.0f,  size_to_use.y / 2.0f  };
    
    for(unsigned char h = 0; h < 9; ++h) {
        al_transform_coordinates(
            &transform_to_use, &handles[h].x, &handles[h].y
        );
    }
    
    *radius = dist(point(), size_to_use).to_float() / 2.0f;
    
    if(transform) *transform = transform_to_use;
}


/* ----------------------------------------------------------------------------
 * Handles the user having held the left mouse button down.
 * Returns true if the user did click on a handle.
 * mouse_coords:
 *   Mouse coordinates.
 * center:
 *   Center point.
 * size:
 *   Width and height. If NULL, no scale handling will be performed.
 * angle:
 *   Angle. If NULL, no rotation handling will be performed.
 * zoom:
 *   Zoom the widget's components by this much.
 */
bool editor::transformation_widget::handle_mouse_down(
    const point &mouse_coords, const point* const center,
    const point* const size, const float* const angle, const float zoom
) {
    if(!center) return false;
    
    point handles[9];
    float radius;
    get_locations(center, size, angle, handles, &radius, NULL);
    
    //Check if the user clicked on a translation or scale handle.
    for(unsigned char h = 0; h < 9; ++h) {
        if(dist(handles[h], mouse_coords) <= HANDLE_RADIUS * zoom) {
            if(h == 4) {
                moving_handle = h;
                return true;
            } else if(size) {
                moving_handle = h;
                old_size = *size;
                return true;
            }
        }
    }
    
    //Check if the user clicked on the rotation handle.
    if(angle) {
        dist d(*center, mouse_coords);
        if(
            d >= radius - ROTATION_HANDLE_THICKNESS / 2.0f * zoom &&
            d <= radius + ROTATION_HANDLE_THICKNESS / 2.0f * zoom
        ) {
            moving_handle = 9;
            old_angle = *angle;
            old_mouse_angle = get_angle(*center, mouse_coords);
            return true;
        }
    }
    
    return false;
}


/* ----------------------------------------------------------------------------
 * Handles the user having moved the mouse cursor.
 * Returns true if the user is dragging a handle.
 * mouse_coords:
 *   Mouse coordinates.
 * center:
 *   Center point.
 * size:
 *   Width and height. If NULL, no scale handling will be performed.
 * angle:
 *   Angle. If NULL, no rotation handling will be performed.
 * zoom:
 *   Zoom the widget's components by this much.
 * keep_aspect_ratio:
 *   If true, aspect ratio is kept when resizing.
 * min_size:
 *   Minimum possible size for the width or height. Use -FLT_MAX for none.
 */
bool editor::transformation_widget::handle_mouse_move(
    const point &mouse_coords, point* center, point* size, float* angle,
    const float zoom, const bool keep_aspect_ratio, const float min_size
) {
    if(!center) return false;
    
    if(moving_handle == -1) {
        return false;
    }
    
    //Logic for moving the center handle.
    if(moving_handle == 4) {
        *center = mouse_coords;
        return true;
    }
    
    //Logic for moving the rotation handle.
    if(moving_handle == 9 && angle) {
        *angle =
            old_angle +
            get_angle(*center, mouse_coords) - old_mouse_angle;
        return true;
    }
    
    //From here on out, it's logic to move a scale handle.
    if(!size) {
        return false;
    }
    
    ALLEGRO_TRANSFORM t;
    point handles[9];
    float radius;
    get_locations(center, size, angle, handles, &radius, &t);
    al_invert_transform(&t);
    
    point transformed_mouse = mouse_coords;
    point transformed_center = *center;
    point new_size = old_size;
    al_transform_coordinates(&t, &transformed_mouse.x, &transformed_mouse.y);
    al_transform_coordinates(&t, &transformed_center.x, &transformed_center.y);
    bool scaling_x = false;
    bool scaling_y = false;
    
    switch(moving_handle) {
    case 0:
    case 3:
    case 6: {
        new_size.x = size->x / 2.0f - transformed_mouse.x;
        scaling_x = true;
        break;
    } case 2:
    case 5:
    case 8: {
        new_size.x = transformed_mouse.x - (-size->x / 2.0f);
        scaling_x = true;
        break;
    }
    }
    
    switch(moving_handle) {
    case 0:
    case 1:
    case 2: {
        new_size.y = (size->y / 2.0f) - transformed_mouse.y;
        scaling_y = true;
        break;
    } case 6:
    case 7:
    case 8: {
        new_size.y = transformed_mouse.y - (-size->y / 2.0f);
        scaling_y = true;
        break;
    }
    }
    
    new_size.x = std::max(min_size, new_size.x);
    new_size.y = std::max(min_size, new_size.y);
    
    if(keep_aspect_ratio && old_size.x != 0.0f && old_size.y != 0.0f) {
        float scale_to_use;
        float w_scale = new_size.x / old_size.x;
        float h_scale = new_size.y / old_size.y;
        if(!scaling_y) {
            scale_to_use = w_scale;
        } else if(!scaling_x) {
            scale_to_use = h_scale;
        } else {
            if(fabs(w_scale) > fabs(h_scale)) {
                scale_to_use = w_scale;
            } else {
                scale_to_use = h_scale;
            }
        }
        scale_to_use = std::max(min_size / old_size.x, scale_to_use);
        scale_to_use = std::max(min_size / old_size.y, scale_to_use);
        new_size = old_size * scale_to_use;
    }
    
    switch(moving_handle) {
    case 0:
    case 3:
    case 6: {
        transformed_center.x = (size->x / 2.0f) - new_size.x / 2.0f;
        break;
    } case 2:
    case 5:
    case 8: {
        transformed_center.x = (-size->x / 2.0f) + new_size.x / 2.0f;
        break;
    }
    }
    
    switch(moving_handle) {
    case 0:
    case 1:
    case 2: {
        transformed_center.y = (size->y / 2.0f) - new_size.y / 2.0f;
        break;
    } case 6:
    case 7:
    case 8: {
        transformed_center.y = (-size->y / 2.0f) + new_size.y / 2.0f;
        break;
    }
    }
    
    point new_center = transformed_center;
    al_invert_transform(&t);
    al_transform_coordinates(&t, &new_center.x, &new_center.y);
    
    *center = new_center;
    *size = new_size;
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Handles the user having released the left mouse button.
 * Returns true if the user stopped dragging a handle.
 */
bool editor::transformation_widget::handle_mouse_up() {
    if(moving_handle == -1) {
        return false;
    }
    
    moving_handle = -1;
    return true;
}


/* ----------------------------------------------------------------------------
 * Is the user currently moving a handle?
 */
bool editor::transformation_widget::is_moving_handle() {
    return (moving_handle != -1);
}
