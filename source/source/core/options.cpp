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
        
        ars.set("draw_cursor_trail", advanced.draw_cursor_trail);
        ars.set("engine_developer", advanced.engine_dev);
        ars.set("fps", advanced.target_fps);
        ars.set("joystick_max_deadzone", advanced.joystick_max_deadzone);
        ars.set("joystick_min_deadzone", advanced.joystick_min_deadzone);
        ars.set("max_particles", advanced.max_particles);
        ars.set("middle_zoom_level", advanced.zoom_mid_level);
        ars.set("mipmaps", advanced.mipmaps_enabled);
        for(unsigned char p = 0; p < MAX_PLAYERS; p++) {
            ars.set(
                "p" + i2s((p + 1)) + "_mouse_moves_cursor",
                advanced.mouse_moves_cursor[p]
            );
        }
        ars.set("smooth_scaling", advanced.smooth_scaling);
        ars.set("window_position_hack", advanced.window_pos_hack);
        
        advanced.target_fps = std::max(1, advanced.target_fps);
        
        if(advanced.joystick_min_deadzone > advanced.joystick_max_deadzone) {
            std::swap(advanced.joystick_min_deadzone, advanced.joystick_max_deadzone);
        }
        if(advanced.joystick_min_deadzone == advanced.joystick_max_deadzone) {
            advanced.joystick_min_deadzone -= 0.1;
            advanced.joystick_max_deadzone += 0.1;
        }
        advanced.joystick_min_deadzone = std::clamp(advanced.joystick_min_deadzone, 0.0f, 1.0f);
        advanced.joystick_max_deadzone = std::clamp(advanced.joystick_max_deadzone, 0.0f, 1.0f);
    }
    
    //Animation editor.
    {
        ReaderSetter ars(file->getChildByName("animation_editor"));
        string history_str;
        
        ars.set("bg_path", anim_editor.bg_path);
        ars.set("history", history_str);
        
        anim_editor.history = loadEditorHistory(history_str);
    }
    
    //Area editor.
    {
        ReaderSetter ars(file->getChildByName("area_editor"));
        string history_str;
        unsigned char snap_mode_c = area_editor.snap_mode;
        unsigned char view_mode_c = area_editor.view_mode;
        
        ars.set("advanced_mode", area_editor.advanced_mode);
        ars.set("backup_interval", area_editor.backup_interval);
        ars.set("grid_interval", area_editor.grid_interval);
        ars.set("history", history_str);
        ars.set("selection_transformation", area_editor.sel_trans);
        ars.set("show_circular_info", area_editor.show_circular_info);
        ars.set("show_edge_length", area_editor.show_edge_length);
        ars.set("show_path_link_length", area_editor.show_path_link_length);
        ars.set("show_territory", area_editor.show_territory);
        ars.set("snap_mode", snap_mode_c);
        ars.set("snap_threshold", area_editor.snap_threshold);
        ars.set("undo_limit", area_editor.undo_limit);
        ars.set("view_mode", view_mode_c);
        
        area_editor.history = loadEditorHistory(history_str);
        
        snap_mode_c =
            std::min(
                snap_mode_c, (unsigned char) (AreaEditor::N_SNAP_MODES - 1)
            );
        area_editor.snap_mode = (AreaEditor::SNAP_MODE) snap_mode_c;
        
        view_mode_c =
            std::min(
                view_mode_c, (unsigned char) (AreaEditor::N_VIEW_MODES - 1)
            );
        area_editor.view_mode = (AreaEditor::VIEW_MODE) view_mode_c;
    }
    
    //Audio.
    {
        ReaderSetter ars(file->getChildByName("audio"));
        
        ars.set("ambiance_sound_volume", audio.ambiance_sound_vol);
        ars.set("gameplay_sound_volume", audio.gameplay_sound_vol);
        ars.set("master_volume", audio.master_vol);
        ars.set("music_volume", audio.music_vol);
        ars.set("ui_sound_volume", audio.ui_sound_vol);
        
        audio.ambiance_sound_vol = std::clamp(audio.ambiance_sound_vol, 0.0f, 1.0f);
        audio.gameplay_sound_vol = std::clamp(audio.gameplay_sound_vol, 0.0f, 1.0f);
        audio.master_vol = std::clamp(audio.master_vol, 0.0f, 1.0f);
        audio.music_vol = std::clamp(audio.music_vol, 0.0f, 1.0f);
        audio.ui_sound_vol = std::clamp(audio.ui_sound_vol, 0.0f, 1.0f);
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
        unsigned char auto_throw_mode_c = controls.auto_throw_mode;
        
        crs.set("auto_throw_mode", auto_throw_mode_c);
        crs.set("cursor_speed", controls.cursor_speed);
        
        auto_throw_mode_c =
            std::min(
                auto_throw_mode_c, (unsigned char) (N_AUTO_THROW_MODES - 1)
            );
        controls.auto_throw_mode = (AUTO_THROW_MODE) auto_throw_mode_c;
    }
    
    //Editors.
    {
        ReaderSetter ers(file->getChildByName("editors"));
        string open_nodes_str;
        
        ers.set("highlight_color", editors.highlight_color);
        ers.set("mmb_pan", editors.mmb_pan);
        ers.set("mouse_drag_threshold", editors.mouse_drag_threshold);
        ers.set("open_nodes", open_nodes_str);
        ers.set("primary_color", editors.primary_color);
        ers.set("secondary_color", editors.secondary_color);
        ers.set("show_tooltips", editors.show_tooltips);
        ers.set("text_color", editors.text_color);
        ers.set("use_custom_style", editors.use_custom_style);
        
        editors.open_nodes.clear();
        vector<string> open_nodes_vector =
            semicolonListToVector(open_nodes_str);
        for(size_t n = 0; n < open_nodes_vector.size(); n++) {
            editors.open_nodes[open_nodes_vector[n]] = true;
        }
        
        //Force the editor styles to be opaque, otherwise there can be problems.
        editors.primary_color.a = 1.0f;
        editors.secondary_color.a = 1.0f;
        editors.text_color.a = 1.0f;
        editors.highlight_color.a = 1.0f;
    }
    
    //Graphics.
    {
        ReaderSetter grs(file->getChildByName("graphics"));
        string resolution_str;
        
        grs.set("fullscreen", graphics.intended_win_fullscreen);
        grs.set("resolution", resolution_str);
        grs.set("true_fullscreen", graphics.true_fullscreen);
        
        vector<string> resolution_parts = split(resolution_str);
        if(resolution_parts.size() >= 2) {
            graphics.intended_win_w = std::max(1, s2i(resolution_parts[0]));
            graphics.intended_win_h = std::max(1, s2i(resolution_parts[1]));
        }
        
    }
    
    //Gui editor.
    {
        ReaderSetter grs(file->getChildByName("gui_editor"));
        string history_str;
        
        grs.set("grid_interval", gui_editor.grid_interval);
        grs.set("history", history_str);
        grs.set("snap", gui_editor.snap);
        
        gui_editor.history = loadEditorHistory(history_str);
    }
    
    //Misc.
    {
        ReaderSetter mrs(file->getChildByName("misc"));
        unsigned char leaving_conf_mode_c = misc.leaving_conf_mode;
        
        mrs.set("cursor_cam_weight", misc.cursor_cam_weight);
        mrs.set("leaving_confirmation_mode", leaving_conf_mode_c);
        mrs.set("show_hud_input_icons", misc.show_hud_input_icons);
        
        leaving_conf_mode_c =
            std::min(
                leaving_conf_mode_c,
                (unsigned char) (N_LEAVING_CONF_MODES - 1)
            );
        misc.leaving_conf_mode = (LEAVING_CONF_MODE) leaving_conf_mode_c;
        
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
        
        prs.set("bg_path", particle_editor.bg_path);
        prs.set("grid_interval", particle_editor.grid_interval);
        prs.set("history", history_str);
        
        particle_editor.history = loadEditorHistory(history_str);
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
        
        agw.get("draw_cursor_trail", advanced.draw_cursor_trail);
        agw.get("engine_developer", advanced.engine_dev);
        agw.get("fps", advanced.target_fps);
        agw.get("joystick_max_deadzone", advanced.joystick_max_deadzone);
        agw.get("joystick_min_deadzone", advanced.joystick_min_deadzone);
        agw.get("max_particles", advanced.max_particles);
        agw.get("middle_zoom_level", advanced.zoom_mid_level);
        agw.get("mipmaps", advanced.mipmaps_enabled);
        for(unsigned char p = 0; p < MAX_PLAYERS; p++) {
            agw.get(
                "p" + i2s((p + 1)) + "_mouse_moves_cursor",
                advanced.mouse_moves_cursor[p]
            );
        }
        agw.get("smooth_scaling", advanced.smooth_scaling);
        agw.get("window_position_hack", advanced.window_pos_hack);
    }
    
    //Animation editor.
    {
        string history_str = saveEditorHistory(anim_editor.history);
        GetterWriter agw(file->addNew("animation_editor"));
        
        agw.get("bg_path", anim_editor.bg_path);
        agw.get("history", history_str);
    }
    
    //Area editor.
    {
        string history_str = saveEditorHistory(area_editor.history);
        GetterWriter agw(file->addNew("area_editor"));
        
        agw.get("advanced_mode", area_editor.advanced_mode);
        agw.get("backup_interval", area_editor.backup_interval);
        agw.get("grid_interval", area_editor.grid_interval);
        agw.get("history", history_str);
        agw.get("selection_transformation", area_editor.sel_trans);
        agw.get("show_circular_info", area_editor.show_circular_info);
        agw.get("show_edge_length", area_editor.show_edge_length);
        agw.get("show_path_link_length", area_editor.show_path_link_length);
        agw.get("show_territory", area_editor.show_territory);
        agw.get("snap_mode", area_editor.snap_mode);
        agw.get("snap_threshold", area_editor.snap_threshold);
        agw.get("undo_limit", area_editor.undo_limit);
        agw.get("view_mode", area_editor.view_mode);
    }
    
    //Audio.
    {
        GetterWriter agw(file->addNew("audio"));
        
        agw.get("ambiance_sound_volume", audio.ambiance_sound_vol);
        agw.get("gameplay_sound_volume", audio.gameplay_sound_vol);
        agw.get("master_volume", audio.master_vol);
        agw.get("music_volume", audio.music_vol);
        agw.get("ui_sound_volume", audio.ui_sound_vol);
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
        
        cgw.get("auto_throw_mode", controls.auto_throw_mode);
        cgw.get("cursor_speed", controls.cursor_speed);
    }
    
    //Editors.
    {
        vector<string> open_nodes_strs;
        for(auto &n : editors.open_nodes) {
            if(n.second) open_nodes_strs.push_back(n.first);
        }
        string open_nodes_str = join(open_nodes_strs, ";");
        GetterWriter egw(file->addNew("editors"));
        
        egw.get("highlight_color", editors.highlight_color);
        egw.get("mmb_pan", editors.mmb_pan);
        egw.get("mouse_drag_threshold", editors.mouse_drag_threshold);
        egw.get("open_nodes", open_nodes_str);
        egw.get("primary_color", editors.primary_color);
        egw.get("secondary_color", editors.secondary_color);
        egw.get("show_tooltips", editors.show_tooltips);
        egw.get("text_color", editors.text_color);
        egw.get("use_custom_style", editors.use_custom_style);
    }
    
    //Graphics.
    {
        string resolution_str =
            i2s(graphics.intended_win_w) + " " + i2s(graphics.intended_win_h);
        GetterWriter ggw(file->addNew("graphics"));
        
        ggw.get("fullscreen", graphics.intended_win_fullscreen);
        ggw.get("resolution", resolution_str);
        ggw.get("true_fullscreen", graphics.true_fullscreen);
    }
    
    //Gui editor.
    {
        string history_str = saveEditorHistory(gui_editor.history);
        GetterWriter ggw(file->addNew("gui_editor"));
        
        ggw.get("grid_interval", gui_editor.grid_interval);
        ggw.get("history", history_str);
        ggw.get("snap", gui_editor.snap);
    }
    
    //Misc.
    {
        GetterWriter mgw(file->addNew("misc"));
        
        mgw.get("cursor_cam_weight", misc.cursor_cam_weight);
        mgw.get("leaving_confirmation_mode", misc.leaving_conf_mode);
        mgw.get("show_hud_input_icons", misc.show_hud_input_icons);
    }
    
    //Packs.
    {
        string packs_disabled_str = join(packs.disabled, ";");
        string pack_load_order_str = join(packs.order, ";");
        GetterWriter pgw(file->addNew("packs"));
        
        pgw.get("disabled", packs_disabled_str);
        pgw.get("order", pack_load_order_str);
    }
    
    //Particle editor.
    {
        string history_str = saveEditorHistory(particle_editor.history);
        GetterWriter pgw(file->addNew("particle_editor"));
        
        pgw.get("bg_path", particle_editor.bg_path);
        pgw.get("grid_interval", particle_editor.grid_interval);
        pgw.get("history", history_str);
    }
}
