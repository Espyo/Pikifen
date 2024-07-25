/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * General animation editor-related functions.
 */

#include <algorithm>
#include <queue>

#include "editor.h"

#include "../../functions.h"
#include "../../game.h"
#include "../../load.h"
#include "../../utils/allegro_utils.h"
#include "../../utils/general_utils.h"
#include "../../utils/string_utils.h"


using std::queue;

namespace ANIM_EDITOR {

//Threshold for the flood-fill algorithm when picking sprite bitmap parts.
const float FLOOD_FILL_ALPHA_THRESHOLD = 0.008;

//Grid interval in the animation editor.
const float GRID_INTERVAL = 16.0f;

//Minimum radius that a hitbox can have.
const float HITBOX_MIN_RADIUS = 1.0f;

//Amount to pan the camera by when using the keyboard.
const float KEYBOARD_PAN_AMOUNT = 32.0f;

//Width of the text widget that shows the mouse cursor coordinates.
const float MOUSE_COORDS_TEXT_WIDTH = 150.0f;

//Name of the song to play in this state.
const string SONG_NAME = "editors";

//How tall the animation timeline header is.
const size_t TIMELINE_HEADER_HEIGHT = 12;

//How tall the animation timeline is, in total.
const size_t TIMELINE_HEIGHT = 48;

//Size of each side of the triangle that marks the loop frame.
const size_t TIMELINE_LOOP_TRI_SIZE = 8;

//Pad the left, right, and bottom of the timeline by this much.
const size_t TIMELINE_PADDING = 6;

//Minimum width or height a Pikmin top can have.
const float TOP_MIN_SIZE = 1.0f;

//Maximum zoom level possible in the editor.
const float ZOOM_MAX_LEVEL = 32.0f;

//Minimum zoom level possible in the editor.
const float ZOOM_MIN_LEVEL = 0.05f;

}


/**
 * @brief Constructs a new animation editor object.
 */
animation_editor::animation_editor() {

    comparison_blink_timer =
        timer(
            0.6,
    [this] () {
        this->comparison_blink_show = !this->comparison_blink_show;
        this->comparison_blink_timer.start();
    }
        );
    comparison_blink_timer.start();
    
    zoom_min_level = ANIM_EDITOR::ZOOM_MIN_LEVEL;
    zoom_max_level = ANIM_EDITOR::ZOOM_MAX_LEVEL;
    
#define register_cmd(ptr, name) \
    commands.push_back( \
                        command(std::bind((ptr), this, std::placeholders::_1), \
                                (name)) \
                      );
    
    register_cmd(&animation_editor::grid_toggle_cmd, "grid_toggle");
    register_cmd(&animation_editor::hitboxes_toggle_cmd, "hitboxes_toggle");
    register_cmd(
        &animation_editor::leader_silhouette_toggle_cmd,
        "leader_silhouette_toggle"
    );
    register_cmd(&animation_editor::load_cmd, "load");
    register_cmd(&animation_editor::mob_radius_toggle_cmd, "mob_radius_toggle");
    register_cmd(&animation_editor::play_animation_cmd, "play_animation");
    register_cmd(&animation_editor::quit_cmd, "quit");
    register_cmd(&animation_editor::reload_cmd, "reload");
    register_cmd(&animation_editor::save_cmd, "save");
    register_cmd(
        &animation_editor::zoom_and_pos_reset_cmd, "zoom_and_pos_reset"
    );
    register_cmd(&animation_editor::zoom_everything_cmd, "zoom_everything");
    register_cmd(&animation_editor::zoom_in_cmd, "zoom_in");
    register_cmd(&animation_editor::zoom_out_cmd, "zoom_out");
    
#undef register_cmd
    
}


/**
 * @brief Centers the camera on the sprite's parent bitmap, so the user
 * can choose what part of the bitmap they want to use for the sprite.
 *
 * @param instant If true, change the camera instantly.
 */
void animation_editor::center_camera_on_sprite_bitmap(const bool instant) {
    if(cur_sprite && cur_sprite->parent_bmp) {
        int bmp_w = al_get_bitmap_width(cur_sprite->parent_bmp);
        int bmp_h = al_get_bitmap_height(cur_sprite->parent_bmp);
        int bmp_x = -bmp_w / 2.0;
        int bmp_y = -bmp_h / 2.0;
        
        center_camera(point(bmp_x, bmp_y), point(bmp_x + bmp_w, bmp_y + bmp_h));
    } else {
        game.cam.target_zoom = 1.0f;
        game.cam.target_pos = point();
    }
    
    if(instant) {
        game.cam.pos = game.cam.target_pos;
        game.cam.zoom = game.cam.target_zoom;
    }
    update_transformations();
}


/**
 * @brief Changes to a new state, cleaning up whatever is needed.
 *
 * @param new_state The new state.
 */
void animation_editor::change_state(const EDITOR_STATE new_state) {
    comparison = false;
    comparison_sprite = nullptr;
    state = new_state;
    set_status();
}


/**
 * @brief Code to run when the load dialog is closed.
 */
void animation_editor::close_load_dialog() {
    if(!loaded_content_yet && file_path.empty()) {
        //The user cancelled the load dialog
        //presented when you enter the animation editor. Quit out.
        leave();
    }
}


/**
 * @brief Code to run when the options dialog is closed.
 */
void animation_editor::close_options_dialog() {
    save_options();
}


/**
 * @brief Handles the logic part of the main loop of the animation editor.
 */
void animation_editor::do_logic() {
    editor::do_logic_pre();
    
    process_gui();
    
    if(
        anim_playing && state == EDITOR_STATE_ANIMATION &&
        cur_anim_i.valid_frame()
    ) {
        frame* f = &cur_anim_i.cur_anim->frames[cur_anim_i.cur_frame_idx];
        if(f->duration != 0) {
            vector<size_t> frame_sounds;
            cur_anim_i.tick(game.delta_t, nullptr, &frame_sounds);
            
            for(size_t s = 0; s < frame_sounds.size(); ++s) {
                play_sound(frame_sounds[s]);
            }
        } else {
            anim_playing = false;
        }
    }
    
    cur_hitbox_alpha += TAU * 1.5 * game.delta_t;
    
    if(comparison_blink) {
        comparison_blink_timer.tick(game.delta_t);
    } else {
        comparison_blink_show = true;
    }
    
    editor::do_logic_post();
}


/**
 * @brief Dear ImGui callback for when the canvas needs to be drawn on-screen.
 *
 * @param parent_list Unused.
 * @param cmd Unused.
 */
void animation_editor::draw_canvas_imgui_callback(
    const ImDrawList* parent_list, const ImDrawCmd* cmd
) {
    game.states.animation_ed->draw_canvas();
}


/**
 * @brief Returns the time in the animation in which the mouse cursor is
 * currently located, if the mouse cursor is within the timeline.
 *
 * @return The time.
 */
float animation_editor::get_cursor_timeline_time() {
    if(!cur_anim_i.valid_frame()) {
        return 0.0f;
    }
    float anim_x1 = canvas_tl.x + ANIM_EDITOR::TIMELINE_PADDING;
    float anim_w = (canvas_br.x - ANIM_EDITOR::TIMELINE_PADDING) - anim_x1;
    float mouse_x = game.mouse_cursor.s_pos.x - anim_x1;
    mouse_x = clamp(mouse_x, 0.0f, anim_w);
    return cur_anim_i.cur_anim->get_duration() * (mouse_x / anim_w);
}


/**
 * @brief In the options data file, options pertaining to an editor's history
 * have a prefix. This function returns that prefix.
 *
 * @return The prefix.
 */
string animation_editor::get_history_option_prefix() const {
    return "animation_editor_history_";
}


/**
 * @brief Returns the name of this state.
 *
 * @return The name.
 */
string animation_editor::get_name() const {
    return "animation editor";
}


/**
 * @brief Returns the name of the currently opened file.
 *
 * @return The name, or an empty string if none.
 */
string animation_editor::get_opened_file_name() const {
    return file_path;
}


/**
 * @brief Returns a file path, but shortened in such a way that only the text
 * file's name and brief context about its folder remain. If that's not
 * possible, it is returned as is, though its beginning may be cropped off
 * with ellipsis if it's too big.
 *
 * @param p The long path name.
 * @return The name.
 */
string animation_editor::get_path_short_name(const string &p) const {
    if(p.find(MOB_TYPES_FOLDER_PATH) != string::npos) {
        vector<string> path_parts = split(p, "/");
        if(
            path_parts.size() > 3 &&
            path_parts[path_parts.size() - 1] == "Animations.txt"
        ) {
            return
                path_parts[path_parts.size() - 3] + "/" +
                path_parts[path_parts.size() - 2];
        }
    } else if(p.find(ANIMATIONS_FOLDER_PATH) != string::npos) {
        vector<string> path_parts = split(p, "/");
        if(!path_parts.empty()) {
            return path_parts[path_parts.size() - 1];
        }
    }
    
    if(p.size() > 33) {
        return "..." + p.substr(p.size() - 30, 30);
    }
    
    return p;
}


/**
 * @brief Imports the animation data from a different animation to the current.
 *
 * @param name Name of the animation to import.
 */
void animation_editor::import_animation_data(const string &name) {
    animation* a = anims.animations[anims.find_animation(name)];
    
    cur_anim_i.cur_anim->frames = a->frames;
    cur_anim_i.cur_anim->hit_rate = a->hit_rate;
    cur_anim_i.cur_anim->loop_frame = a->loop_frame;
    
    changes_mgr.mark_as_changed();
}


/**
 * @brief Imports the sprite file data from a different sprite to the current.
 *
 * @param name Name of the animation to import.
 */
void animation_editor::import_sprite_file_data(const string &name) {
    sprite* s = anims.sprites[anims.find_sprite(name)];
    
    cur_sprite->set_bitmap(s->file, s->file_pos, s->file_size);
    
    changes_mgr.mark_as_changed();
}


/**
 * @brief Imports the sprite hitbox data from a different sprite to the current.
 *
 * @param name Name of the animation to import.
 */
void animation_editor::import_sprite_hitbox_data(const string &name) {
    for(size_t s = 0; s < anims.sprites.size(); ++s) {
        if(anims.sprites[s]->name == name) {
            cur_sprite->hitboxes = anims.sprites[s]->hitboxes;
        }
    }
    
    update_cur_hitbox();
    
    changes_mgr.mark_as_changed();
}


/**
 * @brief Imports the sprite top data from a different sprite to the current.
 *
 * @param name Name of the animation to import.
 */
void animation_editor::import_sprite_top_data(const string &name) {
    sprite* s = anims.sprites[anims.find_sprite(name)];
    cur_sprite->top_visible = s->top_visible;
    cur_sprite->top_pos = s->top_pos;
    cur_sprite->top_size = s->top_size;
    cur_sprite->top_angle = s->top_angle;
    
    changes_mgr.mark_as_changed();
}


/**
 * @brief Imports the sprite transformation data from
 * a different sprite to the current.
 *
 * @param name Name of the animation to import.
 */
void animation_editor::import_sprite_transformation_data(const string &name) {
    sprite* s = anims.sprites[anims.find_sprite(name)];
    cur_sprite->offset = s->offset;
    cur_sprite->scale = s->scale;
    cur_sprite->angle = s->angle;
    cur_sprite->tint = s->tint;
}


/**
 * @brief Returns whether the mouse cursor is inside the animation
 * timeline or not.
 *
 * @return Whether the cursor is inside.
 */
bool animation_editor::is_cursor_in_timeline() {
    return
        state == EDITOR_STATE_ANIMATION &&
        game.mouse_cursor.s_pos.x >= canvas_tl.x &&
        game.mouse_cursor.s_pos.x <= canvas_br.x &&
        game.mouse_cursor.s_pos.y >= canvas_br.y -
        ANIM_EDITOR::TIMELINE_HEIGHT &&
        game.mouse_cursor.s_pos.y <= canvas_br.y;
}


/**
 * @brief Loads the animation editor.
 */
void animation_editor::load() {
    editor::load();
    
    game.content.load(CONTENT_TYPE_CUSTOM_PARTICLE_GEN, false);
    game.content.load(CONTENT_TYPE_STATUS_TYPE, false);
    game.content.load(CONTENT_TYPE_SPRAY_TYPE, false);
    game.content.load(CONTENT_TYPE_LIQUID, false);
    game.content.load(CONTENT_TYPE_HAZARD, false);
    game.content.load(CONTENT_TYPE_SPIKE_DAMAGE_TYPE, false);
    game.content.load(CONTENT_TYPE_MOB_TYPE, false);
    
    load_custom_mob_cat_types(false);
    
    file_path.clear();
    animation_exists_on_disk = false;
    can_save = false;
    loaded_content_yet = false;
    side_view = false;
    change_state(EDITOR_STATE_MAIN);
    game.audio.set_current_song(ANIM_EDITOR::SONG_NAME, false);
    
    if(!game.options.anim_editor_bg_texture.empty()) {
        bg =
            load_bmp(
                game.options.anim_editor_bg_texture,
                nullptr, false, false, false, true
            );
        use_bg = true;
    } else {
        use_bg = false;
    }
    
    if(!auto_load_anim.empty()) {
        loaded_mob_type = nullptr;
        file_path = auto_load_anim;
        load_animation_database(true);
    } else {
        open_load_dialog();
    }
}


/**
 * @brief Loads the animation database for the current object.
 *
 * @param should_update_history If true, this loading process should update
 * the user's file open history.
 */
void animation_editor::load_animation_database(
    const bool should_update_history
) {
    if(state == EDITOR_STATE_SPRITE_BITMAP) {
        //Ideally, states would be handled by a state machine, and this
        //logic would be placed in the sprite bitmap state's "on exit" code...
        game.cam.set_pos(pre_sprite_bmp_cam_pos);
        game.cam.set_zoom(pre_sprite_bmp_cam_zoom);
    }
    
    file_path = standardize_path(file_path);
    
    anims.destroy();
    
    data_node file = data_node(file_path);
    if(!file.file_was_opened) {
        file.save_file(file_path, true);
    }
    anims.load_from_data_node(&file);
    anims.path = file_path;
    
    cur_anim_i.clear();
    anim_playing = false;
    cur_sprite = nullptr;
    cur_hitbox = nullptr;
    cur_hitbox_idx = 0;
    
    animation_exists_on_disk = true;
    can_save = true;
    changes_mgr.reset();
    
    game.cam.set_pos(point());
    game.cam.set_zoom(1.0f);
    
    //Find the most popular file name to suggest for new sprites.
    last_spritesheet_used.clear();
    
    if(!anims.sprites.empty()) {
        map<string, size_t> file_uses_map;
        vector<std::pair<size_t, string> > file_uses_vector;
        for(size_t f = 0; f < anims.sprites.size(); ++f) {
            file_uses_map[anims.sprites[f]->file]++;
        }
        for(auto &u : file_uses_map) {
            file_uses_vector.push_back(make_pair(u.second, u.first));
        }
        std::sort(
            file_uses_vector.begin(),
            file_uses_vector.end(),
            [] (
                std::pair<size_t, string> u1, std::pair<size_t, string> u2
        ) -> bool {
            return u1.first > u2.first;
        }
        );
        last_spritesheet_used = file_uses_vector[0].second;
    }
    
    vector<string> file_path_parts = split(file_path, "/");
    
    if(file_path.find(MOB_TYPES_FOLDER_PATH) != string::npos) {
        vector<string> path_parts = split(file_path, "/");
        if(
            path_parts.size() > 3 &&
            path_parts[path_parts.size() - 1] == "Animations.txt"
        ) {
            mob_category* cat =
                game.mob_categories.get_from_folder_name(
                    MOB_TYPES_FOLDER_PATH + "/" +
                    path_parts[path_parts.size() - 3]
                );
            if(cat) {
                loaded_mob_type =
                    game.mob_categories.find_mob_type_from_folder_name(
                        cat,
                        path_parts[path_parts.size() - 2]
                    );
            }
        }
    }
    
    //Top bitmap.
    for(unsigned char t = 0; t < NR_MATURITIES; ++t) {
        if(top_bmp[t] && top_bmp[t] != game.bmp_error) {
            al_destroy_bitmap(top_bmp[t]);
            top_bmp[t] = nullptr;
        }
    }
    
    if(
        loaded_mob_type &&
        loaded_mob_type->category->id == MOB_CATEGORY_PIKMIN
    ) {
        data_node data =
            load_data_file(
                loaded_mob_type->path + "/Data.txt"
            );
        top_bmp[0] =
            load_bmp(data.get_child_by_name("top_leaf")->value, &data);
        top_bmp[1] =
            load_bmp(data.get_child_by_name("top_bud")->value, &data);
        top_bmp[2] =
            load_bmp(data.get_child_by_name("top_flower")->value, &data);
    }
    
    if(loaded_mob_type) anims.fill_sound_idx_caches(loaded_mob_type);
    
    if(should_update_history) {
        update_history(file_path);
        save_options(); //Save the history in the options.
    }
    
    change_state(EDITOR_STATE_MAIN);
    loaded_content_yet = true;
    
    set_status("Loaded file successfully.");
}


/**
 * @brief Pans the camera around.
 *
 * @param ev Event to handle.
 */
void animation_editor::pan_cam(const ALLEGRO_EVENT &ev) {
    game.cam.set_pos(
        point(
            game.cam.pos.x - ev.mouse.dx / game.cam.zoom,
            game.cam.pos.y - ev.mouse.dy / game.cam.zoom
        )
    );
}


/**
 * @brief Callback for when the user picks an animation from the picker.
 *
 * @param name Name of the animation.
 * @param category Unused.
 * @param is_new Is this a new animation or an existing one?
 */
void animation_editor::pick_animation(
    const string &name, const string &category, const bool is_new
) {
    if(is_new) {
        anims.animations.push_back(new animation(name));
        anims.sort_alphabetically();
        changes_mgr.mark_as_changed();
        set_status("Created animation \"" + name + "\".");
    }
    cur_anim_i.clear();
    cur_anim_i.anim_db = &anims;
    cur_anim_i.cur_anim = anims.animations[anims.find_animation(name)];
}


/**
 * @brief Callback for when the user picks a sprite from the picker.
 *
 * @param name Name of the sprite.
 * @param category Unused.
 * @param is_new Is this a new sprite or an existing one?
 */
void animation_editor::pick_sprite(
    const string &name, const string &category, const bool is_new
) {
    if(is_new) {
        if(anims.find_sprite(name) == INVALID) {
            anims.sprites.push_back(new sprite(name));
            anims.sprites.back()->create_hitboxes(
                &anims,
                loaded_mob_type ? loaded_mob_type->height : 128,
                loaded_mob_type ? loaded_mob_type->radius : 32
            );
            anims.sort_alphabetically();
            changes_mgr.mark_as_changed();
            set_status("Created sprite \"" + name + "\".");
        }
    }
    cur_sprite = anims.sprites[anims.find_sprite(name)];
    update_cur_hitbox();
    
    if(is_new) {
        //New sprite. Suggest file name.
        cur_sprite->set_bitmap(last_spritesheet_used, point(), point());
    }
}


/**
 * @brief Plays one of the mob's sounds.
 *
 * @param sound_idx Index of the sound data in the mob type's sound list.
 */
void animation_editor::play_sound(size_t sound_idx) {
    if(!loaded_mob_type) return;
    mob_type::sfx_t* sfx_data = &loaded_mob_type->sounds[sound_idx];
    if(!sfx_data->sample) return;
    game.audio.create_ui_sfx_source(
        sfx_data->sample,
        sfx_data->config
    );
}


/**
 * @brief Code to run for the grid toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void animation_editor::grid_toggle_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    grid_visible = !grid_visible;
    string state_str = (grid_visible ? "Enabled" : "Disabled");
    set_status(state_str + " grid visibility.");
}


/**
 * @brief Code to run for the hitboxes toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void animation_editor::hitboxes_toggle_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    hitboxes_visible = !hitboxes_visible;
    string state_str = (hitboxes_visible ? "Enabled" : "Disabled");
    set_status(state_str + " hitbox visibility.");
}


/**
 * @brief Code to run for the leader silhouette toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void animation_editor::leader_silhouette_toggle_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    leader_silhouette_visible = !leader_silhouette_visible;
    string state_str = (leader_silhouette_visible ? "Enabled" : "Disabled");
    set_status(state_str + " leader silhouette visibility.");
}


/**
 * @brief Code to run for the load file command.
 *
 * @param input_value Value of the player input for the command.
 */
void animation_editor::load_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changes_mgr.ask_if_unsaved(
        load_widget_pos,
        "loading a file", "load",
        std::bind(&animation_editor::open_load_dialog, this),
        std::bind(&animation_editor::save_animation_database, this)
    );
}


/**
 * @brief Code to run for the mob radius toggle command.
 *
 * @param input_value Value of the player input for the command.
 */
void animation_editor::mob_radius_toggle_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    mob_radius_visible = !mob_radius_visible;
    string state_str = (mob_radius_visible ? "Enabled" : "Disabled");
    set_status(state_str + " object radius visibility.");
}


/**
 * @brief Code to run for the play animation command.
 *
 * @param input_value Value of the player input for the command.
 */
void animation_editor::play_animation_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!cur_anim_i.valid_frame()) {
        anim_playing = false;
    } else {
        anim_playing = !anim_playing;
        if(anim_playing) {
            if(is_shift_pressed) {
                cur_anim_i.to_start();
                set_status("Animation playback started from the beginning.");
            } else {
                set_status("Animation playback started.");
            }
        } else {
            set_status("Animation playback stopped.");
        }
    }
}


/**
 * @brief Code to run for the quit command.
 *
 * @param input_value Value of the player input for the command.
 */
void animation_editor::quit_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    changes_mgr.ask_if_unsaved(
        quit_widget_pos,
        "quitting", "quit",
        std::bind(&animation_editor::leave, this),
        std::bind(&animation_editor::save_animation_database, this)
    );
}


/**
 * @brief Code to run for the reload command.
 *
 * @param input_value Value of the player input for the command.
 */
void animation_editor::reload_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!animation_exists_on_disk) return;
    changes_mgr.ask_if_unsaved(
        reload_widget_pos,
        "reloading the current file", "reload",
    [this] () { load_animation_database(false); },
    std::bind(&animation_editor::save_animation_database, this)
    );
}


/**
 * @brief Code to run for the save command.
 *
 * @param input_value Value of the player input for the command.
 */
void animation_editor::save_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(!can_save) return;
    save_animation_database();
}


/**
 * @brief Code to run when the zoom and position reset button widget is pressed.
 *
 * @param input_value Value of the player input for the command.
 */
void animation_editor::zoom_and_pos_reset_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    if(game.cam.target_zoom == 1.0f) {
        game.cam.target_pos = point();
    } else {
        game.cam.target_zoom = 1.0f;
    }
}


/**
 * @brief Code to run for the zoom everything command.
 *
 * @param input_value Value of the player input for the command.
 */
void animation_editor::zoom_everything_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    sprite* s_ptr = cur_sprite;
    if(!s_ptr && cur_anim_i.valid_frame()) {
        string name =
            cur_anim_i.cur_anim->frames[cur_anim_i.cur_frame_idx].sprite_name;
        size_t s_pos = anims.find_sprite(name);
        if(s_pos != INVALID) s_ptr = anims.sprites[s_pos];
    }
    if(!s_ptr || !s_ptr->bitmap) return;
    
    point cmin, cmax;
    get_transformed_rectangle_bounding_box(
        s_ptr->offset, s_ptr->file_size * s_ptr->scale,
        s_ptr->angle, &cmin, &cmax
    );
    
    if(s_ptr->top_visible) {
        point top_min, top_max;
        get_transformed_rectangle_bounding_box(
            s_ptr->top_pos, s_ptr->top_size,
            s_ptr->top_angle,
            &top_min, &top_max
        );
        cmin.x = std::min(cmin.x, top_min.x);
        cmin.y = std::min(cmin.y, top_min.y);
        cmax.x = std::max(cmax.x, top_max.x);
        cmax.y = std::max(cmax.y, top_max.y);
    }
    
    for(size_t h = 0; h < s_ptr->hitboxes.size(); ++h) {
        hitbox* h_ptr = &s_ptr->hitboxes[h];
        cmin.x = std::min(cmin.x, h_ptr->pos.x - h_ptr->radius);
        cmin.y = std::min(cmin.y, h_ptr->pos.y - h_ptr->radius);
        cmax.x = std::max(cmax.x, h_ptr->pos.x + h_ptr->radius);
        cmax.y = std::max(cmax.y, h_ptr->pos.y + h_ptr->radius);
    }
    
    center_camera(cmin, cmax);
}


/**
 * @brief Code to run for the zoom in command
 *
 * @param input_value Value of the player input for the command.
 */
void animation_editor::zoom_in_cmd(float input_value) {
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
void animation_editor::zoom_out_cmd(float input_value) {
    if(input_value < 0.5f) return;
    
    game.cam.target_zoom =
        clamp(
            game.cam.target_zoom -
            game.cam.zoom * EDITOR::KEYBOARD_CAM_ZOOM,
            zoom_min_level, zoom_max_level
        );
}


/**
 * @brief Renames an animation to the given name.
 *
 * @param anim Animation to rename.
 * @param new_name Its new name.
 */
void animation_editor::rename_animation(
    animation* anim, const string &new_name
) {
    //Check if it's valid.
    if(!anim) {
        return;
    }
    
    string old_name = anim->name;
    
    //Check if the name is the same.
    if(new_name == old_name) {
        set_status();
        return;
    }
    
    //Check if the name is empty.
    if(new_name.empty()) {
        set_status("You need to specify the animation's new name!", true);
        return;
    }
    
    //Check if the name already exists.
    for(size_t a = 0; a < anims.animations.size(); ++a) {
        if(anims.animations[a]->name == new_name) {
            set_status(
                "An animation by the name \"" + new_name + "\" already exists!",
                true
            );
            return;
        }
    }
    
    //Rename!
    anim->name = new_name;
    
    changes_mgr.mark_as_changed();
    set_status(
        "Renamed animation \"" + old_name + "\" to \"" + new_name + "\"."
    );
}


/**
 * @brief Renames a body part to the given name.
 *
 * @param part Body part to rename.
 * @param new_name Its new name.
 */
void animation_editor::rename_body_part(
    body_part* part, const string &new_name
) {
    //Check if it's valid.
    if(!part) {
        return;
    }
    
    string old_name = part->name;
    
    //Check if the name is the same.
    if(new_name == old_name) {
        set_status();
        return;
    }
    
    //Check if the name is empty.
    if(new_name.empty()) {
        set_status("You need to specify the body part's new name!", true);
        return;
    }
    
    //Check if the name already exists.
    for(size_t b = 0; b < anims.body_parts.size(); ++b) {
        if(anims.body_parts[b]->name == new_name) {
            set_status(
                "A body part by the name \"" + new_name + "\" already exists!",
                true
            );
            return;
        }
    }
    
    //Rename!
    for(size_t s = 0; s < anims.sprites.size(); ++s) {
        for(size_t h = 0; h < anims.sprites[s]->hitboxes.size(); ++h) {
            if(anims.sprites[s]->hitboxes[h].body_part_name == old_name) {
                anims.sprites[s]->hitboxes[h].body_part_name = new_name;
            }
        }
    }
    part->name = new_name;
    update_hitboxes();
    
    changes_mgr.mark_as_changed();
    set_status(
        "Renamed body part \"" + old_name + "\" to \"" + new_name + "\"."
    );
}


/**
 * @brief Renames a sprite to the given name.
 *
 * @param spr Sprite to rename.
 * @param new_name Its new name.
 */
void animation_editor::rename_sprite(
    sprite* spr, const string &new_name
) {
    //Check if it's valid.
    if(!spr) {
        return;
    }
    
    string old_name = spr->name;
    
    //Check if the name is the same.
    if(new_name == old_name) {
        set_status();
        return;
    }
    
    //Check if the name is empty.
    if(new_name.empty()) {
        set_status("You need to specify the sprite's new name!", true);
        return;
    }
    
    //Check if the name already exists.
    for(size_t s = 0; s < anims.sprites.size(); ++s) {
        if(anims.sprites[s]->name == new_name) {
            set_status(
                "A sprite by the name \"" + new_name + "\" already exists!",
                true
            );
            return;
        }
    }
    
    //Rename!
    spr->name = new_name;
    for(size_t a = 0; a < anims.animations.size(); ++a) {
        animation* a_ptr = anims.animations[a];
        for(size_t f = 0; f < a_ptr->frames.size(); ++f) {
            if(a_ptr->frames[f].sprite_name == old_name) {
                a_ptr->frames[f].sprite_name = new_name;
            }
        }
    }
    
    changes_mgr.mark_as_changed();
    set_status(
        "Renamed sprite \"" + old_name + "\" to \"" + new_name + "\"."
    );
}


/**
 * @brief Resets the camera's X and Y coordinates.
 */
void animation_editor::reset_cam_xy() {
    game.cam.target_pos = point();
}


/**
 * @brief Resets the camera's zoom.
 */
void animation_editor::reset_cam_zoom() {
    zoom_with_cursor(1.0f);
}


/**
 * @brief Resizes all sprites, hitboxes, etc. by a multiplier.
 *
 * @param mult Multiplier to resize by.
 */
void animation_editor::resize_everything(const float mult) {
    if(mult == 0.0f) {
        set_status("Can't resize everything to size 0!", true);
        return;
    }
    if(mult == 1.0f) {
        set_status(
            "Resizing everything by 1 wouldn't make a difference!", true
        );
        return;
    }
    
    for(size_t s = 0; s < anims.sprites.size(); ++s) {
        resize_sprite(anims.sprites[s], mult);
    }
    
    changes_mgr.mark_as_changed();
    set_status("Resized everything by " + f2s(mult) + ".");
}


/**
 * @brief Resizes a sprite by a multiplier.
 *
 * @param s Sprite to resize.
 * @param mult Multiplier to resize by.
 */
void animation_editor::resize_sprite(sprite* s, const float mult) {
    if(mult == 0.0f) {
        set_status("Can't resize a sprite to size 0!", true);
        return;
    }
    if(mult == 1.0f) {
        set_status("Resizing a sprite by 1 wouldn't make a difference!", true);
        return;
    }
    
    s->scale    *= mult;
    s->offset   *= mult;
    s->top_pos  *= mult;
    s->top_size *= mult;
    
    for(size_t h = 0; h < s->hitboxes.size(); ++h) {
        hitbox* h_ptr = &s->hitboxes[h];
        
        h_ptr->radius = fabs(h_ptr->radius * mult);
        h_ptr->pos    *= mult;
    }
    
    changes_mgr.mark_as_changed();
    set_status("Resized sprite by " + f2s(mult) + ".");
}


/**
 * @brief Saves the animation database onto the mob's file.
 *
 * @return Whether it succeded.
 */
bool animation_editor::save_animation_database() {
    anims.engine_version = get_engine_version_string();
    anims.sort_alphabetically();
    
    data_node file_node = data_node("", "");
    
    anims.save_to_data_node(
        &file_node,
        loaded_mob_type && loaded_mob_type->category->id == MOB_CATEGORY_PIKMIN
    );
    
    if(!file_node.save_file(file_path)) {
        show_message_box(
            nullptr, "Save failed!",
            "Could not save the animation!",
            (
                "An error occured while saving the animation to the file \"" +
                file_path + "\". Make sure that the folder it is saving to "
                "exists and it is not read-only, and try again."
            ).c_str(),
            nullptr,
            ALLEGRO_MESSAGEBOX_WARN
        );
        set_status("Could not save the animation file!", true);
        return false;
        
    } else {
        set_status("Saved file successfully.");
        changes_mgr.mark_as_saved();
        return true;
        
    }
}


/**
 * @brief Sets all sprite scales to the value specified in the textbox.
 *
 * @param scale Value to set the scales to.
 */
void animation_editor::set_all_sprite_scales(const float scale) {
    if(scale == 0) {
        set_status("The scales can't be 0!", true);
        return;
    }
    
    for(size_t s = 0; s < anims.sprites.size(); ++s) {
        sprite* s_ptr = anims.sprites[s];
        s_ptr->scale.x = scale;
        s_ptr->scale.y = scale;
    }
    
    changes_mgr.mark_as_changed();
    set_status("Set all sprite scales to " + f2s(scale) + ".");
}


/**
 * @brief Sets the current frame to be the most apt sprite it can find,
 * given the current circumstances.
 *
 * Basically, it picks a sprite that's called something similar to
 * the current animation.
 */
void animation_editor::set_best_frame_sprite() {
    if(anims.sprites.empty()) return;
    
    //Find the sprites that match the most characters with the animation name.
    //Let's set the starting best score to 3, as an arbitrary way to
    //sift out results that technically match, but likely aren't the same
    //term. Example: If the animation is called "running", and there is no
    //"runnning" sprite, we probably don't want a match with "rummaging".
    //Unless it's the exact same word.
    //Also, set the final sprite index to 0 so that if something goes wrong,
    //we default to the first sprite on the list.
    size_t final_sprite_idx = 0;
    vector<size_t> best_sprite_idxs;
    
    if(anims.sprites.size() > 1) {
        size_t best_score = 3;
        for(size_t s = 0; s < anims.sprites.size(); ++s) {
            size_t score = 0;
            if(
                str_to_lower(cur_anim_i.cur_anim->name) ==
                str_to_lower(anims.sprites[s]->name)
            ) {
                score = 9999;
            } else {
                score =
                    get_matching_string_starts(
                        str_to_lower(cur_anim_i.cur_anim->name),
                        str_to_lower(anims.sprites[s]->name)
                    ).size();
            }
            
            if(score < best_score) {
                continue;
            }
            if(score > best_score) {
                best_score = score;
                best_sprite_idxs.clear();
            }
            best_sprite_idxs.push_back(s);
        }
    }
    
    if(best_sprite_idxs.size() == 1) {
        //If there's only one best match, go for it.
        final_sprite_idx = best_sprite_idxs[0];
        
    } else if(best_sprite_idxs.size() > 1) {
        //Sort them alphabetically and pick the first.
        std::sort(
            best_sprite_idxs.begin(),
            best_sprite_idxs.end(),
        [this, &best_sprite_idxs] (const size_t s1, const size_t s2) {
            return
                str_to_lower(anims.sprites[s1]->name) <
                str_to_lower(anims.sprites[s2]->name);
        });
        final_sprite_idx = best_sprite_idxs[0];
    }
    
    //Finally, set the frame info then.
    frame* cur_frame_ptr =
        &cur_anim_i.cur_anim->frames[cur_anim_i.cur_frame_idx];
    cur_frame_ptr->sprite_idx = final_sprite_idx;
    cur_frame_ptr->sprite_ptr = anims.sprites[final_sprite_idx];
    cur_frame_ptr->sprite_name = anims.sprites[final_sprite_idx]->name;
}


/**
 * @brief Performs a flood fill on the bitmap sprite, to see what parts
 * contain non-alpha pixels, based on a starting position.
 *
 * @param bmp Locked bitmap to check.
 * @param selection_pixels Array that controls which pixels are selected or not.
 * @param x X coordinate to start on.
 * @param y Y coordinate to start on.
 */
void animation_editor::sprite_bmp_flood_fill(
    ALLEGRO_BITMAP* bmp, bool* selection_pixels, const int x, const int y
) {
    //https://en.wikipedia.org/wiki/Flood_fill#The_algorithm
    int bmp_w = al_get_bitmap_width(bmp);
    int bmp_h = al_get_bitmap_height(bmp);
    
    if(x < 0 || x > bmp_w) return;
    if(y < 0 || y > bmp_h) return;
    if(selection_pixels[y * bmp_w + x]) return;
    if(al_get_pixel(bmp, x, y).a < ANIM_EDITOR::FLOOD_FILL_ALPHA_THRESHOLD) {
        return;
    }
    
    /**
     * @brief A point, but with integer coordinates.
     */
    struct int_point {
    
        //--- Members ---
        
        //X coordinate.
        int x = 0;
        
        //Y coordinate.
        int y = 0;
        
        
        //--- Function definitions ---
        
        /**
         * @brief Constructs a new int point object.
         *
         * @param p The float point coordinates.
         */
        explicit int_point(const point &p) :
            x(p.x),
            y(p.y) { }
            
        /**
         * @brief Constructs a new int point object.
         *
         * @param x X coordinate.
         * @param y Y coordinate.
         */
        int_point(int x, int y) :
            x(x),
            y(y) { }
            
    };
    
    queue<int_point> pixels_left;
    pixels_left.push(int_point(x, y));
    
    while(!pixels_left.empty()) {
        int_point p = pixels_left.front();
        pixels_left.pop();
        
        if(
            selection_pixels[(p.y) * bmp_w + p.x] ||
            al_get_pixel(bmp, p.x, p.y).a <
            ANIM_EDITOR::FLOOD_FILL_ALPHA_THRESHOLD
        ) {
            continue;
        }
        
        int_point offset = p;
        vector<int_point> columns;
        columns.push_back(p);
        
        bool add = true;
        //Keep going left and adding columns to check.
        while(add) {
            if(offset.x  == 0) {
                add = false;
            } else {
                offset.x--;
                if(selection_pixels[offset.y * bmp_w + offset.x]) {
                    add = false;
                } else if(
                    al_get_pixel(bmp, offset.x, offset.y).a <
                    ANIM_EDITOR::FLOOD_FILL_ALPHA_THRESHOLD
                ) {
                    add = false;
                } else {
                    columns.push_back(offset);
                }
            }
        }
        offset = p;
        add = true;
        //Keep going right and adding columns to check.
        while(add) {
            if(offset.x == bmp_w - 1) {
                add = false;
            } else {
                offset.x++;
                if(selection_pixels[offset.y * bmp_w + offset.x]) {
                    add = false;
                } else if(
                    al_get_pixel(bmp, offset.x, offset.y).a <
                    ANIM_EDITOR::FLOOD_FILL_ALPHA_THRESHOLD
                ) {
                    add = false;
                } else {
                    columns.push_back(offset);
                }
            }
        }
        
        for(size_t c = 0; c < columns.size(); ++c) {
            //For each column obtained, mark the pixel there,
            //and check the pixels above and below, to see if they should be
            //processed next.
            int_point col = columns[c];
            selection_pixels[col.y * bmp_w + col.x] = true;
            if(
                col.y > 0 &&
                !selection_pixels[(col.y - 1) * bmp_w + col.x] &&
                al_get_pixel(bmp, col.x, col.y - 1).a >=
                ANIM_EDITOR::FLOOD_FILL_ALPHA_THRESHOLD
            ) {
                pixels_left.push(int_point(col.x, col.y - 1));
            }
            if(
                col.y < bmp_h - 1 &&
                !selection_pixels[(col.y + 1) * bmp_w + col.x] &&
                al_get_pixel(bmp, col.x, col.y + 1).a >=
                ANIM_EDITOR::FLOOD_FILL_ALPHA_THRESHOLD
            ) {
                pixels_left.push(int_point(col.x, col.y + 1));
            }
        }
    }
}


/**
 * @brief Unloads the editor from memory.
 */
void animation_editor::unload() {
    editor::unload();
    
    anims.destroy();
    
    game.content.unload(CONTENT_TYPE_MOB_TYPE, false);
    game.content.unload(CONTENT_TYPE_SPIKE_DAMAGE_TYPE, false);
    game.content.unload(CONTENT_TYPE_HAZARD, false);
    game.content.unload(CONTENT_TYPE_LIQUID, false);
    game.content.unload(CONTENT_TYPE_SPRAY_TYPE, false);
    game.content.unload(CONTENT_TYPE_STATUS_TYPE, false);
    game.content.unload(CONTENT_TYPE_CUSTOM_PARTICLE_GEN, false);
    
    if(bg) {
        al_destroy_bitmap(bg);
        bg = NULL;
    }
}


/**
 * @brief Updates the current hitbox pointer to match the same body part as
 * before, but on the hitbox of the current sprite. If not applicable,
 * it chooses a valid hitbox.
 *
 */
void animation_editor::update_cur_hitbox() {
    if(cur_sprite->hitboxes.empty()) {
        cur_hitbox = nullptr;
        cur_hitbox_idx = INVALID;
        return;
    }
    
    cur_hitbox_idx = std::min(cur_hitbox_idx, cur_sprite->hitboxes.size() - 1);
    cur_hitbox = &cur_sprite->hitboxes[cur_hitbox_idx];
}


/**
 * @brief Update every frame's hitbox instances in light of new hitbox info.
 */
void animation_editor::update_hitboxes() {
    for(size_t s = 0; s < anims.sprites.size(); ++s) {
    
        sprite* s_ptr = anims.sprites[s];
        
        //Start by deleting non-existent hitboxes.
        for(size_t h = 0; h < s_ptr->hitboxes.size();) {
            string h_name = s_ptr->hitboxes[h].body_part_name;
            bool name_found = false;
            
            for(size_t b = 0; b < anims.body_parts.size(); ++b) {
                if(anims.body_parts[b]->name == h_name) {
                    name_found = true;
                    break;
                }
            }
            
            if(!name_found) {
                s_ptr->hitboxes.erase(
                    s_ptr->hitboxes.begin() + h
                );
            } else {
                h++;
            }
        }
        
        //Add missing hitboxes.
        for(size_t b = 0; b < anims.body_parts.size(); ++b) {
            bool hitbox_found = false;
            string name = anims.body_parts[b]->name;
            
            for(size_t h = 0; h < s_ptr->hitboxes.size(); ++h) {
                if(s_ptr->hitboxes[h].body_part_name == name) {
                    hitbox_found = true;
                    break;
                }
            }
            
            if(!hitbox_found) {
                s_ptr->hitboxes.push_back(
                    hitbox(
                        name, INVALID, nullptr, point(), 0,
                        loaded_mob_type ? loaded_mob_type->height : 128,
                        loaded_mob_type ? loaded_mob_type->radius : 32
                    )
                );
            }
        }
        
        //Sort them with the new order.
        std::sort(
            s_ptr->hitboxes.begin(),
            s_ptr->hitboxes.end(),
        [this] (const hitbox & h1, const hitbox & h2) -> bool {
            size_t pos1 = 0, pos2 = 1;
            for(size_t b = 0; b < anims.body_parts.size(); ++b) {
                if(anims.body_parts[b]->name == h1.body_part_name) pos1 = b;
                if(anims.body_parts[b]->name == h2.body_part_name) pos2 = b;
            }
            return pos1 < pos2;
        }
        );
    }
}
