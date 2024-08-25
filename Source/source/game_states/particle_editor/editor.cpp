/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * General particle editor-related functions.
 */

#include "editor.h"

#include "../../functions.h"
#include "../../game.h"
#include "../../utils/allegro_utils.h"
#include "../../utils/string_utils.h"


namespace PARTICLE_EDITOR {

//Possible grid intervals.
const vector<float> GRID_INTERVALS =
{4.0f, 8.0f, 16.0f, 32.0f, 64.0f};

//Width of the text widget that shows the mouse cursor coordinates.
const float MOUSE_COORDS_TEXT_WIDTH = 150.0f;

//Name of the song to play in this state.
const string SONG_NAME = "editors";

//Maximum zoom level possible in the editor.
const float ZOOM_MAX_LEVEL = 64;

//Minimum zoom level possible in the editor.
const float ZOOM_MIN_LEVEL = 0.5f;

}


/**
 * @brief Constructs a new GUI editor object.
 */
particle_editor::particle_editor() :
    load_dialog_picker(this) {
    
    zoom_max_level = PARTICLE_EDITOR::ZOOM_MAX_LEVEL;
    zoom_min_level = PARTICLE_EDITOR::ZOOM_MIN_LEVEL;
    
#define register_cmd(ptr, name) \
    commands.push_back( \
                        command(std::bind((ptr), this, std::placeholders::_1), \
                                (name)) \
                      );
    
    register_cmd(
        &particle_editor::grid_interval_decrease_cmd, "grid_interval_decrease"
    );
    register_cmd(
        &particle_editor::grid_interval_increase_cmd, "grid_interval_increase"
    );
    register_cmd(&particle_editor::load_cmd, "load");
    register_cmd(&particle_editor::quit_cmd, "quit");
    register_cmd(&particle_editor::particle_playback_toggle_cmd, "play_animation");
    register_cmd(
        &particle_editor::leader_silhouette_toggle_cmd,
        "leader_silhouette_toggle"
    );
    register_cmd(&particle_editor::reload_cmd, "reload");
    register_cmd(&particle_editor::save_cmd, "save");
    register_cmd(&particle_editor::zoom_and_pos_reset_cmd, "zoom_and_pos_reset");
    register_cmd(&particle_editor::zoom_in_cmd, "zoom_in");
    register_cmd(&particle_editor::zoom_out_cmd, "zoom_out");
    
#undef register_cmd
}


/**
 * @brief Code to run when the load dialog is closed.
 */
void particle_editor::close_load_dialog() {
    if(!loaded_content_yet && file_name.empty()) {
        //The user cancelled the load dialog
        //presented when you enter the GUI editor. Quit out.
        leave();
    }
}


/**
 * @brief Code to run when the options dialog is closed.
 */
void particle_editor::close_options_dialog() {
    save_options();
}


/**
 * @brief Handles the logic part of the main loop of the GUI editor.
 */
void particle_editor::do_logic() {
    editor::do_logic_pre();
    
    process_gui();
    if(loaded_content_yet) {
        if(generator_running) {
            loaded_gen.tick(game.delta_t, part_manager);
            //If the particles are meant to be a burst, turn them off.
            if(loaded_gen.emission.interval == 0)
                generator_running = false;
        }
        part_manager.tick_all(game.delta_t);
    }

    
    editor::do_logic_post();
}


/**
 * @brief Dear ImGui callback for when the canvas needs to be drawn on-screen.
 *
 * @param parent_list Unused.
 * @param cmd Unused.
 */
void particle_editor::draw_canvas_imgui_callback(
    const ImDrawList* parent_list, const ImDrawCmd* cmd
) {
    game.states.particle_ed->draw_canvas();
}


/**
 * @brief In the options data file, options pertaining to an editor's history
 * have a prefix. This function returns that prefix.
 *
 * @return The prefix.
 */
string particle_editor::get_history_option_prefix() const {
    return "particle_editor_history";
}


/**
 * @brief Returns the name of this state.
 *
 * @return The name.
 */
string particle_editor::get_name() const {
    return "Particle editor";
}


/**
 * @brief Returns the name of the currently opened file, or an empty string
 * if none.
 *
 * @return The name.
 */
string particle_editor::get_opened_file_name() const {
    return file_name;
}


/**
 * @brief Loads the GUI editor.
 */
void particle_editor::load() {
    editor::load();
    
    file_name.clear();
    loaded_content_yet = false;
    must_recenter_cam = true;
    game.audio.set_current_song(PARTICLE_EDITOR::SONG_NAME, false);

    part_manager = particle_manager(game.options.max_particles);
    if(!auto_load_file.empty()) {
        file_name = auto_load_file;
        load_particle_generator(true);
    } else {
        open_load_dialog();
    }
}


/**
 * @brief Loads the GUI file.
 *
 * @param should_update_history If true, this loading process should update
 * the user's file open history.
 */
void particle_editor::load_particle_generator(
    const bool should_update_history
) {
    data_node file_node = data_node(PARTICLE_GENERATORS_FOLDER_PATH + "/" + file_name);
    

    if(!file_node.file_was_opened) {
        set_status("Failed to load the file \"" + file_name + "\"!", true);
        open_load_dialog();
        return;
    }

    part_manager.clear();
    loaded_gen.load_from_data_node(&file_node, true);
    changes_mgr.reset();
    loaded_content_yet = true;
    
    generator_running = true;
    selected_color = 0;
    
    if(should_update_history) {
        update_history(file_name);
        save_options(); //Save the history in the options.
    }
    
    set_status("Loaded particle file successfully.");
}


/**
 * @brief Pans the camera around.
 *
 * @param ev Event to handle.
 */
void particle_editor::pan_cam(const ALLEGRO_EVENT &ev) {
    game.cam.set_pos(
        point(
            game.cam.pos.x - ev.mouse.dx / game.cam.zoom,
            game.cam.pos.y - ev.mouse.dy / game.cam.zoom
        )
    );
}


/**
 * @brief Callback for when the user picks a file from the picker.
 *
 * @param name Name of the file.
 * @param category Unused.
 * @param is_new Unused.
 */
void particle_editor::pick_file(
    const string &name, const string &category, const bool is_new
) {
    file_name = name;
    load_particle_generator(true);
    close_top_dialog();
}


/**
 * @brief Code to run for the grid interval decrease command.
 *
 * @param input_value Value of the player input for the command.
 */
void particle_editor::grid_interval_decrease_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    float new_grid_interval = PARTICLE_EDITOR::GRID_INTERVALS[0];
    for(size_t i = 0; i < PARTICLE_EDITOR::GRID_INTERVALS.size(); ++i) {
        if(
            PARTICLE_EDITOR::GRID_INTERVALS[i] >=
            game.options.particle_editor_grid_interval
        ) {
            break;
        }
        new_grid_interval = PARTICLE_EDITOR::GRID_INTERVALS[i];
    }
    game.options.particle_editor_grid_interval = new_grid_interval;
    set_status(
        "Decreased grid interval to " +
        f2s(game.options.particle_editor_grid_interval) + "."
    );
}


/**
 * @brief Code to run for the grid interval increase command.
 *
 * @param input_value Value of the player input for the command.
 */
void particle_editor::grid_interval_increase_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    float new_grid_interval = PARTICLE_EDITOR::GRID_INTERVALS.back();
    for(int i = (int) (PARTICLE_EDITOR::GRID_INTERVALS.size() - 1); i >= 0; --i) {
        if(
            PARTICLE_EDITOR::GRID_INTERVALS[i] <=
            game.options.particle_editor_grid_interval
        ) {
            break;
        }
        new_grid_interval = PARTICLE_EDITOR::GRID_INTERVALS[i];
    }
    game.options.particle_editor_grid_interval = new_grid_interval;
    set_status(
        "Increased grid interval to " +
        f2s(game.options.particle_editor_grid_interval) + "."
    );
}


/**
 * @brief Code to run for the load command.
 *
 * @param input_value Value of the player input for the command.
 */
void particle_editor::load_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changes_mgr.ask_if_unsaved(
        load_widget_pos,
        "loading a file", "load",
        std::bind(&particle_editor::open_load_dialog, this),
        std::bind(&particle_editor::save_file, this)
    );
}


/**
 * @brief Code to run for the quit command.
 *
 * @param input_value Value of the player input for the command.
 */
void particle_editor::quit_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changes_mgr.ask_if_unsaved(
        quit_widget_pos,
        "quitting", "quit",
        std::bind(&particle_editor::leave, this),
        std::bind(&particle_editor::save_file, this)
    );
}


/**
 * @brief Code to run for the reload command.
 *
 * @param input_value Value of the player input for the command.
 */
void particle_editor::reload_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changes_mgr.ask_if_unsaved(
        reload_widget_pos,
        "reloading the current file", "reload",
    [this] () { load_particle_generator(false); },
    std::bind(&particle_editor::save_file, this)
    );
}


/**
 * @brief Code to run for the save command.
 *
 * @param input_value Value of the player input for the command.
 */
void particle_editor::save_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!save_file()) {
        return;
    }
}


/**
 * @brief Code to run for the zoom and position reset command.
 *
 * @param input_value Value of the player input for the command.
 */
void particle_editor::zoom_and_pos_reset_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    reset_cam(false);
}


/**
 * @brief Code to run for the zoom in command.
 *
 * @param input_value Value of the player input for the command.
 */
void particle_editor::zoom_in_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    game.cam.target_zoom =
        clamp(
            game.cam.target_zoom +
            game.cam.zoom * EDITOR::KEYBOARD_CAM_ZOOM,
            zoom_min_level, zoom_max_level
        );
}


/**
 * @brief Code to run for the zoom out command.
 *
 * @param input_value Value of the player input for the command.
 */
void particle_editor::zoom_out_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    game.cam.target_zoom =
        clamp(
            game.cam.target_zoom -
            game.cam.zoom * EDITOR::KEYBOARD_CAM_ZOOM,
            zoom_min_level, zoom_max_level
        );
}


/**
 * @brief Code to run for the leader silhouette toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void particle_editor::leader_silhouette_toggle_cmd(float input_value) {
    if (input_value < 0.5f) return;

    leader_silhouette_visible = !leader_silhouette_visible;
    string state_str = (leader_silhouette_visible ? "Enabled" : "Disabled");
    set_status(state_str + " leader silhouette visibility.");
}


/**
 * @brief Code to run for the particle playback toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void particle_editor::particle_playback_toggle_cmd(float input_value) {
    if (input_value < 0.5f) return;

    generator_running = !generator_running;
    string state_str = (generator_running ? "Enabled" : "Disabled");
    set_status(state_str + " particle playback.");
}


/**
 * @brief Resets the camera.
 *
 * @param instantaneous Whether the camera moves to its spot instantaneously
 * or not.
 */
void particle_editor::reset_cam(const bool instantaneous) {
    center_camera(point(-300.0f, -300.0f), point(300.0f, 300.0f), instantaneous);
}


/**
 * @brief Saves the GUI file onto the disk.
 *
 * @return Whether it succeded.
 */
bool particle_editor::save_file() {
    string file_path = PARTICLE_GENERATORS_FOLDER_PATH + "/" + file_name;

    data_node file_node = data_node("", "");
    loaded_gen.save_to_data_node(&file_node);

    if(!file_node.save_file(file_path)) {
        show_message_box(
            nullptr, "Save failed!",
            "Could not save the particle file!",
            (
                "An error occured while saving the particle data to the file \"" +
                file_path + "\". Make sure that the folder it is saving to "
                "exists and it is not read-only, and try again."
            ).c_str(),
            nullptr,
            ALLEGRO_MESSAGEBOX_WARN
        );
        set_status("Could not save the particle file!", true);
        return false;
    } else {
        set_status("Saved Particle file successfully.");
        changes_mgr.mark_as_saved();
        return true;
    }
    
    return false;
}


/**
 * @brief Unloads the editor from memory.
 */
void particle_editor::unload() {
    editor::unload();
}
