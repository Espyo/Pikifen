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
 * Initializes editor class stuff.
 */
editor::editor() :
    canvas_separator_x(-1),
    dialog_close_callback(nullptr),
    dialog_process_callback(nullptr),
    double_click_time(0),
    input_popup_focus_controller(2),
    is_ctrl_pressed(false),
    is_dialog_open(false),
    is_m1_pressed(false),
    is_m2_pressed(false),
    is_m3_pressed(false),
    is_shift_pressed(false),
    last_mouse_click(INVALID),
    loaded_content_yet(false),
    made_new_changes(false),
    mouse_drag_confirmed(false),
    picker_can_make_new(false),
    picker_pick_callback(nullptr),
    unsaved_changes_warning_timer(UNSAVED_CHANGES_WARNING_DURATION),
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
                c, 1.0 / game.cam.zoom
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
                c, 1.0 / game.cam.zoom
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
            is_dialog_open
        ) {
            is_dialog_open = false;
            if(dialog_close_callback) {
                dialog_close_callback();
            }
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
        if(input_popup_focus_controller > 0) {
            //In order to focus on the InputText when the input popup is
            //opened, we need to call SetKeyboardFocusHere two frames in
            //a row. I'm not quite sure why, but it's likely the same as
            //this: https://github.com/ocornut/imgui/issues/343
            ImGui::SetKeyboardFocusHere();
            input_popup_focus_controller--;
        }
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
    is_dialog_open = false;
    
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
    dialog_title = title;
    dialog_close_callback = nullptr;
    dialog_process_callback = process_callback;
    
    is_dialog_open = true;
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
 *   Its second argument is whether it's a new item or not.
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
    const std::function<void(const string &, const bool)> &pick_callback,
    const string &list_header,
    const bool can_make_new,
    const string &filter
) {
    picker_items = items;
    picker_list_header = list_header;
    picker_pick_callback = pick_callback;
    picker_can_make_new = can_make_new;
    picker_filter = filter;
    
    open_dialog(title, std::bind(&editor::process_picker, this));
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
 * Processes the dialog for this frame.
 */
void editor::process_dialog() {
    if(!is_dialog_open) return;
    
    ImGui::SetNextWindowPos(
        ImVec2(game.win_w / 2.0f, game.win_h / 2.0f),
        ImGuiCond_Always, ImVec2(0.5f, 0.5f)
    );
    ImGui::SetNextWindowSize(
        ImVec2(game.win_w * 0.8, game.win_h * 0.8), ImGuiCond_Once
    );
    ImGui::OpenPopup((dialog_title + "##dialog").c_str());
    
    if(
        ImGui::BeginPopupModal(
            (dialog_title + "##dialog").c_str(), &is_dialog_open
        )
    ) {
    
        dialog_process_callback();
        
        ImGui::EndPopup();
    }
    
    if(!is_dialog_open) {
        //The user closed it.
        if(dialog_close_callback) {
            dialog_close_callback();
        }
    }
}


/* ----------------------------------------------------------------------------
 * Processes the picker dialog for this frame.
 */
void editor::process_picker() {
    vector<picker_item> final_items;
    string filter_lower = str_to_lower(picker_filter);
    
    for(size_t i = 0; i < picker_items.size(); ++i) {
        if(!picker_filter.empty()) {
            string name_lower = str_to_lower(picker_items[i].name);
            if(name_lower.find(filter_lower) == string::npos) {
                continue;
            }
        }
        final_items.push_back(picker_items[i]);
    }
    
    auto try_make_new = [this] () {
        bool is_really_new = true;
        for(size_t i = 0; i < picker_items.size(); ++i) {
            if(picker_filter == picker_items[i].name) {
                is_really_new = false;
                break;
            }
        }
        
        picker_pick_callback(picker_filter, is_really_new);
        is_dialog_open = false;
    };
    
    if(picker_can_make_new) {
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
        picker_can_make_new ?
        "Search filter or new item name" :
        "Search filter";
    if(
        ImGui::InputTextWithHint(
            "##filter", filter_widget_hint.c_str(), &picker_filter,
            ImGuiInputTextFlags_EnterReturnsTrue
        )
    ) {
        if(picker_can_make_new) {
            try_make_new();
        }
    }
    
    if(!picker_list_header.empty()) {
        ImGui::Text("%s", picker_list_header.c_str());
    }
    
    ImGuiStyle &style = ImGui::GetStyle();
    float picker_x2 =
        ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
        
    for(size_t i = 0; i < final_items.size(); ++i) {
        ImGui::PushID(i);
        
        ImVec2 button_size;
        
        if(final_items[i].bitmap) {
        
            ImGui::BeginGroup();
            button_size = ImVec2(160.0f, 160.0f);
            if(
                ImGui::ImageButton(
                    (void*) final_items[i].bitmap,
                    button_size,
                    ImVec2(0.0f, 0.0f),
                    ImVec2(1.0f, 1.0f),
                    4.0f
                )
            ) {
                picker_pick_callback(final_items[i].name, false);
                is_dialog_open = false;
            }
            ImGui::SetNextItemWidth(20.0f);
            ImGui::TextWrapped("%s", final_items[i].name.c_str());
            ImGui::Dummy(ImVec2(0.0f, 8.0f));
            ImGui::EndGroup();
            
        } else {
        
            button_size = ImVec2(168.0f, 32.0f);
            if(ImGui::Button(final_items[i].name.c_str(), button_size)) {
                picker_pick_callback(final_items[i].name, false);
                is_dialog_open = false;
            }
            
        }
        
        float last_x2 = ImGui::GetItemRectMax().x;
        float next_x2 = last_x2 + style.ItemSpacing.x + button_size.x;
        if(i + 1 < final_items.size() && next_x2 < picker_x2) {
            ImGui::SameLine();
        }
        ImGui::PopID();
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
    game.cam.set_zoom(clamp(new_zoom, zoom_min_level, zoom_max_level));
    
    //Keep a backup of the old mouse coordinates.
    point old_mouse_pos = game.mouse_cursor_w;
    
    //Figure out where the mouse will be after the zoom.
    update_transformations();
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
    
    update_transformations();
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



const float editor::transformation_controller::HANDLE_RADIUS = 6.0;
const float editor::transformation_controller::ROTATION_HANDLE_THICKNESS = 8.0;


/* ----------------------------------------------------------------------------
 * Creates a transformation controller.
 */
editor::transformation_controller::transformation_controller() :
    moving_handle(-1),
    size(point(16, 16)),
    angle(0),
    keep_aspect_ratio(true),
    allow_rotation(false) {
    
}


/* ----------------------------------------------------------------------------
 * Draws the transformation (move, scale, rotate) handles.
 */
void editor::transformation_controller::draw_handles() {
    if(size.x == 0 || size.y == 0) return;
    
    //Rotation handle.
    if(allow_rotation && !isnanf(radius)) {
        al_draw_circle(
            center.x, center.y, radius,
            al_map_rgb(64, 64, 192), ROTATION_HANDLE_THICKNESS / game.cam.zoom
        );
    }
    
    //Outline.
    point corners[4];
    corners[0] = point(-size.x / 2.0, -size.y / 2.0);
    corners[1] = point(size.x / 2.0, -size.y / 2.0);
    corners[2] = point(size.x / 2.0, size.y / 2.0);
    corners[3] = point(-size.x / 2.0, size.y / 2.0);
    for(unsigned char c = 0; c < 4; ++c) {
        al_transform_coordinates(
            &disalign_transform, &corners[c].x, &corners[c].y
        );
    }
    for(unsigned char c = 0; c < 4; ++c) {
        size_t c2 = sum_and_wrap(c, 1, 4);
        al_draw_line(
            corners[c].x, corners[c].y,
            corners[c2].x, corners[c2].y,
            al_map_rgb(32, 32, 160), 2.0 / game.cam.zoom
        );
    }
    
    //Translation and scale handles.
    for(unsigned char h = 0; h < 9; ++h) {
        point handle_pos = get_handle_pos(h);
        al_draw_filled_circle(
            handle_pos.x, handle_pos.y,
            HANDLE_RADIUS / game.cam.zoom, al_map_rgb(96, 96, 224)
        );
    }
}


/* ----------------------------------------------------------------------------
 * Returns the angle.
 */
float editor::transformation_controller::get_angle() const {
    return angle;
}


/* ----------------------------------------------------------------------------
 * Returns the center.
 */
point editor::transformation_controller::get_center() const {
    return center;
}


/* ----------------------------------------------------------------------------
 * Returns the position at which a handle is.
 */
point editor::transformation_controller::get_handle_pos(
    const unsigned char handle
) const {
    point result;
    switch(handle) {
    case 0:
    case 3:
    case 6: {
        result.x = -size.x / 2.0;
        break;
    } case 2:
    case 5:
    case 8: {
        result.x = size.x / 2.0;
        break;
    }
    }
    
    switch(handle) {
    case 0:
    case 1:
    case 2: {
        result.y = -size.y / 2.0;
        break;
    }
    case 6:
    case 7:
    case 8: {
        result.y = size.y / 2.0;
        break;
    }
    }
    
    al_transform_coordinates(&disalign_transform, &result.x, &result.y);
    return result;
}


/* ----------------------------------------------------------------------------
 * Returns the size.
 */
point editor::transformation_controller::get_size() const {
    return size;
}


/* ----------------------------------------------------------------------------
 * Handles a mouse press, allowing a handle to be grabbed.
 * Returns true if handled, false if nothing was done.
 */
bool editor::transformation_controller::handle_mouse_down(const point pos) {
    if(size.x == 0 || size.y == 0) return false;
    
    for(unsigned char h = 0; h < 9; ++h) {
        point handle_pos = get_handle_pos(h);
        if(dist(handle_pos, pos) <= HANDLE_RADIUS / game.cam.zoom) {
            moving_handle = h;
            pre_move_size = size;
            return true;
        }
    }
    
    if(allow_rotation) {
        dist d(center, pos);
        if(
            d >= radius - ROTATION_HANDLE_THICKNESS / game.cam.zoom / 2.0 &&
            d <= radius + ROTATION_HANDLE_THICKNESS / game.cam.zoom / 2.0
        ) {
            moving_handle = 9;
            pre_rotation_angle = angle;
            pre_rotation_mouse_angle = ::get_angle(center, pos);
            return true;
        }
    }
    
    return false;
}


/* ----------------------------------------------------------------------------
 * Handles a mouse move, allowing a handle to be moved.
 * Returns true if handled, false if nothing was done.
 */
bool editor::transformation_controller::handle_mouse_move(const point pos) {
    if(moving_handle == -1) {
        return false;
    }
    
    if(moving_handle == 4) {
        set_center(pos);
        return true;
    }
    
    if(moving_handle == 9) {
        set_angle(
            pre_rotation_angle +
            (::get_angle(center, pos) - pre_rotation_mouse_angle)
        );
        return true;
    }
    
    point aligned_cursor_pos = pos;
    al_transform_coordinates(
        &align_transform,
        &aligned_cursor_pos.x, &aligned_cursor_pos.y
    );
    point new_size = pre_move_size;
    point aligned_new_center = center;
    al_transform_coordinates(
        &align_transform,
        &aligned_new_center.x, &aligned_new_center.y
    );
    
    switch(moving_handle) {
    case 0:
    case 3:
    case 6: {
        new_size.x = size.x / 2.0 - aligned_cursor_pos.x;
        break;
    } case 2:
    case 5:
    case 8: {
        new_size.x = aligned_cursor_pos.x - (-size.x / 2.0);
        break;
    }
    }
    
    switch(moving_handle) {
    case 0:
    case 1:
    case 2: {
        new_size.y = (size.y / 2.0) - aligned_cursor_pos.y;
        break;
    } case 6:
    case 7:
    case 8: {
        new_size.y = aligned_cursor_pos.y - (-size.y / 2.0);
        break;
    }
    }
    
    if(keep_aspect_ratio) {
        if(
            fabs(pre_move_size.x - new_size.x) >
            fabs(pre_move_size.y - new_size.y)
        ) {
            //Most significant change is width.
            if(pre_move_size.x != 0) {
                float ratio = pre_move_size.y / pre_move_size.x;
                new_size.y = new_size.x * ratio;
            }
            
        } else {
            //Most significant change is height.
            if(pre_move_size.y != 0) {
                float ratio = pre_move_size.x / pre_move_size.y;
                new_size.x = new_size.y * ratio;
            }
            
        }
    }
    
    switch(moving_handle) {
    case 0:
    case 3:
    case 6: {
        aligned_new_center.x = (size.x / 2.0) - new_size.x / 2.0;
        break;
    } case 2:
    case 5:
    case 8: {
        aligned_new_center.x = (-size.x / 2.0) + new_size.x / 2.0;
        break;
    }
    }
    
    switch(moving_handle) {
    case 0:
    case 1:
    case 2: {
        aligned_new_center.y = (size.y / 2.0) - new_size.y / 2.0;
        break;
    } case 6:
    case 7:
    case 8: {
        aligned_new_center.y = (-size.y / 2.0) + new_size.y / 2.0;
        break;
    }
    }
    
    point new_center = aligned_new_center;
    al_transform_coordinates(
        &disalign_transform,
        &new_center.x, &new_center.y
    );
    
    set_center(new_center);
    set_size(new_size);
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Handles a mouse release, allowing a handle to be released.
 */
void editor::transformation_controller::handle_mouse_up() {
    moving_handle = -1;
}


/* ----------------------------------------------------------------------------
 * Sets the angle.
 */
void editor::transformation_controller::set_angle(const float angle) {
    this->angle = angle;
    update();
}


/* ----------------------------------------------------------------------------
 * Sets the center.
 */
void editor::transformation_controller::set_center(const point &center) {
    this->center = center;
    update();
}


/* ----------------------------------------------------------------------------
 * Sets the size.
 */
void editor::transformation_controller::set_size(const point &size) {
    this->size = size;
    update();
}


/* ----------------------------------------------------------------------------
 * Updates the transformations to match the new data, as well as
 * some caches.
 */
void editor::transformation_controller::update() {
    al_identity_transform(&align_transform);
    al_translate_transform(&align_transform, -center.x, -center.y);
    al_rotate_transform(&align_transform, -angle);
    
    al_copy_transform(&disalign_transform, &align_transform);
    al_invert_transform(&disalign_transform);
    
    radius = dist(center, center + (size / 2)).to_float();
}
