/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Game options class and related functions.
 */

#include "options.h"

//Default values for the different options.
const float options_struct::DEF_AREA_EDITOR_BACKUP_INTERVAL = 120.0f;
const float options_struct::DEF_AREA_EDITOR_GRID_INTERVAL = 32.0f;
const bool options_struct::DEF_AREA_EDITOR_SHOW_EDGE_LENGTH = true;
const bool options_struct::DEF_AREA_EDITOR_SHOW_TERRITORY = false;
const size_t options_struct::DEF_AREA_EDITOR_SNAP_THRESHOLD = 80;
const size_t options_struct::DEF_AREA_EDITOR_UNDO_LIMIT = 20;
const unsigned char options_struct::DEF_AREA_EDITOR_VIEW_MODE = 0;
const bool options_struct::DEF_DRAW_CURSOR_TRAIL = true;
const bool options_struct::DEF_EDITOR_MMB_PAN = false;
const float options_struct::DEF_EDITOR_MOUSE_DRAG_THRESHOLD = 4;
const float options_struct::DEF_JOYSTICK_MAX_DEADZONE = 0.9f;
const float options_struct::DEF_JOYSTICK_MIN_DEADZONE = 0.2f;
const size_t options_struct::DEF_MAX_PARTICLES = 200;
const bool options_struct::DEF_MIPMAPS_ENABLED = true;
const bool options_struct::DEF_MOUSE_MOVES_CURSOR[MAX_PLAYERS] =
{true, false, false, false};
const bool options_struct::DEF_PRETTY_WHISTLE = true;
const bool options_struct::DEF_SMOOTH_SCALING = true;
const unsigned int options_struct::DEF_TARGET_FPS = 60;
const bool options_struct::DEF_WIN_FULLSCREEN = false;
const unsigned int options_struct::DEF_WIN_H = 768;
const bool options_struct::DEF_WINDOW_POSITION_HACK = false;
const unsigned int options_struct::DEF_WIN_W = 1024;
const float options_struct::DEF_ZOOM_MID_LEVEL = 1.4f;

options_struct::options_struct() :
    area_editor_backup_interval(DEF_AREA_EDITOR_BACKUP_INTERVAL),
    area_editor_grid_interval(DEF_AREA_EDITOR_GRID_INTERVAL),
    area_editor_show_edge_length(DEF_AREA_EDITOR_SHOW_EDGE_LENGTH),
    area_editor_show_territory(DEF_AREA_EDITOR_SHOW_TERRITORY),
    area_editor_snap_threshold(DEF_AREA_EDITOR_SNAP_THRESHOLD),
    area_editor_undo_limit(DEF_AREA_EDITOR_UNDO_LIMIT),
    area_editor_view_mode(DEF_AREA_EDITOR_VIEW_MODE),
    draw_cursor_trail(DEF_DRAW_CURSOR_TRAIL),
    editor_mmb_pan(DEF_EDITOR_MMB_PAN),
    editor_mouse_drag_threshold(DEF_EDITOR_MOUSE_DRAG_THRESHOLD),
    intended_win_fullscreen(DEF_WIN_FULLSCREEN),
    intended_win_h(DEF_WIN_H),
    intended_win_w(DEF_WIN_W),
    joystick_max_deadzone(DEF_JOYSTICK_MAX_DEADZONE),
    joystick_min_deadzone(DEF_JOYSTICK_MIN_DEADZONE),
    max_particles(DEF_MAX_PARTICLES),
    mipmaps_enabled(DEF_MIPMAPS_ENABLED),
    pretty_whistle(DEF_PRETTY_WHISTLE),
    smooth_scaling(DEF_SMOOTH_SCALING),
    target_fps(DEF_TARGET_FPS),
    window_position_hack(DEF_WINDOW_POSITION_HACK),
    zoom_mid_level(DEF_ZOOM_MID_LEVEL) {
    
    mouse_moves_cursor[0] = DEF_MOUSE_MOVES_CURSOR[0];
    mouse_moves_cursor[1] = DEF_MOUSE_MOVES_CURSOR[1];
    mouse_moves_cursor[2] = DEF_MOUSE_MOVES_CURSOR[2];
    mouse_moves_cursor[3] = DEF_MOUSE_MOVES_CURSOR[3];
}
