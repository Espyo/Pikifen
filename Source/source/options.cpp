/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Game options class and related functions.
 */

#include <algorithm>

#include "options.h"

#include "functions.h"
#include "game.h"
#include "misc_structs.h"
#include "utils/string_utils.h"


namespace OPTIONS {
//Default value for the area editor advanced mode setting.
const bool DEF_AREA_EDITOR_ADVANCED_MODE = false;
//Default value for the area editor backup interval.
const float DEF_AREA_EDITOR_BACKUP_INTERVAL = 120.0f;
//Default value for the area editor grid interval.
const float DEF_AREA_EDITOR_GRID_INTERVAL = 32.0f;
//Default value for the area editor selection transformation widget.
const bool DEF_AREA_EDITOR_SEL_TRANS = false;
//Default value for whether to show a circular sector's info in the area editor.
const bool DEF_AREA_EDITOR_SHOW_CIRCULAR_INFO = true;
//Default value for whether to show an edge's length in the area editor.
const bool DEF_AREA_EDITOR_SHOW_EDGE_LENGTH = true;
//Default value for whether to show a path link's length in the area editor.
const bool DEF_AREA_EDITOR_SHOW_PATH_LINK_LENGTH = true;
//Default value for whether to show a mob's territory in the area editor.
const bool DEF_AREA_EDITOR_SHOW_TERRITORY = false;
//Default value for the area editor snap mode.
const area_editor::SNAP_MODES DEF_AREA_EDITOR_SNAP_MODE =
    area_editor::SNAP_GRID;
//Default value for the area editor snap threshold.
const size_t DEF_AREA_EDITOR_SNAP_THRESHOLD = 80;
//Default value for the area editor undo limit.
const size_t DEF_AREA_EDITOR_UNDO_LIMIT = 20;
//Default value for the area editor view mode.
const area_editor::VIEW_MODES DEF_AREA_EDITOR_VIEW_MODE =
    area_editor::VIEW_MODE_TEXTURES;
//Default value for the auto-throw mode.
const AUTO_THROW_MODES DEF_AUTO_THROW_MODE = AUTO_THROW_OFF;
//Default value for the cursor camera weight.
const float DEF_CURSOR_CAM_WEIGHT = 0.0f;
//Default value for the cursor speed.
const float DEF_CURSOR_SPEED = 500.0f;
//Default value for the cursor trail.
const bool DEF_DRAW_CURSOR_TRAIL = true;
//Default value for the editor highlights.
const ALLEGRO_COLOR DEF_EDITOR_HIGHLIGHT_COLOR = { 1.0f, 1.0f, 1.0f, 1.0f };
//Default value for whether the middle mouse button pans in editors.
const bool DEF_EDITOR_MMB_PAN = false;
//Default value for the editor mouse drag threshold.
const float DEF_EDITOR_MOUSE_DRAG_THRESHOLD = 4;
//Default value for the editor primary color.
const ALLEGRO_COLOR DEF_EDITOR_PRIMARY_COLOR = {0.05f, 0.05f, 0.05f, 1.0f};
//Default value for the editor secondary color.
const ALLEGRO_COLOR DEF_EDITOR_SECONDARY_COLOR = {0.19f, 0.47f, 0.78f, 1.0f};
//Default value for whether to show tooltips in editors.
const bool DEF_EDITOR_SHOW_TOOLTIPS = true;
//Default value for the editor text color.
const ALLEGRO_COLOR DEF_EDITOR_TEXT_COLOR = {1.0f, 1.0f, 1.0f, 1.0f};
//Default value for whether to use custom styles in editors.
const bool DEF_EDITOR_USE_CUSTOM_STYLE = false;
//Default value for the GUI editor grid interval.
const float DEF_GUI_EDITOR_GRID_INTERVAL = 2.5f;
//Default value for the GUI editor snap mode.
const bool DEF_GUI_EDITOR_SNAP = true;
//Default value for the joystick maximum deadzone.
const float DEF_JOYSTICK_MAX_DEADZONE = 0.9f;
//Default value for the joystick minimum deadzone.
const float DEF_JOYSTICK_MIN_DEADZONE = 0.2f;
//Default value for the pause menu leaving confirmation mode.
const LEAVING_CONFIRMATION_MODES DEF_LEAVING_CONFIRMATION_MODE =
    LEAVING_CONFIRMATION_ALWAYS;
//Default value for the maximum amount of particles.
const size_t DEF_MAX_PARTICLES = 200;
//Default value for whether mipmaps are enabled.
const bool DEF_MIPMAPS_ENABLED = true;
//Default value for whether the mouse moves the cursor, for each player.
const bool DEF_MOUSE_MOVES_CURSOR[MAX_PLAYERS] = {true, false, false, false};
//Default value for whether to show HUD controls.
const bool DEF_SHOW_HUD_CONTROLS = true;
//Default value for whether to use smooth scaling.
const bool DEF_SMOOTH_SCALING = true;
//Default value for the default target framerate.
const unsigned int DEF_TARGET_FPS = 60;
//Default value for whether to use true fullscreen.
const bool DEF_TRUE_FULLSCREEN = false;
//Default value for whether to use the window position hack.
const bool DEF_WINDOW_POSITION_HACK = false;
//Default value for whether to use fullscreen.
const bool DEF_WIN_FULLSCREEN = false;
//Default value for the window height.
const unsigned int DEF_WIN_H = 768;
//Default value for the window width.
const unsigned int DEF_WIN_W = 1024;
//Default value for the middle zoom level.
const float DEF_ZOOM_MID_LEVEL = 1.4f;
}


/* ----------------------------------------------------------------------------
 * Creates an options struct.
 */
options_struct::options_struct() :
    area_editor_advanced_mode(OPTIONS::DEF_AREA_EDITOR_ADVANCED_MODE),
    area_editor_backup_interval(OPTIONS::DEF_AREA_EDITOR_BACKUP_INTERVAL),
    area_editor_grid_interval(OPTIONS::DEF_AREA_EDITOR_GRID_INTERVAL),
    area_editor_sel_trans(OPTIONS::DEF_AREA_EDITOR_SEL_TRANS),
    area_editor_show_circular_info(OPTIONS::DEF_AREA_EDITOR_SHOW_CIRCULAR_INFO),
    area_editor_show_edge_length(OPTIONS::DEF_AREA_EDITOR_SHOW_EDGE_LENGTH),
    area_editor_show_path_link_length(
        OPTIONS::DEF_AREA_EDITOR_SHOW_PATH_LINK_LENGTH
    ),
    area_editor_show_territory(OPTIONS::DEF_AREA_EDITOR_SHOW_TERRITORY),
    area_editor_snap_mode(OPTIONS::DEF_AREA_EDITOR_SNAP_MODE),
    area_editor_snap_threshold(OPTIONS::DEF_AREA_EDITOR_SNAP_THRESHOLD),
    area_editor_undo_limit(OPTIONS::DEF_AREA_EDITOR_UNDO_LIMIT),
    area_editor_view_mode(OPTIONS::DEF_AREA_EDITOR_VIEW_MODE),
    auto_throw_mode(OPTIONS::DEF_AUTO_THROW_MODE),
    cursor_cam_weight(OPTIONS::DEF_CURSOR_CAM_WEIGHT),
    cursor_speed(OPTIONS::DEF_CURSOR_SPEED),
    draw_cursor_trail(OPTIONS::DEF_DRAW_CURSOR_TRAIL),
    editor_highlight_color(OPTIONS::DEF_EDITOR_HIGHLIGHT_COLOR),
    editor_mmb_pan(OPTIONS::DEF_EDITOR_MMB_PAN),
    editor_mouse_drag_threshold(OPTIONS::DEF_EDITOR_MOUSE_DRAG_THRESHOLD),
    editor_primary_color(OPTIONS::DEF_EDITOR_PRIMARY_COLOR),
    editor_secondary_color(OPTIONS::DEF_EDITOR_SECONDARY_COLOR),
    editor_show_tooltips(OPTIONS::DEF_EDITOR_SHOW_TOOLTIPS),
    editor_text_color(OPTIONS::DEF_EDITOR_TEXT_COLOR),
    editor_use_custom_style(OPTIONS::DEF_EDITOR_USE_CUSTOM_STYLE),
    gui_editor_grid_interval(OPTIONS::DEF_GUI_EDITOR_GRID_INTERVAL),
    gui_editor_snap(OPTIONS::DEF_GUI_EDITOR_SNAP),
    intended_win_fullscreen(OPTIONS::DEF_WIN_FULLSCREEN),
    intended_win_h(OPTIONS::DEF_WIN_H),
    intended_win_w(OPTIONS::DEF_WIN_W),
    joystick_max_deadzone(OPTIONS::DEF_JOYSTICK_MAX_DEADZONE),
    joystick_min_deadzone(OPTIONS::DEF_JOYSTICK_MIN_DEADZONE),
    leaving_confirmation_mode(OPTIONS::DEF_LEAVING_CONFIRMATION_MODE),
    max_particles(OPTIONS::DEF_MAX_PARTICLES),
    mipmaps_enabled(OPTIONS::DEF_MIPMAPS_ENABLED),
    smooth_scaling(OPTIONS::DEF_SMOOTH_SCALING),
    show_hud_controls(OPTIONS::DEF_SHOW_HUD_CONTROLS),
    target_fps(OPTIONS::DEF_TARGET_FPS),
    true_fullscreen(OPTIONS::DEF_TRUE_FULLSCREEN),
    window_position_hack(OPTIONS::DEF_WINDOW_POSITION_HACK),
    zoom_mid_level(OPTIONS::DEF_ZOOM_MID_LEVEL) {
    
    mouse_moves_cursor[0] = OPTIONS::DEF_MOUSE_MOVES_CURSOR[0];
    mouse_moves_cursor[1] = OPTIONS::DEF_MOUSE_MOVES_CURSOR[1];
    mouse_moves_cursor[2] = OPTIONS::DEF_MOUSE_MOVES_CURSOR[2];
    mouse_moves_cursor[3] = OPTIONS::DEF_MOUSE_MOVES_CURSOR[3];
}


/* ----------------------------------------------------------------------------
 * Loads the player's options from a file.
 * file:
 *   File to read from.
 */
void options_struct::load(data_node* file) {
    reader_setter rs(file);
    
    /* Load controls. Format of a control:
     * "p<player>_<action>=<possible control 1>,<possible control 2>,<...>"
     * Format of a possible control:
     * "<input method>_<parameters, underscore separated>"
     * Input methods:
     * "k" (keyboard key), "mb" (mouse button),
     * "mwu" (mouse wheel up), "mwd" (down),
     * "mwl" (left), "mwr" (right), "jb" (joystick button),
     * "jap" (joystick axis, positive), "jan" (joystick axis, negative).
     * The parameters are the key/button number, joystick number,
     * joystick stick and axis, etc.
     * Check the constructor of control_info for more information.
     */
    for(unsigned char p = 0; p < MAX_PLAYERS; ++p) {
        controls[p].clear();
        for(size_t b = 0; b < N_BUTTONS; ++b) {
            string internal_name = game.buttons.list[b].internal_name;
            if(internal_name.empty()) continue;
            load_control(game.buttons.list[b].id, p, internal_name, file);
        }
    }
    
    //Weed out controls that didn't parse correctly.
    for(unsigned char p = 0; p < MAX_PLAYERS; p++) {
        for(size_t c = 0; c < controls[p].size(); ) {
            if(controls[p][c].action == BUTTON_NONE) {
                controls[p].erase(controls[p].begin() + c);
            } else {
                c++;
            }
        }
    }
    
    for(unsigned char p = 0; p < MAX_PLAYERS; ++p) {
        rs.set(
            "p" + i2s((p + 1)) + "_mouse_moves_cursor",
            mouse_moves_cursor[p]
        );
    }
    
    //Opened tree nodes in editors.
    editor_open_nodes.clear();
    vector<string> open_nodes_vector =
        semicolon_list_to_vector(
            file->get_child_by_name("editor_open_nodes")->value
        );
    for(size_t n = 0; n < open_nodes_vector.size(); ++n) {
        editor_open_nodes[open_nodes_vector[n]] = true;
    }
    
    //Other options.
    string resolution_str;
    unsigned char editor_snap_mode_c;
    unsigned char editor_view_mode_c;
    unsigned char auto_throw_mode_c;
    unsigned char leaving_confirmation_mode_c;
    
    rs.set("area_editor_advanced_mode", area_editor_advanced_mode);
    rs.set("area_editor_backup_interval", area_editor_backup_interval);
    rs.set("area_editor_grid_interval", area_editor_grid_interval);
    rs.set("area_editor_selection_transformation", area_editor_sel_trans);
    rs.set("area_editor_show_circular_info", area_editor_show_circular_info);
    rs.set("area_editor_show_edge_length", area_editor_show_edge_length);
    rs.set(
        "area_editor_show_path_link_length",
        area_editor_show_path_link_length
    );
    rs.set("area_editor_show_territory", area_editor_show_territory);
    rs.set("area_editor_snap_mode", editor_snap_mode_c);
    rs.set("area_editor_snap_threshold", area_editor_snap_threshold);
    rs.set("area_editor_undo_limit", area_editor_undo_limit);
    rs.set("area_editor_view_mode", editor_view_mode_c);
    rs.set("auto_throw_mode", auto_throw_mode_c);
    rs.set("cursor_cam_weight", cursor_cam_weight);
    rs.set("cursor_speed", cursor_speed);
    rs.set("draw_cursor_trail", draw_cursor_trail);
    rs.set("editor_highlight_color", editor_highlight_color);
    rs.set("editor_mmb_pan", editor_mmb_pan);
    rs.set("editor_mouse_drag_threshold", editor_mouse_drag_threshold);
    rs.set("editor_primary_color", editor_primary_color);
    rs.set("editor_secondary_color", editor_secondary_color);
    rs.set("editor_show_tooltips", editor_show_tooltips);
    rs.set("editor_text_color", editor_text_color);
    rs.set("editor_use_custom_style", editor_use_custom_style);
    rs.set("fps", target_fps);
    rs.set("fullscreen", intended_win_fullscreen);
    rs.set("gui_editor_grid_interval", gui_editor_grid_interval);
    rs.set("gui_editor_snap", gui_editor_snap);
    rs.set("joystick_min_deadzone", joystick_min_deadzone);
    rs.set("joystick_max_deadzone", joystick_max_deadzone);
    rs.set("leaving_confirmation_mode", leaving_confirmation_mode_c);
    rs.set("max_particles", max_particles);
    rs.set("middle_zoom_level", zoom_mid_level);
    rs.set("mipmaps", mipmaps_enabled);
    rs.set("resolution", resolution_str);
    rs.set("smooth_scaling", smooth_scaling);
    rs.set("show_hud_controls", show_hud_controls);
    rs.set("true_fullscreen", true_fullscreen);
    rs.set("window_position_hack", window_position_hack);
    
    auto_throw_mode =
        (AUTO_THROW_MODES)
        std::min(
            auto_throw_mode_c,
            (unsigned char) (N_AUTO_THROW_MODES - 1)
        );
    area_editor_snap_mode =
        (area_editor::SNAP_MODES)
        std::min(
            editor_snap_mode_c,
            (unsigned char) (area_editor::N_SNAP_MODES - 1)
        );
    area_editor_view_mode =
        (area_editor::VIEW_MODES)
        std::min(
            editor_view_mode_c,
            (unsigned char) (area_editor::N_VIEW_MODES - 1)
        );
    leaving_confirmation_mode =
        (LEAVING_CONFIRMATION_MODES)
        std::min(
            leaving_confirmation_mode_c,
            (unsigned char) (N_LEAVING_CONFIRMATION_MODES - 1)
        );
    target_fps = std::max(1, target_fps);
    
    if(joystick_min_deadzone > joystick_max_deadzone) {
        std::swap(joystick_min_deadzone, joystick_max_deadzone);
    }
    if(joystick_min_deadzone == joystick_max_deadzone) {
        joystick_min_deadzone -= 0.1;
        joystick_max_deadzone += 0.1;
    }
    joystick_min_deadzone = clamp(joystick_min_deadzone, 0.0f, 1.0f);
    joystick_max_deadzone = clamp(joystick_max_deadzone, 0.0f, 1.0f);
    
    vector<string> resolution_parts = split(resolution_str);
    if(resolution_parts.size() >= 2) {
        intended_win_w = std::max(1, s2i(resolution_parts[0]));
        intended_win_h = std::max(1, s2i(resolution_parts[1]));
    }
    
    //Force the editor styles to be opaque, otherwise there can be problems.
    editor_primary_color.a = 1.0f;
    editor_secondary_color.a = 1.0f;
    editor_text_color.a = 1.0f;
    editor_highlight_color.a = 1.0f;
}


/* ----------------------------------------------------------------------------
 * Loads a game control from the options file.
 * action:
 *   Load the control corresponding to this action.
 * player:
 *   Load the control corresponding to this player.
 * name:
 *   Name of the option in the file.
 * file:
 *   File to load from.
 * def:
 *   Default value of this control. Only applicable for player 1.
 */
void options_struct::load_control(
    const BUTTONS action, const unsigned char player,
    const string &name, data_node* file, const string &def
) {
    data_node* control_node =
        file->get_child_by_name("p" + i2s((player + 1)) + "_" + name);
    vector<string> possible_controls =
        semicolon_list_to_vector(
            control_node->get_value_or_default((player == 0) ? def : "")
        );
        
    for(size_t c = 0; c < possible_controls.size(); ++c) {
        controls[player].push_back(control_info(action, possible_controls[c]));
    }
}


/* ----------------------------------------------------------------------------
 * Saves the player's options into a file.
 * file:
 *   File to write to.
 */
void options_struct::save(data_node* file) const {
    //First, group the controls by action and player.
    map<string, string> grouped_controls;
    
    for(unsigned char p = 0; p < MAX_PLAYERS; ++p) {
        string prefix = "p" + i2s((p + 1)) + "_";
        for(size_t b = 0; b < N_BUTTONS; ++b) {
            string internal_name = game.buttons.list[b].internal_name;
            if(internal_name.empty()) continue;
            grouped_controls[prefix + internal_name].clear();
        }
    }
    
    //Write down their control strings.
    for(size_t p = 0; p < MAX_PLAYERS; p++) {
        for(size_t c = 0; c < controls[p].size(); ++c) {
            string name = "p" + i2s(p + 1) + "_";
            
            for(size_t b = 0; b < N_BUTTONS; ++b) {
                if(game.buttons.list[b].internal_name.empty()) continue;
                if(controls[p][c].action == game.buttons.list[b].id) {
                    name += game.buttons.list[b].internal_name;
                    break;
                }
            }
            
            grouped_controls[name] += controls[p][c].stringify() + ";";
        }
    }
    
    //Save controls.
    for(auto &c : grouped_controls) {
        //Remove the final character, which is always an extra semicolon.
        if(c.second.size()) c.second.erase(c.second.size() - 1);
        
        file->add(new data_node(c.first, c.second));
    }
    
    for(unsigned char p = 0; p < MAX_PLAYERS; ++p) {
        file->add(
            new data_node(
                "p" + i2s((p + 1)) + "_mouse_moves_cursor",
                b2s(mouse_moves_cursor[p])
            )
        );
    }
    
    //Figure out the value for the editor tree node preferences.
    string open_nodes_str;
    for(auto n : editor_open_nodes) {
        if(n.second) {
            open_nodes_str += n.first + ";";
        }
    }
    if(!open_nodes_str.empty()) open_nodes_str.pop_back();
    
    //Other options.
    file->add(
        new data_node(
            "area_editor_advanced_mode",
            b2s(area_editor_advanced_mode)
        )
    );
    file->add(
        new data_node(
            "area_editor_backup_interval",
            f2s(area_editor_backup_interval)
        )
    );
    file->add(
        new data_node(
            "area_editor_grid_interval",
            i2s(area_editor_grid_interval)
        )
    );
    file->add(
        new data_node(
            "area_editor_selection_transformation",
            b2s(area_editor_sel_trans)
        )
    );
    file->add(
        new data_node(
            "area_editor_show_circular_info",
            b2s(area_editor_show_circular_info)
        )
    );
    file->add(
        new data_node(
            "area_editor_show_edge_length",
            b2s(area_editor_show_edge_length)
        )
    );
    file->add(
        new data_node(
            "area_editor_show_path_link_length",
            b2s(area_editor_show_path_link_length)
        )
    );
    file->add(
        new data_node(
            "area_editor_show_territory",
            b2s(area_editor_show_territory)
        )
    );
    file->add(
        new data_node(
            "area_editor_snap_mode",
            i2s(area_editor_snap_mode)
        )
    );
    file->add(
        new data_node(
            "area_editor_snap_threshold",
            i2s(area_editor_snap_threshold)
        )
    );
    file->add(
        new data_node(
            "area_editor_undo_limit",
            i2s(area_editor_undo_limit)
        )
    );
    file->add(
        new data_node(
            "area_editor_view_mode",
            i2s(area_editor_view_mode)
        )
    );
    file->add(
        new data_node(
            "auto_throw_mode",
            i2s(auto_throw_mode)
        )
    );
    file->add(
        new data_node(
            "cursor_cam_weight",
            f2s(cursor_cam_weight)
        )
    );
    file->add(
        new data_node(
            "cursor_speed",
            f2s(cursor_speed)
        )
    );
    file->add(
        new data_node(
            "draw_cursor_trail",
            b2s(draw_cursor_trail)
        )
    );
    file->add(
        new data_node(
            "editor_highlight_color",
            c2s(editor_highlight_color)
        )
    );
    file->add(
        new data_node(
            "editor_mmb_pan",
            b2s(editor_mmb_pan)
        )
    );
    file->add(
        new data_node(
            "editor_mouse_drag_threshold",
            i2s(editor_mouse_drag_threshold)
        )
    );
    file->add(
        new data_node(
            "editor_open_nodes",
            open_nodes_str
        )
    );
    file->add(
        new data_node(
            "editor_primary_color",
            c2s(editor_primary_color)
        )
    );
    file->add(
        new data_node(
            "editor_secondary_color",
            c2s(editor_secondary_color)
        )
    );
    file->add(
        new data_node(
            "editor_show_tooltips",
            b2s(editor_show_tooltips)
        )
    );
    file->add(
        new data_node(
            "editor_text_color",
            c2s(editor_text_color)
        )
    );
    file->add(
        new data_node(
            "editor_use_custom_style",
            b2s(editor_use_custom_style)
        )
    );
    file->add(
        new data_node(
            "fps",
            i2s(target_fps)
        )
    );
    file->add(
        new data_node(
            "fullscreen",
            b2s(intended_win_fullscreen)
        )
    );
    file->add(
        new data_node(
            "gui_editor_grid_interval",
            f2s(gui_editor_grid_interval)
        )
    );
    file->add(
        new data_node(
            "gui_editor_snap",
            b2s(gui_editor_snap)
        )
    );
    file->add(
        new data_node(
            "joystick_max_deadzone",
            f2s(joystick_max_deadzone)
        )
    );
    file->add(
        new data_node(
            "joystick_min_deadzone",
            f2s(joystick_min_deadzone)
        )
    );
    file->add(
        new data_node(
            "leaving_confirmation_mode",
            i2s(leaving_confirmation_mode)
        )
    );
    file->add(
        new data_node(
            "max_particles",
            i2s(max_particles)
        )
    );
    file->add(
        new data_node(
            "middle_zoom_level",
            f2s(zoom_mid_level)
        )
    );
    file->add(
        new data_node(
            "mipmaps",
            b2s(mipmaps_enabled)
        )
    );
    file->add(
        new data_node(
            "resolution",
            i2s(intended_win_w) + " " +
            i2s(intended_win_h)
        )
    );
    file->add(
        new data_node(
            "smooth_scaling",
            b2s(smooth_scaling)
        )
    );
    file->add(
        new data_node(
            "show_hud_controls",
            b2s(show_hud_controls)
        )
    );
    file->add(
        new data_node(
            "true_fullscreen",
            b2s(true_fullscreen)
        )
    );
    file->add(
        new data_node(
            "window_position_hack",
            b2s(window_position_hack)
        )
    );
}
