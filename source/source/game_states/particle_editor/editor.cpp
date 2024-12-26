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
#include "../../load.h"


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
 * @brief Constructs a new particle editor object.
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
    register_cmd(&particle_editor::particle_playback_toggle_cmd, "toggle_playback");
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
    if(manifest.internal_name.empty() && dialogs.size() == 1) {
        //If nothing got loaded, we can't return to the editor proper.
        //Quit out, since most of the time that's the user's intent. (e.g.
        //they entered the editor and want to leave without doing anything.)
        //Also make sure no other dialogs are trying to show up, like the load
        //failed dialog.
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
 * @brief Creates a new, empty particle generator.
 *
 * @param requested_name Name of the requested generator.
 */
void particle_editor::create_particle_generator(
    const string &requested_name
) {
    particle_generator new_gen = particle_generator();
    
    //Set up some default parameters
    new_gen.name = replace_all(requested_name, " ", "_");
    new_gen.base_particle.duration = 1;
    new_gen.base_particle.set_bitmap("");
    new_gen.base_particle.size = keyframe_interpolator<float>(32);
    new_gen.base_particle.color = keyframe_interpolator<ALLEGRO_COLOR>(map_alpha(255));
    new_gen.base_particle.color.add(1, map_alpha(0));
    
    new_gen.emission.interval = 0.5f;
    new_gen.emission.number = 1;
    new_gen.base_particle.outwards_speed = keyframe_interpolator<float>(32);
    
    loaded_gen = new_gen;
    
    save_options(); //Save the history in the options.
    save_part_gen(); //Write the file to disk
    setup_for_new_part_gen();
    
    set_status(
        "Created particle \"" + requested_name + "\" successfully."
    );
}


/**
 * @brief Handles the logic part of the main loop of the GUI editor.
 */
void particle_editor::do_logic() {
    editor::do_logic_pre();
    
    process_gui();
    
    if(generator_running) {
        loaded_gen.tick(game.delta_t, part_mgr);
        //If the particles are meant to be a burst, turn them off.
        if(loaded_gen.emission.interval == 0) {
            generator_running = false;
        }
        part_mgr.tick_all(game.delta_t);
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
 * @brief Returns some tooltip text that represents a particle generator
 * file's manifest.
 *
 * @param path Path to the file.
 * @return The tooltip text.
 */
string particle_editor::get_file_tooltip(const string &path) const {
    content_manifest temp_manif;
    game.content.custom_particle_gen.path_to_manifest(
        path, &temp_manif
    );
    return
        "Internal name: " + temp_manif.internal_name + "\n"
        "File path: " + path + "\n"
        "Pack: " + game.content.packs.list[temp_manif.pack].name;
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
 * @brief Returns the path to the currently opened content,
 * or an empty string if none.
 *
 * @return The path.
 */
string particle_editor::get_opened_content_path() const {
    return manifest.path;
}


/**
 * @brief Loads the GUI editor.
 */
void particle_editor::load() {
    editor::load();
    
    //Load necessary game content.
    game.content.reload_packs();
    game.content.load_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_CUSTOM_PARTICLE_GEN,
    },
    CONTENT_LOAD_LEVEL_BASIC
    );
    
    //Misc. setup.
    must_recenter_cam = true;
    
    game.audio.set_current_song(PARTICLE_EDITOR::SONG_NAME, false);
    
    part_mgr = particle_manager(game.options.max_particles);
    
    //Set the background.
    if(!game.options.particle_editor_bg_texture.empty()) {
        bg =
            load_bmp(
                game.options.particle_editor_bg_texture,
                nullptr, false, false, false
            );
        use_bg = true;
    } else {
        use_bg = false;
    }
    
    //Automatically load a file if needed, or show the load dialog.
    if(!auto_load_file.empty()) {
        load_part_gen_file(auto_load_file, true);
    } else {
        open_load_dialog();
    }
}


/**
 * @brief Loads a particle generator.
 *
 * @param path Path to the file.
 * @param should_update_history If true, this loading process should update
 * the user's file open history.
 */
void particle_editor::load_part_gen_file(
    const string &path, const bool should_update_history
) {
    //Setup.
    setup_for_new_part_gen();
    changes_mgr.mark_as_non_existent();
    
    //Load.
    manifest.fill_from_path(path);
    data_node file = data_node(manifest.path);
    
    if(!file.file_was_opened) {
        open_message_dialog(
            "Load failed!",
            "Failed to load the particle generator file \"" +
            manifest.path + "\"!",
        [this] () { open_load_dialog(); }
        );
        manifest.clear();
        return;
    }
    
    loaded_gen.manifest = &manifest;
    loaded_gen.load_from_data_node(&file, CONTENT_LOAD_LEVEL_FULL);
    
    //Finish up.
    changes_mgr.reset();
    
    if(should_update_history) {
        update_history(manifest, loaded_gen.name);
    }
    
    set_status("Loaded file \"" + manifest.internal_name + "\" successfully.");
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
 * @param top_cat Unused.
 * @param sec_cat Unused.
 * @param info Pointer to the file's content manifest.
 * @param is_new Unused.
 */
void particle_editor::pick_part_gen_file(
    const string &name, const string &top_cat, const string &sec_cat,
    void* info, bool is_new
) {
    content_manifest* temp_manif = (content_manifest*) info;
    
    auto really_load = [ = ] () {
        close_top_dialog();
        load_part_gen_file(temp_manif->path, true);
    };
    
    if(
        temp_manif->pack == FOLDER_NAMES::BASE_PACK &&
        !game.options.engine_developer
    ) {
        open_base_content_warning_dialog(really_load);
    } else {
        really_load();
    }
}


/**
 * @brief Reloads all loaded particle generators.
 */
void particle_editor::reload_part_gens() {
    game.content.unload_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_CUSTOM_PARTICLE_GEN,
    }
    );
    game.content.load_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_CUSTOM_PARTICLE_GEN,
    },
    CONTENT_LOAD_LEVEL_BASIC
    );
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
        std::bind(&particle_editor::save_part_gen, this)
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
        std::bind(&particle_editor::save_part_gen, this)
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
    [this] () { load_part_gen_file(string(manifest.path), false); },
    std::bind(&particle_editor::save_part_gen, this)
    );
}


/**
 * @brief Code to run for the save command.
 *
 * @param input_value Value of the player input for the command.
 */
void particle_editor::save_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    save_part_gen();
}


/**
 * @brief Sets up the editor for a new particle generator,
 * be it from an existing file or from scratch.
 */
void particle_editor::setup_for_new_part_gen() {
    part_mgr.clear();
    changes_mgr.reset();
    
    generator_running = true;
    selected_color_keyframe = 0;
    selected_size_keyframe = 0;
    selected_linear_speed_keyframe = 0;
    selected_oribital_velocity_keyframe = 0;
    selected_outward_velocity_keyframe = 0;
    
    game.cam.set_pos(point());
    game.cam.set_zoom(1.0f);
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
    if(input_value < 0.5f) return;
    
    leader_silhouette_visible = !leader_silhouette_visible;
    string state_str = (leader_silhouette_visible ? "Enabled" : "Disabled");
    set_status(state_str + " leader silhouette visibility.");
}


/**
 * @brief Code to run for the particle clearing command.
 *
 * @param input_value Value of the player input for the command.
 */
void particle_editor::clear_particles_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    part_mgr.clear();
    set_status("Cleared particles.");
}


/**
 * @brief Code to run for the emission offset toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void particle_editor::emission_outline_toggle_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    emission_offset_visible = !emission_offset_visible;
    string state_str = (emission_offset_visible ? "Enabled" : "Disabled");
    set_status(state_str + " emission offset visibility.");
}


/**
 * @brief Code to run for the particle playback toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void particle_editor::particle_playback_toggle_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
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
 * @brief Saves the particle generator onto the disk.
 *
 * @return Whether it succeded.
 */
bool particle_editor::save_part_gen() {
    loaded_gen.engine_version = get_engine_version_string();
    
    data_node file_node = data_node("", "");
    loaded_gen.save_to_data_node(&file_node);
    
    if(!file_node.save_file(manifest.path)) {
        show_message_box(
            nullptr, "Save failed!",
            "Could not save the particle file!",
            (
                "An error occured while saving the particle generator to the file \"" +
                manifest.path + "\". Make sure that the folder it is saving to "
                "exists and it is not read-only, and try again."
            ).c_str(),
            nullptr,
            ALLEGRO_MESSAGEBOX_WARN
        );
        set_status("Could not save the particle generator file!", true);
        return false;
    } else {
        set_status("Saved file successfully.");
        changes_mgr.mark_as_saved();
        update_history(manifest, loaded_gen.name);
        return true;
    }
    
    return false;
}


/**
 * @brief Unloads the editor from memory.
 */
void particle_editor::unload() {
    editor::unload();
    
    part_mgr.clear();
    
    game.content.unload_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_CUSTOM_PARTICLE_GEN,
    }
    );
}
