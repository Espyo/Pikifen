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

//Default value for the middle zoom level.
const float ZOOM_MID_LEVEL = 1.4f;

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

//Default value for the cursor speed.
const float CURSOR_SPEED = 500.0f;

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

//Default value for the cursor camera weight.
const float CURSOR_CAM_WEIGHT = 0.0f;

//Default value for the pause menu leaving confirmation mode.
const LEAVING_CONF_MODE LEAVING_CONF = LEAVING_CONF_MODE_ALWAYS;

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
 * @brief Loads the player options from a file.
 *
 * @param file File to read from.
 */
void Options::loadFromDataNode(DataNode* file) {
    //Advanced.
    {
        ReaderSetter ars(file->getChildByName("advanced"));
        
        ars.set("draw_cursor_trail", advanced.drawCursorTrail);
        ars.set("engine_developer", advanced.engineDev);
        ars.set("fps", advanced.targetFps);
        ars.set("joystick_max_deadzone", advanced.joystickMaxDeadzone);
        ars.set("joystick_min_deadzone", advanced.joystickMinDeadzone);
        ars.set("max_particles", advanced.maxParticles);
        ars.set("middle_zoom_level", advanced.zoomMidLevel);
        ars.set("mipmaps", advanced.mipmapsEnabled);
        for(unsigned char p = 0; p < MAX_PLAYERS; p++) {
            ars.set(
                "p" + i2s((p + 1)) + "_mouse_moves_cursor",
                advanced.mouseMovesCursor[p]
            );
        }
        ars.set("smooth_scaling", advanced.smoothScaling);
        ars.set("window_position_hack", advanced.windowPosHack);
        
        advanced.targetFps = std::max(1, advanced.targetFps);
        
        if(advanced.joystickMinDeadzone > advanced.joystickMaxDeadzone) {
            std::swap(advanced.joystickMinDeadzone, advanced.joystickMaxDeadzone);
        }
        if(advanced.joystickMinDeadzone == advanced.joystickMaxDeadzone) {
            advanced.joystickMinDeadzone -= 0.1;
            advanced.joystickMaxDeadzone += 0.1;
        }
        advanced.joystickMinDeadzone = std::clamp(advanced.joystickMinDeadzone, 0.0f, 1.0f);
        advanced.joystickMaxDeadzone = std::clamp(advanced.joystickMaxDeadzone, 0.0f, 1.0f);
    }
    
    //Animation editor.
    {
        ReaderSetter ars(file->getChildByName("animation_editor"));
        string history_str;
        
        ars.set("bg_path", animEd.bgPath);
        ars.set("history", history_str);
        
        animEd.history = loadEditorHistory(history_str);
    }
    
    //Area editor.
    {
        ReaderSetter ars(file->getChildByName("area_editor"));
        string history_str;
        unsigned char snap_mode_c = areaEd.snapMode;
        unsigned char view_mode_c = areaEd.viewMode;
        
        ars.set("advanced_mode", areaEd.advancedMode);
        ars.set("backup_interval", areaEd.backupInterval);
        ars.set("grid_interval", areaEd.gridInterval);
        ars.set("history", history_str);
        ars.set("selection_transformation", areaEd.selTrans);
        ars.set("show_circular_info", areaEd.showCircularInfo);
        ars.set("show_edge_length", areaEd.showEdgeLength);
        ars.set("show_path_link_length", areaEd.showPathLinkLength);
        ars.set("show_territory", areaEd.showTerritory);
        ars.set("snap_mode", snap_mode_c);
        ars.set("snap_threshold", areaEd.snapThreshold);
        ars.set("undo_limit", areaEd.undoLimit);
        ars.set("view_mode", view_mode_c);
        
        areaEd.history = loadEditorHistory(history_str);
        
        snap_mode_c =
            std::min(
                snap_mode_c, (unsigned char) (AreaEditor::N_SNAP_MODES - 1)
            );
        areaEd.snapMode = (AreaEditor::SNAP_MODE) snap_mode_c;
        
        view_mode_c =
            std::min(
                view_mode_c, (unsigned char) (AreaEditor::N_VIEW_MODES - 1)
            );
        areaEd.viewMode = (AreaEditor::VIEW_MODE) view_mode_c;
    }
    
    //Audio.
    {
        ReaderSetter ars(file->getChildByName("audio"));
        
        ars.set("ambiance_sound_volume", audio.ambianceSoundVol);
        ars.set("gameplay_sound_volume", audio.gameplaySoundVol);
        ars.set("master_volume", audio.masterVol);
        ars.set("music_volume", audio.musicVol);
        ars.set("ui_sound_volume", audio.uiSoundVol);
        
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
            DataNode* binds_node =
                file->getChildByName("control_binds_p" + i2s(p + 1));
            game.controls.loadBindsFromDataNode(binds_node, p);
        }
    }
    
    //Controls.
    {
        ReaderSetter crs(file->getChildByName("controls"));
        unsigned char auto_throw_mode_c = controls.autoThrowMode;
        
        crs.set("auto_throw_mode", auto_throw_mode_c);
        crs.set("cursor_speed", controls.cursorSpeed);
        
        auto_throw_mode_c =
            std::min(
                auto_throw_mode_c, (unsigned char) (N_AUTO_THROW_MODES - 1)
            );
        controls.autoThrowMode = (AUTO_THROW_MODE) auto_throw_mode_c;
    }
    
    //Editors.
    {
        ReaderSetter ers(file->getChildByName("editors"));
        string open_nodes_str;
        
        ers.set("highlight_color", editors.highlightColor);
        ers.set("mmb_pan", editors.mmbPan);
        ers.set("mouse_drag_threshold", editors.mouseDragThreshold);
        ers.set("open_nodes", open_nodes_str);
        ers.set("primary_color", editors.primaryColor);
        ers.set("secondary_color", editors.secondaryColor);
        ers.set("show_tooltips", editors.showTooltips);
        ers.set("text_color", editors.textColor);
        ers.set("use_custom_style", editors.useCustomStyle);
        
        editors.openNodes.clear();
        vector<string> open_nodes_vector =
            semicolonListToVector(open_nodes_str);
        for(size_t n = 0; n < open_nodes_vector.size(); n++) {
            editors.openNodes[open_nodes_vector[n]] = true;
        }
        
        //Force the editor styles to be opaque, otherwise there can be problems.
        editors.primaryColor.a = 1.0f;
        editors.secondaryColor.a = 1.0f;
        editors.textColor.a = 1.0f;
        editors.highlightColor.a = 1.0f;
    }
    
    //Graphics.
    {
        ReaderSetter grs(file->getChildByName("graphics"));
        string resolution_str;
        
        grs.set("fullscreen", graphics.intendedWinFullscreen);
        grs.set("resolution", resolution_str);
        grs.set("true_fullscreen", graphics.trueFullscreen);
        
        vector<string> resolution_parts = split(resolution_str);
        if(resolution_parts.size() >= 2) {
            graphics.intendedWinW = std::max(1, s2i(resolution_parts[0]));
            graphics.intendedWinH = std::max(1, s2i(resolution_parts[1]));
        }
        
    }
    
    //Gui editor.
    {
        ReaderSetter grs(file->getChildByName("gui_editor"));
        string history_str;
        
        grs.set("grid_interval", guiEd.gridInterval);
        grs.set("history", history_str);
        grs.set("snap", guiEd.snap);
        
        guiEd.history = loadEditorHistory(history_str);
    }
    
    //Misc.
    {
        ReaderSetter mrs(file->getChildByName("misc"));
        unsigned char leaving_conf_mode_c = misc.leavingConfMode;
        
        mrs.set("cursor_cam_weight", misc.cursorCamWeight);
        mrs.set("leaving_confirmation_mode", leaving_conf_mode_c);
        mrs.set("show_hud_input_icons", misc.showHudInputIcons);
        
        leaving_conf_mode_c =
            std::min(
                leaving_conf_mode_c,
                (unsigned char) (N_LEAVING_CONF_MODES - 1)
            );
        misc.leavingConfMode = (LEAVING_CONF_MODE) leaving_conf_mode_c;
        
    }
    
    //Packs.
    {
        ReaderSetter prs(file->getChildByName("packs"));
        string packs_disabled_str;
        string packs_load_order_str;
        
        prs.set("disabled", packs_disabled_str);
        prs.set("order", packs_load_order_str);
        
        packs.disabled = semicolonListToVector(packs_disabled_str);
        packs.order = semicolonListToVector(packs_load_order_str);
    }
    
    //Particle editor.
    {
        ReaderSetter prs(file->getChildByName("particle_editor"));
        string history_str;
        
        prs.set("bg_path", partEd.bgPath);
        prs.set("grid_interval", partEd.gridInterval);
        prs.set("history", history_str);
        
        partEd.history = loadEditorHistory(history_str);
    }
}


/**
 * @brief Loads an editor's history from a string in the options file.
 *
 * @param str The string.
 * @return Vector with the history.
 */
vector<pair<string, string> > Options::loadEditorHistory(
    const string &str
) const {
    vector<pair<string, string> > result;
    vector<string> parts = semicolonListToVector(str);
    for(size_t e = 0; e < parts.size(); e += 2) {
        result.push_back(make_pair(parts[e], parts[e + 1]));
    }
    return result;
}


/**
 * @brief Saves an editor's history to a string in the options file.
 *
 * @param vec Vector with the history.
 * @return The string.
 */
string Options::saveEditorHistory(
    const vector<pair<string, string> > &vec
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
        GetterWriter agw(file->addNew("advanced"));
        
        agw.write("draw_cursor_trail", advanced.drawCursorTrail);
        agw.write("engine_developer", advanced.engineDev);
        agw.write("fps", advanced.targetFps);
        agw.write("joystick_max_deadzone", advanced.joystickMaxDeadzone);
        agw.write("joystick_min_deadzone", advanced.joystickMinDeadzone);
        agw.write("max_particles", advanced.maxParticles);
        agw.write("middle_zoom_level", advanced.zoomMidLevel);
        agw.write("mipmaps", advanced.mipmapsEnabled);
        for(unsigned char p = 0; p < MAX_PLAYERS; p++) {
            agw.write(
                "p" + i2s((p + 1)) + "_mouse_moves_cursor",
                advanced.mouseMovesCursor[p]
            );
        }
        agw.write("smooth_scaling", advanced.smoothScaling);
        agw.write("window_position_hack", advanced.windowPosHack);
    }
    
    //Animation editor.
    {
        string history_str = saveEditorHistory(animEd.history);
        GetterWriter agw(file->addNew("animation_editor"));
        
        agw.write("bg_path", animEd.bgPath);
        agw.write("history", history_str);
    }
    
    //Area editor.
    {
        string history_str = saveEditorHistory(areaEd.history);
        GetterWriter agw(file->addNew("area_editor"));
        
        agw.write("advanced_mode", areaEd.advancedMode);
        agw.write("backup_interval", areaEd.backupInterval);
        agw.write("grid_interval", areaEd.gridInterval);
        agw.write("history", history_str);
        agw.write("selection_transformation", areaEd.selTrans);
        agw.write("show_circular_info", areaEd.showCircularInfo);
        agw.write("show_edge_length", areaEd.showEdgeLength);
        agw.write("show_path_link_length", areaEd.showPathLinkLength);
        agw.write("show_territory", areaEd.showTerritory);
        agw.write("snap_mode", areaEd.snapMode);
        agw.write("snap_threshold", areaEd.snapThreshold);
        agw.write("undo_limit", areaEd.undoLimit);
        agw.write("view_mode", areaEd.viewMode);
    }
    
    //Audio.
    {
        GetterWriter agw(file->addNew("audio"));
        
        agw.write("ambiance_sound_volume", audio.ambianceSoundVol);
        agw.write("gameplay_sound_volume", audio.gameplaySoundVol);
        agw.write("master_volume", audio.masterVol);
        agw.write("music_volume", audio.musicVol);
        agw.write("ui_sound_volume", audio.uiSoundVol);
    }
    
    //Control binds.
    {
        for(unsigned char p = 0; p < MAX_PLAYERS; p++) {
            DataNode* binds_node =
                file->addNew("control_binds_p" + i2s(p + 1));
            game.controls.saveBindsToDataNode(binds_node, p);
        }
    }
    
    //Controls.
    {
        GetterWriter cgw(file->addNew("controls"));
        
        cgw.write("auto_throw_mode", controls.autoThrowMode);
        cgw.write("cursor_speed", controls.cursorSpeed);
    }
    
    //Editors.
    {
        vector<string> open_nodes_strs;
        for(auto &n : editors.openNodes) {
            if(n.second) open_nodes_strs.push_back(n.first);
        }
        string open_nodes_str = join(open_nodes_strs, ";");
        GetterWriter egw(file->addNew("editors"));
        
        egw.write("highlight_color", editors.highlightColor);
        egw.write("mmb_pan", editors.mmbPan);
        egw.write("mouse_drag_threshold", editors.mouseDragThreshold);
        egw.write("open_nodes", open_nodes_str);
        egw.write("primary_color", editors.primaryColor);
        egw.write("secondary_color", editors.secondaryColor);
        egw.write("show_tooltips", editors.showTooltips);
        egw.write("text_color", editors.textColor);
        egw.write("use_custom_style", editors.useCustomStyle);
    }
    
    //Graphics.
    {
        string resolution_str =
            i2s(graphics.intendedWinW) + " " + i2s(graphics.intendedWinH);
        GetterWriter ggw(file->addNew("graphics"));
        
        ggw.write("fullscreen", graphics.intendedWinFullscreen);
        ggw.write("resolution", resolution_str);
        ggw.write("true_fullscreen", graphics.trueFullscreen);
    }
    
    //Gui editor.
    {
        string history_str = saveEditorHistory(guiEd.history);
        GetterWriter ggw(file->addNew("gui_editor"));
        
        ggw.write("grid_interval", guiEd.gridInterval);
        ggw.write("history", history_str);
        ggw.write("snap", guiEd.snap);
    }
    
    //Misc.
    {
        GetterWriter mgw(file->addNew("misc"));
        
        mgw.write("cursor_cam_weight", misc.cursorCamWeight);
        mgw.write("leaving_confirmation_mode", misc.leavingConfMode);
        mgw.write("show_hud_input_icons", misc.showHudInputIcons);
    }
    
    //Packs.
    {
        string packs_disabled_str = join(packs.disabled, ";");
        string pack_load_order_str = join(packs.order, ";");
        GetterWriter pgw(file->addNew("packs"));
        
        pgw.write("disabled", packs_disabled_str);
        pgw.write("order", pack_load_order_str);
    }
    
    //Particle editor.
    {
        string history_str = saveEditorHistory(partEd.history);
        GetterWriter pgw(file->addNew("particle_editor"));
        
        pgw.write("bg_path", partEd.bgPath);
        pgw.write("grid_interval", partEd.gridInterval);
        pgw.write("history", history_str);
    }
}
