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
extern const bool HIDE_STOPPED_MOUSE_CURSOR;
extern const float JOYSTICK_MAX_DEADZONE;
extern const float JOYSTICK_MIN_DEADZONE;
extern const size_t MAX_PARTICLES;
extern const bool MIPMAPS_ENABLED;
extern const bool SMOOTH_SCALING;
extern const unsigned int TARGET_FPS;
extern const bool WINDOW_POS_HACK;
extern const float ZOOM_MEDIUM_REACH;
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
extern const float LEADER_CURSOR_SPEED;
extern const bool MOUSE_MOVES_LEADER_CURSOR[MAX_PLAYERS];
extern const bool MOUSE_MOVES_LEADER[MAX_PLAYERS];
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
extern const float CAM_SHAKE_MULT;
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
extern const float LEADER_CURSOR_CAM_WEIGHT;
extern const bool DISMISS_ALL;
extern const LEAVING_CONF_MODE LEAVING_CONF;
extern const bool MAKER_TOOLS_IN_PLAY;
extern const float PIKMIN_BUMP_DIST;
extern const bool SHOW_LEADER_CURSOR_COUNTER;
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
    
    //Advanced. These typically don't appear in any options menu.
    struct {
    
        //Draw a trail behind the mouse cursor?
        bool drawCursorTrail = ADVANCED_D::DRAW_CURSOR_TRAIL;
        
        //Is the player a developer of the engine?
        bool engineDev = ADVANCED_D::ENGINE_DEV;
        
        //Should the mouse cursor be hidden when stopped for some seconds?
        bool hideStoppedMouseCursor = ADVANCED_D::HIDE_STOPPED_MOUSE_CURSOR;
        
        //Maximum deadzone for joysticks.
        float joystickMaxDeadzone = ADVANCED_D::JOYSTICK_MAX_DEADZONE;
        
        //Minimum deadzone for joysticks.
        float joystickMinDeadzone = ADVANCED_D::JOYSTICK_MIN_DEADZONE;
        
        //Maximum number of particles.
        size_t maxParticles = ADVANCED_D::MAX_PARTICLES;
        
        //Enables or disables mipmaps.
        bool mipmapsEnabled = ADVANCED_D::MIPMAPS_ENABLED;
        
        //True to use interpolation when graphics are scaled up/down.
        bool smoothScaling = ADVANCED_D::SMOOTH_SCALING;
        
        //Target framerate.
        int targetFps = ADVANCED_D::TARGET_FPS;
        
        //Should we force the window's positioning?
        //(on some machines it appears out-of-bounds by default)
        bool windowPosHack = ADVANCED_D::WINDOW_POS_HACK;
        
        //Set the camera's middle zoom level to this amount.
        float zoomMediumReach = ADVANCED_D::ZOOM_MEDIUM_REACH;
        
    } advanced;
    
    //Animation editor.
    struct {
    
        //Background texture for the animation editor, if any.
        string bgPath = ANIM_ED_D::BG_PATH;
        
        //History for the last content entries that were opened.
        //Saves pairs of path-name.
        vector<pair<string, string> > history;
        
    } animEd;
    
    //Area editor.
    struct {
    
        //Use the advanced interface mode in the area editor?
        bool advancedMode = AREA_ED_D::ADVANCED_MODE;
        
        //Backup the area in the area editor every X seconds.
        float backupInterval = AREA_ED_D::BACKUP_INTERVAL;
        
        //Grid interval in the area editor, in units.
        float gridInterval = AREA_ED_D::GRID_INTERVAL;
        
        //History for the last content entries that were opened.
        //Saves pairs of path-name.
        vector<pair<string, string> > history;
        
        //Can the user transform the selected vertexes?
        bool selTrans = AREA_ED_D::SEL_TRANS;
        
        //Show info of a circular sector that's being drawn?
        bool showCircularInfo = AREA_ED_D::SHOW_CIRCULAR_INFO;
        
        //Show the length of an edge that's being drawn/moved?
        bool showEdgeLength = AREA_ED_D::SHOW_EDGE_LENGTH;
        
        //Show the length of a path link that's being drawn/moved?
        bool showPathLinkLength = AREA_ED_D::SHOW_PATH_LINK_LENGTH;
        
        //Show the selected mob(s)'s territory?
        bool showTerritory = AREA_ED_D::SHOW_TERRITORY;
        
        //Snap mode to use.
        AreaEditor::SNAP_MODE snapMode = AREA_ED_D::SNAP_MODE;
        
        //Snap when the mouse cursor is this close to a vertex/edge.
        size_t snapThreshold = AREA_ED_D::SNAP_THRESHOLD;
        
        //Maximum number of undo operations.
        size_t undoLimit = AREA_ED_D::UNDO_LIMIT;
        
        //View mode to use.
        AreaEditor::VIEW_MODE viewMode = AREA_ED_D::VIEW_MODE;
        
    } areaEd;
    
    //Audio.
    struct {
    
        //Ambiance sound volume [0 - 1].
        float ambianceSoundVol = AUDIO_D::AMBIANCE_SOUND_VOl;
        
        //Gameplay sound effects volume [0 - 1].
        float gameplaySoundVol = AUDIO_D::GAMEPLAY_SOUND_VOL;
        
        //Master sound volume [0 - 1].
        float masterVol = AUDIO_D::MASTER_VOL;
        
        //Music volume [0 - 1].
        float musicVol = AUDIO_D::MUSIC_VOL;
        
        //UI sound effects volume [0 - 1].
        float uiSoundVol = AUDIO_D::UI_SOUND_VOL;
        
    } audio;
    
    //Controls. These do not include control binds.
    struct {
    
        //Auto-throw mode.
        AUTO_THROW_MODE autoThrowMode = CONTROLS_D::AUTO_THROW;
        
        //Leader cursor speed, in pixels per second. N/A when using the mouse.
        float leaderCursorSpeed = CONTROLS_D::LEADER_CURSOR_SPEED;
        
        //For each player, does the mouse move their leader's cursor?
        bool mouseMovesLeaderCursor[MAX_PLAYERS] = {
            CONTROLS_D::MOUSE_MOVES_LEADER_CURSOR[0],
            CONTROLS_D::MOUSE_MOVES_LEADER_CURSOR[1],
            CONTROLS_D::MOUSE_MOVES_LEADER_CURSOR[2],
            CONTROLS_D::MOUSE_MOVES_LEADER_CURSOR[3]
        };
        
        //For each player, does the mouse move their leader?
        bool mouseMovesLeader[MAX_PLAYERS] = {
            CONTROLS_D::MOUSE_MOVES_LEADER[0],
            CONTROLS_D::MOUSE_MOVES_LEADER[1],
            CONTROLS_D::MOUSE_MOVES_LEADER[2],
            CONTROLS_D::MOUSE_MOVES_LEADER[3]
        };
        
    } controls;
    
    //Editors in general.
    struct {
    
        //Editor's custom style highlight color.
        ALLEGRO_COLOR highlightColor = EDITORS_D::HIGHLIGHT_COLOR;
        
        //If true, the middle mouse button pans in editors.
        bool mmbPan = EDITORS_D::MMB_PAN;
        
        //In editors, only consider a mouse drag if it moves these many pixels.
        float mouseDragThreshold = EDITORS_D::MOUSE_DRAG_THRESHOLD;
        
        //List of which editor node widgets the user wants open.
        map<string, bool> openNodes;
        
        //Editor's custom style main reference color.
        ALLEGRO_COLOR primaryColor = EDITORS_D::PRIMARY_COLOR;
        
        //Editor's custom style accent reference color.
        ALLEGRO_COLOR secondaryColor = EDITORS_D::SECONDARY_COLOR;
        
        //In editors, show widget tooltips when the mouse is over them.
        bool showTooltips = EDITORS_D::SHOW_TOOLTIPS;
        
        //Editor's custom style text color.
        ALLEGRO_COLOR textColor = EDITORS_D::TEXT_COLOR;
        
        //Should the editors use a custom style, or the default?
        bool useCustomStyle = EDITORS_D::USE_CUSTOM_STYLE;
        
    } editors;
    
    //Graphics.
    struct {
    
        //Camera shake strength multiplier.
        float camShakeMult = GRAPHICS_D::CAM_SHAKE_MULT;
        
        //Player's intended option for fullscreen, before restarting the game.
        bool intendedWinFullscreen = GRAPHICS_D::WIN_FULLSCREEN;
        
        //Player's intended option for window height,
        //before restarting the game.
        int intendedWinH = GRAPHICS_D::WIN_H;
        
        //Player's intended option for window width,
        //before restarting the game.
        int intendedWinW = GRAPHICS_D::WIN_W;
        
        //When using fullscreen, is this true fullscreen, or borderless window?
        bool trueFullscreen = GRAPHICS_D::TRUE_FULLSCREEN;
        
    } graphics;
    
    //GUI editor.
    struct {
    
        //Grid interval in the GUI editor, in units.
        float gridInterval = GUI_ED_D::GRID_INTERVAL;
        
        //History for the last content entries that were opened.
        //Saves pairs of path-name.
        vector<pair<string, string> > history;
        
        //Snap to grid?
        bool snap = GUI_ED_D::SNAP;
        
    } guiEd;
    
    //Misc.
    struct {
    
        //Dismissing dismisses all.
        bool dismissAll = OPTIONS::MISC_D::DISMISS_ALL;
        
        //Leader cursor camera movement weight.
        float leaderCursorCamWeight = OPTIONS::MISC_D::LEADER_CURSOR_CAM_WEIGHT;
        
        //Pause menu leaving confirmation question mode.
        LEAVING_CONF_MODE leavingConfMode = OPTIONS::MISC_D::LEAVING_CONF;
        
        //Whether to freely allow maker tools in the play mode.
        bool makerToolsInPlay = false;
        
        //Idle Pikmin leader bumping distance.
        float pikminBumpDist = OPTIONS::MISC_D::PIKMIN_BUMP_DIST;
        
        //Show a standby type counter on the leader cursor?
        bool showLeaderCursorCounter = OPTIONS::MISC_D::SHOW_LEADER_CURSOR_COUNTER;
        
        //Show control bind icons on top of HUD elements?
        bool showHudInputIcons = OPTIONS::MISC_D::SHOW_HUD_INPUT_ICONS;
        
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
        string bgPath = PART_ED_D::BG_PATH;
        
        //Grid interval in the particle editor, in units.
        float gridInterval = PART_ED_D::GRID_INTERVAL;
        
        //History for the last content entries that were opened.
        //Saves pairs of path-name.
        vector<pair<string, string> > history;
        
    } partEd;
    
    
    //--- Function declarations ---
    
    void loadFromDataNode(DataNode* file);
    void saveToDataNode(DataNode* file) const;
    
    
private:

    //--- Function declarations ---
    
    vector<pair<string, string> > loadEditorHistory(const string& str) const;
    string saveEditorHistory(const vector<pair<string, string> >& vec) const;
};
