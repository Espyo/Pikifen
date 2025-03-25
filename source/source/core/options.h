/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the game options class and related functions.
 */

#pragma once

#include <cstddef>
#include <map>
#include <vector>

#include "../game_state/area_editor/editor.h"
#include "../lib/data_file/data_file.h"
#include "const.h"
#include "controls_mediator.h"


using std::map;
using std::size_t;
using std::vector;


//Modes for the auto-throw feature.
enum AUTO_THROW_MODE {

    //Off.
    AUTO_THROW_MODE_OFF,
    
    //Hold input to auto-throw.
    AUTO_THROW_MODE_HOLD,
    
    //Press input to toggle auto-throw.
    AUTO_THROW_MODE_TOGGLE,
    
    //Total amount of auto-throw modes.
    N_AUTO_THROW_MODES,
    
};


//Modes for the pause menu leaving confirmation question.
enum LEAVING_CONF_MODE {

    //Never ask.
    LEAVING_CONF_MODE_NEVER,
    
    //Ask if it's been over a minute of gameplay.
    LEAVING_CONF_MODE_1_MIN,
    
    //Always ask.
    LEAVING_CONF_MODE_ALWAYS,
    
    //Total amount of leaving confirmation modes.
    N_LEAVING_CONF_MODES,
    
};


namespace OPTIONS {

namespace ADVANCED_D {
extern const bool DRAW_CURSOR_TRAIL;
extern const bool ENGINE_DEV;
extern const float JOYSTICK_MAX_DEADZONE;
extern const float JOYSTICK_MIN_DEADZONE;
extern const size_t MAX_PARTICLES;
extern const bool MIPMAPS_ENABLED;
extern const bool MOUSE_MOVES_CURSOR[MAX_PLAYERS];
extern const bool SMOOTH_SCALING;
extern const unsigned int TARGET_FPS;
extern const bool WINDOW_POS_HACK;
extern const float ZOOM_MID_LEVEL;
}

namespace AUDIO_D {
extern const float AMBIANCE_SOUND_VOl;
extern const float GAMEPLAY_SOUND_VOL;
extern const float MASTER_VOL;
extern const float MUSIC_VOL;
extern const float UI_SOUND_VOL;
}

namespace ANIM_ED_D {
extern const char* BG_PATH;
}

namespace AREA_ED_D {
extern const bool ADVANCED_MODE;
extern const float BACKUP_INTERVAL;
extern const float GRID_INTERVAL;
extern const bool SEL_TRANS;
extern const bool SHOW_CIRCULAR_INFO;
extern const bool SHOW_EDGE_LENGTH;
extern const bool SHOW_PATH_LINK_LENGTH;
extern const bool SHOW_TERRITORY;
extern const AreaEditor::SNAP_MODE SNAP_MODE;
extern const size_t SNAP_THRESHOLD;
extern const size_t UNDO_LIMIT;
extern const AreaEditor::VIEW_MODE VIEW_MODE;
}

namespace CONTROLS_D {
extern const AUTO_THROW_MODE AUTO_THROW;
extern const float CURSOR_SPEED;
}

namespace EDITORS_D {
extern const ALLEGRO_COLOR HIGHLIGHT_COLOR;
extern const bool MMB_PAN;
extern const float MOUSE_DRAG_THRESHOLD;
extern const ALLEGRO_COLOR PRIMARY_COLOR;
extern const ALLEGRO_COLOR SECONDARY_COLOR;
extern const bool SHOW_TOOLTIPS;
extern const ALLEGRO_COLOR TEXT_COLOR;
extern const bool USE_CUSTOM_STYLE;
}

namespace GRAPHICS_D {
extern const bool TRUE_FULLSCREEN;
extern const bool WIN_FULLSCREEN;
extern const unsigned int WIN_H;
extern const unsigned int WIN_W;
}

namespace GUI_ED_D {
extern const float GRID_INTERVAL;
extern const bool SNAP;
}

namespace MISC_D {
extern const float CURSOR_CAM_WEIGHT;
extern const LEAVING_CONF_MODE LEAVING_CONF;
extern const bool SHOW_HUD_INPUT_ICONS;
}

namespace PART_ED_D {
extern const char* BG_PATH;
extern const float GRID_INTERVAL;
}

}


using namespace OPTIONS;

/**
 * @brief Game options.
 */
struct Options {

    //--- Members ---
    
    //Advanced. These typicall don't appear in any options menu.
    struct {
    
        //Draw a trail behind the mouse cursor?
        bool draw_cursor_trail = ADVANCED_D::DRAW_CURSOR_TRAIL;
        
        //Is the player a developer of the engine?
        bool engine_dev = ADVANCED_D::ENGINE_DEV;
        
        //Maximum deadzone for joysticks.
        float joystick_max_deadzone = ADVANCED_D::JOYSTICK_MAX_DEADZONE;
        
        //Minimum deadzone for joysticks.
        float joystick_min_deadzone = ADVANCED_D::JOYSTICK_MIN_DEADZONE;
        
        //Maximum number of particles.
        size_t max_particles = ADVANCED_D::MAX_PARTICLES;
        
        //Enables or disables mipmaps.
        bool mipmaps_enabled = ADVANCED_D::MIPMAPS_ENABLED;
        
        //For each player, does the mouse move their leader's cursor?
        bool mouse_moves_cursor[MAX_PLAYERS] = {
            ADVANCED_D::MOUSE_MOVES_CURSOR[0],
            ADVANCED_D::MOUSE_MOVES_CURSOR[1],
            ADVANCED_D::MOUSE_MOVES_CURSOR[2],
            ADVANCED_D::MOUSE_MOVES_CURSOR[3]
        };
        
        //True to use interpolation when graphics are scaled up/down.
        bool smooth_scaling = ADVANCED_D::SMOOTH_SCALING;
        
        //Target framerate.
        int target_fps = ADVANCED_D::TARGET_FPS;
        
        //Should we force the window's positioning?
        //(on some machines it appears out-of-bounds by default)
        bool window_pos_hack = ADVANCED_D::WINDOW_POS_HACK;
        
        //Set the camera's middle zoom level to this amount.
        float zoom_mid_level = ADVANCED_D::ZOOM_MID_LEVEL;
        
    } advanced;
    
    //Animation editor.
    struct {
    
        //Background texture for the animation editor, if any.
        string bg_path = ANIM_ED_D::BG_PATH;
        
        //History for the last content entries that were opened.
        //Saves pairs of path-name.
        vector<pair<string, string> > history;
        
    } anim_editor;
    
    //Area editor.
    struct {
    
        //Use the advanced interface mode in the area editor?
        bool advanced_mode = AREA_ED_D::ADVANCED_MODE;
        
        //Backup the area in the area editor every X seconds.
        float backup_interval = AREA_ED_D::BACKUP_INTERVAL;
        
        //Grid interval in the area editor, in units.
        float grid_interval = AREA_ED_D::GRID_INTERVAL;
        
        //History for the last content entries that were opened.
        //Saves pairs of path-name.
        vector<pair<string, string> > history;
        
        //Can the user transform the selected vertexes?
        bool sel_trans = AREA_ED_D::SEL_TRANS;
        
        //Show info of a circular sector that's being drawn?
        bool show_circular_info = AREA_ED_D::SHOW_CIRCULAR_INFO;
        
        //Show the length of an edge that's being drawn/moved?
        bool show_edge_length = AREA_ED_D::SHOW_EDGE_LENGTH;
        
        //Show the length of a path link that's being drawn/moved?
        bool show_path_link_length = AREA_ED_D::SHOW_PATH_LINK_LENGTH;
        
        //Show the selected mob(s)'s territory?
        bool show_territory = AREA_ED_D::SHOW_TERRITORY;
        
        //Snap mode to use.
        AreaEditor::SNAP_MODE snap_mode = AREA_ED_D::SNAP_MODE;
        
        //Snap when the cursor is this close to a vertex/edge.
        size_t snap_threshold = AREA_ED_D::SNAP_THRESHOLD;
        
        //Maximum number of undo operations.
        size_t undo_limit = AREA_ED_D::UNDO_LIMIT;
        
        //View mode to use.
        AreaEditor::VIEW_MODE view_mode = AREA_ED_D::VIEW_MODE;
        
    } area_editor;
    
    //Audio.
    struct {
    
        //Ambiance sound volume (0 - 1).
        float ambiance_sound_vol = AUDIO_D::AMBIANCE_SOUND_VOl;
        
        //Gameplay sound effects volume (0 - 1).
        float gameplay_sound_vol = AUDIO_D::GAMEPLAY_SOUND_VOL;
        
        //Master sound volume (0 - 1).
        float master_vol = AUDIO_D::MASTER_VOL;
        
        //Music volume (0 - 1).
        float music_vol = AUDIO_D::MUSIC_VOL;
        
        //UI sound effects volume (0 - 1).
        float ui_sound_vol = AUDIO_D::UI_SOUND_VOL;
        
    } audio;
    
    //Controls. These do not include control binds.
    struct {
    
        //Auto-throw mode.
        AUTO_THROW_MODE auto_throw_mode = CONTROLS_D::AUTO_THROW;
        
        //Cursor speed, in pixels per second. N/A when using the mouse.
        float cursor_speed = CONTROLS_D::CURSOR_SPEED;
        
    } controls;
    
    //Editors in general.
    struct {
    
        //Editor's custom style highlight color.
        ALLEGRO_COLOR highlight_color = EDITORS_D::HIGHLIGHT_COLOR;
        
        //If true, the middle mouse button pans in editors.
        bool mmb_pan = EDITORS_D::MMB_PAN;
        
        //In editors, only consider a mouse drag if it moves these many pixels.
        float mouse_drag_threshold = EDITORS_D::MOUSE_DRAG_THRESHOLD;
        
        //List of which editor node widgets the user wants open.
        map<string, bool> open_nodes;
        
        //Editor's custom style main reference color.
        ALLEGRO_COLOR primary_color = EDITORS_D::PRIMARY_COLOR;
        
        //Editor's custom style accent reference color.
        ALLEGRO_COLOR secondary_color = EDITORS_D::SECONDARY_COLOR;
        
        //In editors, show widget tooltips when the mouse is over them.
        bool show_tooltips = EDITORS_D::SHOW_TOOLTIPS;
        
        //Editor's custom style text color.
        ALLEGRO_COLOR text_color = EDITORS_D::TEXT_COLOR;
        
        //Should the editors use a custom style, or the default?
        bool use_custom_style = EDITORS_D::USE_CUSTOM_STYLE;
        
    } editors;
    
    //Graphics.
    struct {
    
        //Player's intended option for fullscreen, before restarting the game.
        bool intended_win_fullscreen = GRAPHICS_D::WIN_FULLSCREEN;
        
        //Player's intended option for window height, before restarting the game.
        int intended_win_h = GRAPHICS_D::WIN_H;
        
        //Player's intended option for window width, before restarting the game.
        int intended_win_w = GRAPHICS_D::WIN_W;
        
        //When using fullscreen, is this true fullscreen, or borderless window?
        bool true_fullscreen = GRAPHICS_D::TRUE_FULLSCREEN;
        
    } graphics;
    
    //GUI editor.
    struct {
    
        //Grid interval in the GUI editor, in units.
        float grid_interval = GUI_ED_D::GRID_INTERVAL;
        
        //History for the last content entries that were opened.
        //Saves pairs of path-name.
        vector<pair<string, string> > history;
        
        //Snap to grid?
        bool snap = GUI_ED_D::SNAP;
        
    } gui_editor;
    
    //Misc.
    struct {
    
        //Cursor camera movement weight.
        float cursor_cam_weight = MISC_D::CURSOR_CAM_WEIGHT;
        
        //Pause menu leaving confirmation question mode.
        LEAVING_CONF_MODE leaving_conf_mode = MISC_D::LEAVING_CONF;
        
        //Show control bind icons on top of HUD elements?
        bool show_hud_input_icons = MISC_D::SHOW_HUD_INPUT_ICONS;
        
    } misc;
    
    //Packs.
    struct {
    
        //Disabled packs.
        vector<string> disabled;
        
        //Preferred pack load order.
        vector<string> order;
        
    } packs;
    
    //Particle editor.
    struct {
    
        //Background texture for the particle editor, if any.
        string bg_path = PART_ED_D::BG_PATH;
        
        //Grid interval in the particle editor, in units.
        float grid_interval = PART_ED_D::GRID_INTERVAL;
        
        //History for the last content entries that were opened.
        //Saves pairs of path-name.
        vector<pair<string, string> > history;
        
    } particle_editor;
    
    
    //--- Function declarations ---
    
    void load_from_data_node(DataNode* file);
    void save_to_data_node(DataNode* file) const;
    
    
private:

    //--- Function declarations ---
    
    vector<pair<string, string> > load_editor_history(const string &str) const;
    string save_editor_history(const vector<pair<string, string> > &vec) const;
};
