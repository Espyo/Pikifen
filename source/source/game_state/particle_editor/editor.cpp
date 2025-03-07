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

#include "../../core/misc_functions.h"
#include "../../core/game.h"
#include "../../core/load.h"
#include "../../util/allegro_utils.h"
#include "../../util/string_utils.h"


namespace PARTICLE_EDITOR {

//Possible grid intervals.
const vector<float> GRID_INTERVALS =
{4.0f, 8.0f, 16.0f, 32.0f, 64.0f};

//Maximum zoom level possible in the editor.
const float ZOOM_MAX_LEVEL = 64;

//Minimum zoom level possible in the editor.
const float ZOOM_MIN_LEVEL = 0.5f;

}


/**
 * @brief Constructs a new particle editor object.
 */
ParticleEditor::ParticleEditor() :
    load_dialog_picker(this) {
    
    zoom_max_level = PARTICLE_EDITOR::ZOOM_MAX_LEVEL;
    zoom_min_level = PARTICLE_EDITOR::ZOOM_MIN_LEVEL;
    
#define register_cmd(ptr, name) \
    commands.push_back( \
                        Command(std::bind((ptr), this, std::placeholders::_1), \
                                (name)) \
                      );
    
    register_cmd(
        &ParticleEditor::grid_interval_decrease_cmd, "grid_interval_decrease"
    );
    register_cmd(
        &ParticleEditor::grid_interval_increase_cmd, "grid_interval_increase"
    );
    register_cmd(&ParticleEditor::grid_toggle_cmd, "grid_toggle");
    register_cmd(&ParticleEditor::delete_part_gen_cmd, "delete_part_gen");
    register_cmd(&ParticleEditor::load_cmd, "load");
    register_cmd(&ParticleEditor::quit_cmd, "quit");
    register_cmd(
        &ParticleEditor::part_mgr_playback_toggle_cmd, "part_mgr_toggle"
    );
    register_cmd(
        &ParticleEditor::part_gen_playback_toggle_cmd, "part_gen_toggle"
    );
    register_cmd(
        &ParticleEditor::leader_silhouette_toggle_cmd,
        "leader_silhouette_toggle"
    );
    register_cmd(&ParticleEditor::reload_cmd, "reload");
    register_cmd(&ParticleEditor::save_cmd, "save");
    register_cmd(
        &ParticleEditor::zoom_and_pos_reset_cmd, "zoom_and_pos_reset"
    );
    register_cmd(&ParticleEditor::zoom_in_cmd, "zoom_in");
    register_cmd(&ParticleEditor::zoom_out_cmd, "zoom_out");
    
#undef register_cmd
}


/**
 * @brief Code to run when the load dialog is closed.
 */
void ParticleEditor::close_load_dialog() {
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
void ParticleEditor::close_options_dialog() {
    save_options();
}


/**
 * @brief Creates a new, empty particle generator.
 *
 * @param part_gen_path Path to the new particle generator.
 */
void ParticleEditor::create_part_gen(
    const string &part_gen_path
) {
    //Setup.
    setup_for_new_part_gen_pre();
    changes_mgr.mark_as_non_existent();
    
    //Create a particle generator with some defaults.
    loaded_gen = ParticleGenerator();
    game.content.particle_gen.path_to_manifest(
        part_gen_path, &manifest
    );
    loaded_gen.manifest = &manifest;
    loaded_gen.base_particle.duration = 1.0f;
    loaded_gen.base_particle.set_bitmap("");
    loaded_gen.base_particle.size =
        KeyframeInterpolator<float>(32);
    loaded_gen.base_particle.color =
        KeyframeInterpolator<ALLEGRO_COLOR>(map_alpha(255));
    loaded_gen.base_particle.color.add(1, map_alpha(0));
    
    loaded_gen.emission.interval = 0.5f;
    loaded_gen.emission.number = 1;
    loaded_gen.base_particle.outwards_speed =
        KeyframeInterpolator<float>(32);
        
    //Finish up.
    setup_for_new_part_gen_post();
    update_history(manifest, "");
    set_status(
        "Created particle generator \"" + manifest.internal_name +
        "\" successfully."
    );
}


/**
 * @brief Deletes the current particle generator.
 */
void ParticleEditor::delete_current_part_gen() {
    string orig_internal_name = manifest.internal_name;
    bool go_to_load_dialog = true;
    bool success = false;
    string message_box_text;
    
    if(!changes_mgr.exists_on_disk()) {
        //If the generator doesn't exist on disk, since it was never
        //saved, then there's nothing to delete.
        success = true;
        go_to_load_dialog = true;
        
    } else {
        //Delete the file.
        FS_DELETE_RESULT result = delete_file(manifest.path);
        
        switch(result) {
        case FS_DELETE_RESULT_OK:
        case FS_DELETE_RESULT_HAS_IMPORTANT: {
            success = true;
            go_to_load_dialog = true;
            break;
        } case FS_DELETE_RESULT_NOT_FOUND: {
            success = false;
            message_box_text =
                "Particle generator \"" + orig_internal_name +
                "\" deletion failed! The file was not found!";
            go_to_load_dialog = false;
            break;
        } case FS_DELETE_RESULT_DELETE_ERROR: {
            success = false;
            message_box_text =
                "Particle generator \"" + orig_internal_name +
                "\" deletion failed! Something went wrong. Please make sure "
                "there are enough permissions to delete the file and "
                "try again.";
            go_to_load_dialog = false;
            break;
        }
        }
        
    }
    
    //This code will be run after everything is done, be it after the standard
    //procedure, or after the user hits OK on the message box.
    const auto finish_up = [ = ] () {
        if(go_to_load_dialog) {
            setup_for_new_part_gen_pre();
            open_load_dialog();
        }
    };
    
    //Update the status bar.
    if(success) {
        set_status(
            "Deleted particle generator \"" + orig_internal_name +
            "\" successfully."
        );
    } else {
        set_status(
            "Particle generator \"" + orig_internal_name +
            "\" deletion failed!", true
        );
    }
    
    //If there's something to tell the user, tell them.
    if(message_box_text.empty()) {
        finish_up();
    } else {
        open_message_dialog(
            "Particle generator deletion failed!",
            message_box_text,
            finish_up
        );
    }
}


/**
 * @brief Code to run for the delete current particle generator command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::delete_part_gen_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    open_dialog(
        "Delete particle generator?",
        std::bind(&ParticleEditor::process_gui_delete_part_gen_dialog, this)
    );
    dialogs.back()->custom_size = Point(600, 0);
}


/**
 * @brief Handles the logic part of the main loop of the GUI editor.
 */
void ParticleEditor::do_logic() {
    Editor::do_logic_pre();
    
    process_gui();
    
    if(mgr_running) {
        if(gen_running) {
            loaded_gen.follow_pos_offset =
                rotate_point(generator_pos_offset, -generator_angle_offset);
            loaded_gen.tick(game.delta_t, part_mgr);
            //If the particles are meant to emit once, turn them off.
            if(loaded_gen.emission.interval == 0) {
                gen_running = false;
            }
        }
        part_mgr.tick_all(game.delta_t);
    }
    
    Editor::do_logic_post();
}


/**
 * @brief Dear ImGui callback for when the canvas needs to be drawn on-screen.
 *
 * @param parent_list Unused.
 * @param cmd Unused.
 */
void ParticleEditor::draw_canvas_imgui_callback(
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
string ParticleEditor::get_file_tooltip(const string &path) const {
    ContentManifest temp_manif;
    game.content.particle_gen.path_to_manifest(
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
string ParticleEditor::get_history_option_prefix() const {
    return "particle_editor_history";
}


/**
 * @brief Returns the name of this state.
 *
 * @return The name.
 */
string ParticleEditor::get_name() const {
    return "Particle editor";
}


/**
 * @brief Returns the path to the currently opened content,
 * or an empty string if none.
 *
 * @return The path.
 */
string ParticleEditor::get_opened_content_path() const {
    return manifest.path;
}


/**
 * @brief Loads the GUI editor.
 */
void ParticleEditor::load() {
    Editor::load();
    
    //Load necessary game content.
    game.content.reload_packs();
    game.content.load_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_PARTICLE_GEN,
    },
    CONTENT_LOAD_LEVEL_BASIC
    );
    
    //Misc. setup.
    game.audio.set_current_song(game.sys_content_names.sng_editors, false);
    
    part_mgr = ParticleManager(game.options.max_particles);
    
    //Set the background.
    if(!game.options.particle_editor_bg_path.empty()) {
        bg =
            load_bmp(
                game.options.particle_editor_bg_path,
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
void ParticleEditor::load_part_gen_file(
    const string &path, const bool should_update_history
) {
    //Setup.
    setup_for_new_part_gen_pre();
    changes_mgr.mark_as_non_existent();
    
    //Load.
    manifest.fill_from_path(path);
    DataNode file = DataNode(manifest.path);
    
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
    setup_for_new_part_gen_post();
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
void ParticleEditor::pan_cam(const ALLEGRO_EVENT &ev) {
    game.cam.set_pos(
        Point(
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
void ParticleEditor::pick_part_gen_file(
    const string &name, const string &top_cat, const string &sec_cat,
    void* info, bool is_new
) {
    ContentManifest* temp_manif = (ContentManifest*) info;
    
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
void ParticleEditor::reload_part_gens() {
    game.content.unload_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_PARTICLE_GEN,
    }
    );
    game.content.load_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_PARTICLE_GEN,
    },
    CONTENT_LOAD_LEVEL_BASIC
    );
}


/**
 * @brief Code to run for the grid interval decrease command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::grid_interval_decrease_cmd(float input_value) {
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
void ParticleEditor::grid_interval_increase_cmd(float input_value) {
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
 * @brief Code to run for the grid toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::grid_toggle_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    grid_visible = !grid_visible;
    string state_str = (grid_visible ? "Enabled" : "Disabled");
    set_status(state_str + " grid visibility.");
}


/**
 * @brief Code to run for the load command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::load_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changes_mgr.ask_if_unsaved(
        load_widget_pos,
        "loading a file", "load",
        std::bind(&ParticleEditor::open_load_dialog, this),
        std::bind(&ParticleEditor::save_part_gen, this)
    );
}


/**
 * @brief Code to run for the quit command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::quit_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changes_mgr.ask_if_unsaved(
        quit_widget_pos,
        "quitting", "quit",
        std::bind(&ParticleEditor::leave, this),
        std::bind(&ParticleEditor::save_part_gen, this)
    );
}


/**
 * @brief Code to run for the reload command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::reload_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changes_mgr.ask_if_unsaved(
        reload_widget_pos,
        "reloading the current file", "reload",
    [this] () { load_part_gen_file(string(manifest.path), false); },
    std::bind(&ParticleEditor::save_part_gen, this)
    );
}


/**
 * @brief Code to run for the save command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::save_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    save_part_gen();
}


/**
 * @brief Sets up the editor for a new particle generator,
 * be it from an existing file or from scratch, after the actual creation/load
 * takes place.
 */
void ParticleEditor::setup_for_new_part_gen_post() {
    loaded_gen.follow_angle = &generator_angle_offset;
}


/**
 * @brief Sets up the editor for a new particle generator,
 * be it from an existing file or from scratch, before the actual creation/load
 * takes place.
 */
void ParticleEditor::setup_for_new_part_gen_pre() {
    part_mgr.clear();
    changes_mgr.reset();
    manifest.clear();
    
    mgr_running = true;
    gen_running = true;
    generator_angle_offset = 0.0f;
    selected_color_keyframe = 0;
    selected_size_keyframe = 0;
    selected_linear_speed_keyframe = 0;
    selected_oribital_velocity_keyframe = 0;
    selected_outward_velocity_keyframe = 0;
    loaded_gen = ParticleGenerator();
    
    game.cam.set_pos(Point());
    game.cam.set_zoom(1.0f);
}


/**
 * @brief Code to run for the zoom and position reset command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::zoom_and_pos_reset_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(game.cam.target_zoom == 1.0f) {
        game.cam.target_pos = Point();
    } else {
        game.cam.target_zoom = 1.0f;
    }
}


/**
 * @brief Code to run for the zoom in command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::zoom_in_cmd(float input_value) {
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
void ParticleEditor::zoom_out_cmd(float input_value) {
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
void ParticleEditor::leader_silhouette_toggle_cmd(float input_value) {
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
void ParticleEditor::clear_particles_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    part_mgr.clear();
    set_status("Cleared particles.");
}


/**
 * @brief Code to run for the emission shape toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::emission_shape_toggle_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    emission_shape_visible = !emission_shape_visible;
    string state_str = (emission_shape_visible ? "Enabled" : "Disabled");
    set_status(state_str + " emission shape visibility.");
}


/**
 * @brief Code to run for the particle generator playback toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::part_gen_playback_toggle_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    gen_running = !gen_running;
    string state_str = (gen_running ? "Enabled" : "Disabled");
    set_status(state_str + " particle generator playback.");
}


/**
 * @brief Code to run for the particle manager playback toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void ParticleEditor::part_mgr_playback_toggle_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    mgr_running = !mgr_running;
    string state_str = (mgr_running ? "Enabled" : "Disabled");
    set_status(state_str + " particle system playback.");
}


/**
 * @brief Resets the camera's X and Y coordinates.
 */
void ParticleEditor::reset_cam_xy() {
    game.cam.target_pos = Point();
}


/**
 * @brief Resets the camera's zoom.
 */
void ParticleEditor::reset_cam_zoom() {
    zoom_with_cursor(1.0f);
}


/**
 * @brief Saves the particle generator onto the disk.
 *
 * @return Whether it succeded.
 */
bool ParticleEditor::save_part_gen() {
    loaded_gen.engine_version = get_engine_version_string();
    
    DataNode file_node = DataNode("", "");
    loaded_gen.save_to_data_node(&file_node);
    
    if(!file_node.save_file(manifest.path)) {
        show_system_message_box(
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
void ParticleEditor::unload() {
    Editor::unload();
    
    part_mgr.clear();
    
    game.content.unload_all(
    vector<CONTENT_TYPE> {
        CONTENT_TYPE_PARTICLE_GEN,
    }
    );
}
