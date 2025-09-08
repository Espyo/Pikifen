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

#include "../util/allegro_utils.h"
#include "../util/string_utils.h"
#include "game.h"
#include "misc_functions.h"
#include "misc_structs.h"


namespace OPTIONS {

namespace ADVANCED_D {

//Default value for the cursor trail.
const bool DRAW_CURSOR_TRAIL = true;

//Default value for whether the player is an engine developer.
const bool ENGINE_DEV = false;

//Default value for the joystick maximum deadzone.
const float JOYSTICK_MAX_DEADZONE = 0.9f;

//Default value for the joystick minimum deadzone.
const float JOYSTICK_MIN_DEADZONE = 0.2f;

//Default value for the maximum amount of particles.
const size_t MAX_PARTICLES = 1000;

//Default value for whether mipmaps are enabled.
const bool MIPMAPS_ENABLED = true;

//Default value for whether the mouse moves the cursor, for each player.
const bool MOUSE_MOVES_CURSOR[MAX_PLAYERS] = {true, false, false, false};

//Default value for whether to use smooth scaling.
const bool SMOOTH_SCALING = true;

//Default value for the default target framerate.
const unsigned int TARGET_FPS = 60;

//Default value for whether to use the window position hack.
const bool WINDOW_POS_HACK = false;

//Default value for the zoom medium reach.
const float ZOOM_MEDIUM_REACH = 630.0f;

}


namespace AUDIO_D {

//Default value for the ambiance sound volume.
const float AMBIANCE_SOUND_VOl = 1.0f;

//Default value for gameplay sound effects volume.
const float GAMEPLAY_SOUND_VOL = 1.0f;

//Default value for the master sound volume.
const float MASTER_VOL = 0.8f;

//Default value for the music volume.
const float MUSIC_VOL = 1.0f;

//Default value for UI sound effects volume.
const float UI_SOUND_VOL = 1.0f;

}


namespace ANIM_ED_D {

//Default value for the animation editor background texture.
const char* BG_PATH = "";

}


namespace AREA_ED_D {

//Default value for the area editor advanced mode setting.
const bool ADVANCED_MODE = false;

//Default value for the area editor backup interval.
const float BACKUP_INTERVAL = 120.0f;

//Default value for the area editor grid interval.
const float GRID_INTERVAL = 32.0f;

//Default value for the area editor selection transformation widget.
const bool SEL_TRANS = false;

//Default value for whether to show a circular sector's info in the area editor.
const bool SHOW_CIRCULAR_INFO = true;

//Default value for whether to show an edge's length in the area editor.
const bool SHOW_EDGE_LENGTH = true;

//Default value for whether to show a path link's length in the area editor.
const bool SHOW_PATH_LINK_LENGTH = true;

//Default value for whether to show a mob's territory in the area editor.
const bool SHOW_TERRITORY = false;

//Default value for the area editor snap mode.
const AreaEditor::SNAP_MODE SNAP_MODE =
    AreaEditor::SNAP_MODE_GRID;
    
//Default value for the area editor snap threshold.
const size_t SNAP_THRESHOLD = 80;

//Default value for the area editor undo limit.
const size_t UNDO_LIMIT = 20;

//Default value for the area editor view mode.
const AreaEditor::VIEW_MODE VIEW_MODE =
    AreaEditor::VIEW_MODE_TEXTURES;
    
}


namespace CONTROLS_D {

//Default value for the auto-throw mode.
const AUTO_THROW_MODE AUTO_THROW = AUTO_THROW_MODE_OFF;

//Default value for the leader cursor speed.
const float LEADER_CURSOR_SPEED = 500.0f;

}


namespace EDITORS_D {

//Default value for the editor highlights.
const ALLEGRO_COLOR HIGHLIGHT_COLOR = { 1.0f, 1.0f, 1.0f, 1.0f };

//Default value for whether the middle mouse button pans in editors.
const bool MMB_PAN = false;

//Default value for the editor mouse drag threshold.
const float MOUSE_DRAG_THRESHOLD = 4;

//Default value for the editor primary color.
const ALLEGRO_COLOR PRIMARY_COLOR = {0.05f, 0.05f, 0.05f, 1.0f};

//Default value for the editor secondary color.
const ALLEGRO_COLOR SECONDARY_COLOR = {0.19f, 0.47f, 0.78f, 1.0f};

//Default value for whether to show tooltips in editors.
const bool SHOW_TOOLTIPS = true;

//Default value for the editor text color.
const ALLEGRO_COLOR TEXT_COLOR = {1.0f, 1.0f, 1.0f, 1.0f};

//Default value for whether to use custom styles in editors.
const bool USE_CUSTOM_STYLE = false;

}


namespace GRAPHICS_D {

//Default value for the camera shake multiplier.
const float CAM_SHAKE_MULT = 1.0f;

//Default value for whether to use true fullscreen.
const bool TRUE_FULLSCREEN = false;

//Default value for whether to use fullscreen.
const bool WIN_FULLSCREEN = false;

//Default value for the window height.
const unsigned int WIN_H = 768;

//Default value for the window width.
const unsigned int WIN_W = 1024;

}


namespace GUI_ED_D {

//Default value for the GUI editor grid interval.
const float GRID_INTERVAL = 2.5f;

//Default value for the GUI editor snap mode.
const bool SNAP = true;

}


namespace MISC_D {

//Default value for the dismiss dismissing all.
const bool DISMISS_ALL = true;

//Default value for the leader cursor camera weight.
const float LEADER_CURSOR_CAM_WEIGHT = 0.0f;

//Default value for the pause menu leaving confirmation mode.
const LEAVING_CONF_MODE LEAVING_CONF = LEAVING_CONF_MODE_ALWAYS;

//Default value for whether to freely allow maker tools in play mode.
const bool MAKER_TOOLS_IN_PLAY = false;

//Default value for the Pikmin bumping distance.
const float PIKMIN_BUMP_DIST = 50.0f;

//Default value for whether to show the standby type counter on the leader cursor.
const bool SHOW_LEADER_CURSOR_COUNTER = false;

//Default value for whether to show player input icons on the HUD.
const bool SHOW_HUD_INPUT_ICONS = true;

}


namespace PART_ED_D {

//Default value for the particle editor background texture.
const char* BG_PATH = "";

//Default value for the area editor grid interval.
const float GRID_INTERVAL = 32.0f;

}


}


/**
 * @brief Loads an editor's history from a string in the options file.
 *
 * @param str The string.
 * @return Vector with the history.
 */
vector<pair<string, string> > Options::loadEditorHistory(
    const string& str
) const {
    vector<pair<string, string> > result;
    vector<string> parts = semicolonListToVector(str);
    for(size_t e = 0; e < parts.size(); e += 2) {
        result.push_back(make_pair(parts[e], parts[e + 1]));
    }
    return result;
}


/**
 * @brief Loads the player options from a file.
 *
 * @param file File to read from.
 */
void Options::loadFromDataNode(DataNode* file) {
    //Advanced.
    {
        ReaderSetter aRS(file->getChildByName("advanced"));
        
        aRS.set("draw_cursor_trail", advanced.drawCursorTrail);
        aRS.set("engine_developer", advanced.engineDev);
        aRS.set("fps", advanced.targetFps);
        aRS.set("joystick_max_deadzone", advanced.joystickMaxDeadzone);
        aRS.set("joystick_min_deadzone", advanced.joystickMinDeadzone);
        aRS.set("max_particles", advanced.maxParticles);
        aRS.set("mipmaps", advanced.mipmapsEnabled);
        for(unsigned char p = 0; p < MAX_PLAYERS; p++) {
            aRS.set(
                "p" + i2s((p + 1)) + "_mouse_moves_cursor",
                advanced.mouseMovesCursor[p]
            );
        }
        aRS.set("smooth_scaling", advanced.smoothScaling);
        aRS.set("window_position_hack", advanced.windowPosHack);
        aRS.set("zoom_medium_reach", advanced.zoomMediumReach);
        
        advanced.targetFps = std::max(LOWEST_FRAMERATE, advanced.targetFps);
        
        if(advanced.joystickMinDeadzone > advanced.joystickMaxDeadzone) {
            std::swap(
                advanced.joystickMinDeadzone, advanced.joystickMaxDeadzone
            );
        }
        if(advanced.joystickMinDeadzone == advanced.joystickMaxDeadzone) {
            advanced.joystickMinDeadzone -= 0.1;
            advanced.joystickMaxDeadzone += 0.1;
        }
        advanced.joystickMinDeadzone =
            std::clamp(advanced.joystickMinDeadzone, 0.0f, 1.0f);
        advanced.joystickMaxDeadzone =
            std::clamp(advanced.joystickMaxDeadzone, 0.0f, 1.0f);
    }
    
    //Animation editor.
    {
        ReaderSetter aRS(file->getChildByName("animation_editor"));
        
        string historyStr;
        
        aRS.set("bg_path", animEd.bgPath);
        aRS.set("history", historyStr);
        
        animEd.history = loadEditorHistory(historyStr);
    }
    
    //Area editor.
    {
        ReaderSetter aRS(file->getChildByName("area_editor"));
        
        string historyStr;
        unsigned char snapModeChar = areaEd.snapMode;
        unsigned char viewModeChar = areaEd.viewMode;
        
        aRS.set("advanced_mode", areaEd.advancedMode);
        aRS.set("backup_interval", areaEd.backupInterval);
        aRS.set("grid_interval", areaEd.gridInterval);
        aRS.set("history", historyStr);
        aRS.set("selection_transformation", areaEd.selTrans);
        aRS.set("show_circular_info", areaEd.showCircularInfo);
        aRS.set("show_edge_length", areaEd.showEdgeLength);
        aRS.set("show_path_link_length", areaEd.showPathLinkLength);
        aRS.set("show_territory", areaEd.showTerritory);
        aRS.set("snap_mode", snapModeChar);
        aRS.set("snap_threshold", areaEd.snapThreshold);
        aRS.set("undo_limit", areaEd.undoLimit);
        aRS.set("view_mode", viewModeChar);
        
        areaEd.history = loadEditorHistory(historyStr);
        
        snapModeChar =
            std::min(
                snapModeChar, (unsigned char) (AreaEditor::N_SNAP_MODES - 1)
            );
        areaEd.snapMode = (AreaEditor::SNAP_MODE) snapModeChar;
        
        viewModeChar =
            std::min(
                viewModeChar, (unsigned char) (AreaEditor::N_VIEW_MODES - 1)
            );
        areaEd.viewMode = (AreaEditor::VIEW_MODE) viewModeChar;
    }
    
    //Audio.
    {
        ReaderSetter aRS(file->getChildByName("audio"));
        
        aRS.set("ambiance_sound_volume", audio.ambianceSoundVol);
        aRS.set("gameplay_sound_volume", audio.gameplaySoundVol);
        aRS.set("master_volume", audio.masterVol);
        aRS.set("music_volume", audio.musicVol);
        aRS.set("ui_sound_volume", audio.uiSoundVol);
        
        audio.ambianceSoundVol = std::clamp(audio.ambianceSoundVol, 0.0f, 1.0f);
        audio.gameplaySoundVol = std::clamp(audio.gameplaySoundVol, 0.0f, 1.0f);
        audio.masterVol = std::clamp(audio.masterVol, 0.0f, 1.0f);
        audio.musicVol = std::clamp(audio.musicVol, 0.0f, 1.0f);
        audio.uiSoundVol = std::clamp(audio.uiSoundVol, 0.0f, 1.0f);
    }
    
    //Control binds.
    {
        game.controls.binds().clear();
        for(unsigned char p = 0; p < MAX_PLAYERS; p++) {
            DataNode* bindsNode =
                file->getChildByName("control_binds_p" + i2s(p + 1));
            game.controls.loadBindsFromDataNode(bindsNode, p);
        }
    }
    
    //Controls.
    {
        ReaderSetter cRS(file->getChildByName("controls"));
        
        unsigned char autoThrowModeChar = controls.autoThrowMode;
        
        //DEPRECATED in 1.1.0 by "leader_cursor_speed".
        cRS.set("cursor_speed", controls.leaderCursorSpeed);

        cRS.set("auto_throw_mode", autoThrowModeChar);
        cRS.set("leader_cursor_speed", controls.leaderCursorSpeed);
        
        autoThrowModeChar =
            std::min(
                autoThrowModeChar, (unsigned char) (N_AUTO_THROW_MODES - 1)
            );
        controls.autoThrowMode = (AUTO_THROW_MODE) autoThrowModeChar;
    }
    
    //Editors.
    {
        ReaderSetter eRS(file->getChildByName("editors"));
        
        string openNodesStr;
        
        eRS.set("highlight_color", editors.highlightColor);
        eRS.set("mmb_pan", editors.mmbPan);
        eRS.set("mouse_drag_threshold", editors.mouseDragThreshold);
        eRS.set("open_nodes", openNodesStr);
        eRS.set("primary_color", editors.primaryColor);
        eRS.set("secondary_color", editors.secondaryColor);
        eRS.set("show_tooltips", editors.showTooltips);
        eRS.set("text_color", editors.textColor);
        eRS.set("use_custom_style", editors.useCustomStyle);
        
        editors.openNodes.clear();
        vector<string> openNodesVector =
            semicolonListToVector(openNodesStr);
        for(size_t n = 0; n < openNodesVector.size(); n++) {
            editors.openNodes[openNodesVector[n]] = true;
        }
        
        //Force the editor styles to be opaque, otherwise there can be problems.
        editors.primaryColor.a = 1.0f;
        editors.secondaryColor.a = 1.0f;
        editors.textColor.a = 1.0f;
        editors.highlightColor.a = 1.0f;
    }
    
    //Graphics.
    {
        ReaderSetter gRS(file->getChildByName("graphics"));
        
        string resolutionStr;
        
        gRS.set("cam_shake_mult", graphics.camShakeMult);
        gRS.set("fullscreen", graphics.intendedWinFullscreen);
        gRS.set("resolution", resolutionStr);
        gRS.set("true_fullscreen", graphics.trueFullscreen);
        
        vector<string> resolutionParts = split(resolutionStr);
        if(resolutionParts.size() >= 2) {
            graphics.intendedWinW = std::max(1, s2i(resolutionParts[0]));
            graphics.intendedWinH = std::max(1, s2i(resolutionParts[1]));
        }
        
        graphics.camShakeMult = std::clamp(graphics.camShakeMult, 0.0f, 1.0f);
        
    }
    
    //GUI editor.
    {
        ReaderSetter gRS(file->getChildByName("gui_editor"));
        
        string historyStr;
        
        gRS.set("grid_interval", guiEd.gridInterval);
        gRS.set("history", historyStr);
        gRS.set("snap", guiEd.snap);
        
        guiEd.history = loadEditorHistory(historyStr);
    }
    
    //Misc.
    {
        ReaderSetter mRS(file->getChildByName("misc"));
        
        unsigned char leavingConfModeChar = misc.leavingConfMode;
        
        //DEPRECATED in 1.1.0 by "leader_cursor_cam_weight".
        mRS.set("cursor_cam_weight", misc.leaderCursorCamWeight);
        //DEPRECATED in 1.1.0 by "show_leader_cursor_counter".
        mRS.set("show_counter_on_cursor", misc.showLeaderCursorCounter);

        mRS.set("dismiss_all", misc.dismissAll);
        mRS.set("leader_cursor_cam_weight", misc.leaderCursorCamWeight);
        mRS.set("leaving_confirmation_mode", leavingConfModeChar);
        mRS.set("maker_tools_in_play", misc.makerToolsInPlay);
        mRS.set("pikmin_bump_dist", misc.pikminBumpDist);
        mRS.set("show_leader_cursor_counter", misc.showLeaderCursorCounter);
        mRS.set("show_hud_input_icons", misc.showHudInputIcons);
        
        leavingConfModeChar =
            std::min(
                leavingConfModeChar,
                (unsigned char) (N_LEAVING_CONF_MODES - 1)
            );
        misc.leavingConfMode = (LEAVING_CONF_MODE) leavingConfModeChar;
        
    }
    
    //Packs.
    {
        ReaderSetter pRS(file->getChildByName("packs"));
        
        string packsDisabledStr;
        string packsLoadOrderStr;
        
        pRS.set("disabled", packsDisabledStr);
        pRS.set("order", packsLoadOrderStr);
        
        packs.disabled = semicolonListToVector(packsDisabledStr);
        packs.order = semicolonListToVector(packsLoadOrderStr);
    }
    
    //Particle editor.
    {
        ReaderSetter pRS(file->getChildByName("particle_editor"));
        
        string historyStr;
        
        pRS.set("bg_path", partEd.bgPath);
        pRS.set("grid_interval", partEd.gridInterval);
        pRS.set("history", historyStr);
        
        partEd.history = loadEditorHistory(historyStr);
    }
}


/**
 * @brief Saves an editor's history to a string in the options file.
 *
 * @param vec Vector with the history.
 * @return The string.
 */
string Options::saveEditorHistory(
    const vector<pair<string, string> >& vec
) const {
    vector<string> parts;
    for(size_t e = 0; e < vec.size(); e ++) {
        parts.push_back(vec[e].first);
        parts.push_back(vec[e].second);
    }
    return join(parts, ";");
}


/**
 * @brief Saves the player's options into a file.
 *
 * @param file File to write to.
 */
void Options::saveToDataNode(DataNode* file) const {
    //Advanced.
    {
        GetterWriter aGW(file->addNew("advanced"));
        
        aGW.write("draw_cursor_trail", advanced.drawCursorTrail);
        aGW.write("engine_developer", advanced.engineDev);
        aGW.write("fps", advanced.targetFps);
        aGW.write("joystick_max_deadzone", advanced.joystickMaxDeadzone);
        aGW.write("joystick_min_deadzone", advanced.joystickMinDeadzone);
        aGW.write("max_particles", advanced.maxParticles);
        aGW.write("mipmaps", advanced.mipmapsEnabled);
        for(unsigned char p = 0; p < MAX_PLAYERS; p++) {
            aGW.write(
                "p" + i2s((p + 1)) + "_mouse_moves_cursor",
                advanced.mouseMovesCursor[p]
            );
        }
        aGW.write("smooth_scaling", advanced.smoothScaling);
        aGW.write("window_position_hack", advanced.windowPosHack);
        aGW.write("zoom_medium_reach", advanced.zoomMediumReach);
    }
    
    //Animation editor.
    {
        string historyStr = saveEditorHistory(animEd.history);
        GetterWriter aGW(file->addNew("animation_editor"));
        
        aGW.write("bg_path", animEd.bgPath);
        aGW.write("history", historyStr);
    }
    
    //Area editor.
    {
        string historyStr = saveEditorHistory(areaEd.history);
        GetterWriter aGW(file->addNew("area_editor"));
        
        aGW.write("advanced_mode", areaEd.advancedMode);
        aGW.write("backup_interval", areaEd.backupInterval);
        aGW.write("grid_interval", areaEd.gridInterval);
        aGW.write("history", historyStr);
        aGW.write("selection_transformation", areaEd.selTrans);
        aGW.write("show_circular_info", areaEd.showCircularInfo);
        aGW.write("show_edge_length", areaEd.showEdgeLength);
        aGW.write("show_path_link_length", areaEd.showPathLinkLength);
        aGW.write("show_territory", areaEd.showTerritory);
        aGW.write("snap_mode", areaEd.snapMode);
        aGW.write("snap_threshold", areaEd.snapThreshold);
        aGW.write("undo_limit", areaEd.undoLimit);
        aGW.write("view_mode", areaEd.viewMode);
    }
    
    //Audio.
    {
        GetterWriter aGW(file->addNew("audio"));
        
        aGW.write("ambiance_sound_volume", audio.ambianceSoundVol);
        aGW.write("gameplay_sound_volume", audio.gameplaySoundVol);
        aGW.write("master_volume", audio.masterVol);
        aGW.write("music_volume", audio.musicVol);
        aGW.write("ui_sound_volume", audio.uiSoundVol);
    }
    
    //Control binds.
    {
        for(unsigned char p = 0; p < MAX_PLAYERS; p++) {
            DataNode* bindsNode =
                file->addNew("control_binds_p" + i2s(p + 1));
            game.controls.saveBindsToDataNode(bindsNode, p);
        }
    }
    
    //Controls.
    {
        GetterWriter cGW(file->addNew("controls"));

        cGW.write("auto_throw_mode", controls.autoThrowMode);
        cGW.write("leader_cursor_speed", controls.leaderCursorSpeed);
    }
    
    //Editors.
    {
        vector<string> openNodesStrs;
        for(auto& n : editors.openNodes) {
            if(n.second) openNodesStrs.push_back(n.first);
        }
        string openNodesStr = join(openNodesStrs, ";");
        GetterWriter eGW(file->addNew("editors"));
        
        eGW.write("highlight_color", editors.highlightColor);
        eGW.write("mmb_pan", editors.mmbPan);
        eGW.write("mouse_drag_threshold", editors.mouseDragThreshold);
        eGW.write("open_nodes", openNodesStr);
        eGW.write("primary_color", editors.primaryColor);
        eGW.write("secondary_color", editors.secondaryColor);
        eGW.write("show_tooltips", editors.showTooltips);
        eGW.write("text_color", editors.textColor);
        eGW.write("use_custom_style", editors.useCustomStyle);
    }
    
    //Graphics.
    {
        string resolutionStr =
            i2s(graphics.intendedWinW) + " " + i2s(graphics.intendedWinH);
        GetterWriter gGW(file->addNew("graphics"));
        
        gGW.write("cam_shake_mult", graphics.camShakeMult);
        gGW.write("fullscreen", graphics.intendedWinFullscreen);
        gGW.write("resolution", resolutionStr);
        gGW.write("true_fullscreen", graphics.trueFullscreen);
    }
    
    //Gui editor.
    {
        string historyStr = saveEditorHistory(guiEd.history);
        GetterWriter gGW(file->addNew("gui_editor"));
        
        gGW.write("grid_interval", guiEd.gridInterval);
        gGW.write("history", historyStr);
        gGW.write("snap", guiEd.snap);
    }
    
    //Misc.
    {
        GetterWriter mGW(file->addNew("misc"));
        
        mGW.write("dismiss_all", misc.dismissAll);
        mGW.write("leader_cursor_cam_weight", misc.leaderCursorCamWeight);
        mGW.write("leaving_confirmation_mode", misc.leavingConfMode);
        mGW.write("maker_tools_in_play", misc.makerToolsInPlay);
        mGW.write("pikmin_bump_dist", misc.pikminBumpDist);
        mGW.write("show_leader_cursor_counter", misc.showLeaderCursorCounter);
        mGW.write("show_hud_input_icons", misc.showHudInputIcons);
    }
    
    //Packs.
    {
        string packsDisabledStr = join(packs.disabled, ";");
        string packsLoadOrderStr = join(packs.order, ";");
        GetterWriter pGW(file->addNew("packs"));
        
        pGW.write("disabled", packsDisabledStr);
        pGW.write("order", packsLoadOrderStr);
    }
    
    //Particle editor.
    {
        string historyStr = saveEditorHistory(partEd.history);
        GetterWriter pGW(file->addNew("particle_editor"));
        
        pGW.write("bg_path", partEd.bgPath);
        pGW.write("grid_interval", partEd.gridInterval);
        pGW.write("history", historyStr);
    }
}
