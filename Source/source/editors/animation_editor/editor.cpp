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
#include "../../utils/string_utils.h"

using std::queue;


//How many entries of the history to store, at most.
const size_t animation_editor::HISTORY_SIZE = 6;
//Amount to pan the camera by when using the keyboard.
const float animation_editor::KEYBOARD_PAN_AMOUNT = 32.0f;
//How tall the animation timeline header is.
const size_t animation_editor::TIMELINE_HEADER_HEIGHT = 12;
//How tall the animation timeline is, in total.
const size_t animation_editor::TIMELINE_HEIGHT = 48;
//Size of each side of the triangle that marks the loop frame.
const size_t animation_editor::TIMELINE_LOOP_TRI_SIZE = 8;
//Pad the left, right, and bottom of the timeline by this much.
const size_t animation_editor::TIMELINE_PADDING = 6;
//Maximum zoom level possible in the editor.
const float animation_editor::ZOOM_MAX_LEVEL_EDITOR = 32.0f;
//Minimum zoom level possible in the editor.
const float animation_editor::ZOOM_MIN_LEVEL_EDITOR = 0.05f;


/* ----------------------------------------------------------------------------
 * Initializes animation editor class stuff.
 */
animation_editor::animation_editor() :
    anim_playing(false),
    comparison(false),
    comparison_above(true),
    comparison_blink(true),
    comparison_blink_show(true),
    comparison_blink_timer(0),
    comparison_sprite(nullptr),
    comparison_tint(true),
    cur_anim(NULL),
    cur_frame_nr(INVALID),
    cur_frame_time(0),
    cur_hitbox(nullptr),
    cur_hitbox_alpha(0),
    cur_hitbox_nr(INVALID),
    cur_maturity(0),
    cur_sprite(NULL),
    hitboxes_visible(true),
    loaded_mob_type(nullptr),
    mob_radius_visible(false),
    grid_visible(true),
    pikmin_silhouette_visible(false),
    reset_load_dialog(true),
    sprite_bmp_add_mode(false),
    side_view(false) {
    
    top_bmp[0] = NULL;
    top_bmp[1] = NULL;
    top_bmp[2] = NULL;
    comparison_blink_timer =
        timer(
            0.6,
    [this] () {
        this->comparison_blink_show = !this->comparison_blink_show;
        this->comparison_blink_timer.start();
    }
        );
    comparison_blink_timer.start();
    
    cur_hitbox_tc.keep_aspect_ratio = true;
    cur_sprite_tc.keep_aspect_ratio = true;
    cur_sprite_tc.allow_rotation = true;
    top_tc.keep_aspect_ratio = true;
    top_tc.allow_rotation = true;
    
    zoom_min_level = ZOOM_MIN_LEVEL_EDITOR;
    zoom_max_level = ZOOM_MAX_LEVEL_EDITOR;
}


/* ----------------------------------------------------------------------------
 * Centers the camera on the sprite's parent bitmap, so the user can choose
 * what part of the bitmap they want to use for the sprite.
 * instant:
 *   If true, change the camera instantly.
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


/* ----------------------------------------------------------------------------
 * Changes to a new state, cleaning up whatever is needed.
 */
void animation_editor::change_state(const EDITOR_STATES new_state) {
    comparison = false;
    comparison_sprite = NULL;
    state = new_state;
    status_text.clear();
}


/* ----------------------------------------------------------------------------
 * Code to run when the load dialog is closed.
 */
void animation_editor::close_load_dialog() {
    if(!loaded_content_yet && file_path.empty()) {
        //The user cancelled the load dialog
        //presented when you enter the animation editor. Quit out.
        leave();
    }
}


/* ----------------------------------------------------------------------------
 * Code to run when the options dialog is closed.
 */
void animation_editor::close_options_dialog() {
    save_options();
}


/* ----------------------------------------------------------------------------
 * Handles the logic part of the main loop of the animation editor.
 */
void animation_editor::do_logic() {
    editor::do_logic_pre();
    
    process_gui();
    
    if(
        anim_playing && state == EDITOR_STATE_ANIMATION &&
        cur_anim && cur_frame_nr != INVALID
    ) {
        frame* f = &cur_anim->frames[cur_frame_nr];
        if(f->duration != 0) {
            cur_frame_time += game.delta_t;
            
            while(cur_frame_time > f->duration) {
                cur_frame_time = cur_frame_time - f->duration;
                cur_frame_nr++;
                if(cur_frame_nr >= cur_anim->frames.size()) {
                    cur_frame_nr =
                        (
                            cur_anim->loop_frame >=
                            cur_anim->frames.size()
                        ) ? 0 : cur_anim->loop_frame;
                }
                f = &cur_anim->frames[cur_frame_nr];
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


/* ----------------------------------------------------------------------------
 * Dear ImGui callback for when the canvas needs to be drawn on-screen.
 */
void animation_editor::draw_canvas_imgui_callback(
    const ImDrawList* parent_list, const ImDrawCmd* cmd
) {
    game.states.animation_editor_st->draw_canvas();
}


/* ----------------------------------------------------------------------------
 * Returns the name of this state.
 */
string animation_editor::get_name() const {
    return "animation editor";
}


/* ----------------------------------------------------------------------------
 * Returns a file path, but shortened in such a way that only the text file's
 * name and brief context about its folder remain. If that's not possible, it
 * is returned as is, though its beginning may be cropped off with ellipsis
 * if it's too big.
 */
string animation_editor::get_path_short_name(const string &p) const {
    if(p.find(TYPES_FOLDER_PATH) != string::npos) {
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


/* ----------------------------------------------------------------------------
 * Handles the current hitbox's transformation controller having been altered.
 */
void animation_editor::handle_cur_hitbox_tc() {
    if(!cur_sprite && !cur_hitbox) return;
    
    if(side_view) {
        cur_hitbox->pos.x = cur_hitbox_tc.get_center().x;
        cur_hitbox->radius = cur_hitbox_tc.get_size().x / 2.0f;
        cur_hitbox->z =
            -(
                cur_hitbox_tc.get_center().y +
                cur_hitbox_tc.get_size().y / 2.0f
            );
        cur_hitbox->height = cur_hitbox_tc.get_size().y;
    } else {
        cur_hitbox->pos = cur_hitbox_tc.get_center();
        cur_hitbox->radius = cur_hitbox_tc.get_size().x / 2.0f;
    }
    
    if(cur_hitbox->radius <= 0.0f) {
        cur_hitbox->radius = 16.0f;
        update_cur_hitbox_tc();
    }
}


/* ----------------------------------------------------------------------------
 * Handles the current sprite's transformation controller having
 * been altered.
 */
void animation_editor::handle_cur_sprite_tc() {
    if(!cur_sprite) return;
    
    cur_sprite->offset = cur_sprite_tc.get_center();
    cur_sprite->scale =
        cur_sprite_tc.get_size() / cur_sprite->file_size;
    cur_sprite->angle = cur_sprite_tc.get_angle();
}


/* ----------------------------------------------------------------------------
 * Handles the current sprite's top transformation controller having
 * been altered.
 */
void animation_editor::handle_top_tc() {
    if(!cur_sprite) return;
    
    cur_sprite->top_pos = top_tc.get_center();
    cur_sprite->top_size = top_tc.get_size();
    cur_sprite->top_angle = top_tc.get_angle();
}


/* ----------------------------------------------------------------------------
 * Imports the animation data from a different animation to the current.
 */
void animation_editor::import_animation_data(const string &name) {
    animation* a = anims.animations[anims.find_animation(name)];
    
    cur_anim->frames = a->frames;
    cur_anim->hit_rate = a->hit_rate;
    cur_anim->loop_frame = a->loop_frame;
    
    made_new_changes = true;
}


/* ----------------------------------------------------------------------------
 * Imports the sprite file data from a different sprite to the current.
 */
void animation_editor::import_sprite_file_data(const string &name) {
    sprite* s = anims.sprites[anims.find_sprite(name)];
    
    cur_sprite->set_bitmap(s->file, s->file_pos, s->file_size);
    
    made_new_changes = true;
}


/* ----------------------------------------------------------------------------
 * Imports the sprite hitbox data from a different sprite to the current.
 */
void animation_editor::import_sprite_hitbox_data(const string &name) {
    for(size_t s = 0; s < anims.sprites.size(); ++s) {
        if(anims.sprites[s]->name == name) {
            cur_sprite->hitboxes = anims.sprites[s]->hitboxes;
        }
    }
    
    cur_hitbox_nr = INVALID;
    cur_hitbox = NULL;
    if(!cur_sprite->hitboxes.empty()) {
        cur_hitbox_nr = 0;
        cur_hitbox = &cur_sprite->hitboxes[0];
    }
    
    update_cur_hitbox_tc();
    made_new_changes = true;
}


/* ----------------------------------------------------------------------------
 * Imports the sprite top data from a different sprite to the current.
 */
void animation_editor::import_sprite_top_data(const string &name) {
    sprite* s = anims.sprites[anims.find_sprite(name)];
    cur_sprite->top_visible = s->top_visible;
    cur_sprite->top_pos = s->top_pos;
    cur_sprite->top_size = s->top_size;
    cur_sprite->top_angle = s->top_angle;
    
    top_tc.set_center(cur_sprite->top_pos);
    top_tc.set_size(cur_sprite->top_size);
    top_tc.set_angle(cur_sprite->top_angle);
    
    made_new_changes = true;
}


/* ----------------------------------------------------------------------------
 * Imports the sprite transformation data from
 * a different sprite to the current.
 */
void animation_editor::import_sprite_transformation_data(const string &name) {
    sprite* s = anims.sprites[anims.find_sprite(name)];
    cur_sprite->offset = s->offset;
    cur_sprite->scale = s->scale;
    cur_sprite->angle = s->angle;
    update_cur_sprite_tc();
}


/* ----------------------------------------------------------------------------
 * Loads the animation database for the current object.
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
    anims = load_animation_database_from_file(&file);
    
    anim_playing = false;
    cur_anim = NULL;
    cur_sprite = NULL;
    cur_frame_nr = INVALID;
    cur_hitbox = NULL;
    cur_hitbox_nr = INVALID;
    
    can_reload = true;
    can_save = true;
    
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
    
    if(file_path.find(TYPES_FOLDER_PATH) != string::npos) {
        vector<string> path_parts = split(file_path, "/");
        if(
            path_parts.size() > 3 &&
            path_parts[path_parts.size() - 1] == "Animations.txt"
        ) {
            mob_category* cat =
                game.mob_categories.get_from_folder_name(
                    TYPES_FOLDER_PATH + "/" + path_parts[path_parts.size() - 3]
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
    for(unsigned char t = 0; t < N_MATURITIES; ++t) {
        if(top_bmp[t] && top_bmp[t] != game.bmp_error) {
            al_destroy_bitmap(top_bmp[t]);
            top_bmp[t] = NULL;
        }
    }
    
    if(
        loaded_mob_type &&
        loaded_mob_type->category->id == MOB_CATEGORY_PIKMIN
    ) {
        data_node data =
            load_data_file(
                loaded_mob_type->category->folder + "/" +
                file_path_parts[file_path_parts.size() - 2] +
                "/Data.txt"
            );
        top_bmp[0] =
            load_bmp(data.get_child_by_name("top_leaf")->value, &data);
        top_bmp[1] =
            load_bmp(data.get_child_by_name("top_bud")->value, &data);
        top_bmp[2] =
            load_bmp(data.get_child_by_name("top_flower")->value, &data);
    }
    
    if(should_update_history) {
        update_history(file_path);
        save_options(); //Save the history on the options.
    }
    
    change_state(EDITOR_STATE_MAIN);
    loaded_content_yet = true;
    
    status_text = "Loaded file successfully.";
}


/* ----------------------------------------------------------------------------
 * Loads the animation editor.
 */
void animation_editor::load() {
    editor::load();
    
    load_custom_particle_generators(false);
    load_status_types(false);
    load_spray_types(false);
    load_liquids(false);
    load_hazards();
    load_spike_damage_types();
    load_mob_types(false);
    
    file_path.clear();
    can_reload = false;
    can_save = false;
    loaded_content_yet = false;
    side_view = false;
    cur_hitbox_tc.keep_aspect_ratio = true;
    change_state(EDITOR_STATE_MAIN);
    
    if(!auto_load_anim.empty()) {
        loaded_mob_type = NULL;
        file_path = auto_load_anim;
        load_animation_database(true);
    } else {
        open_load_dialog();
    }
}


/* ----------------------------------------------------------------------------
 * Code to run when the grid button widget is pressed.
 */
void animation_editor::press_grid_button() {
    grid_visible = !grid_visible;
    string state_str = (grid_visible ? "visible" : "invisible");
    status_text = "The grid is now " + state_str + ".";
}


/* ----------------------------------------------------------------------------
 * Code to run when the hitboxes button widget is pressed.
 */
void animation_editor::press_hitboxes_button() {
    hitboxes_visible = !hitboxes_visible;
    string state_str = (hitboxes_visible ? "visible" : "invisible");
    status_text = "The hitboxes are now " + state_str + ".";
}


/* ----------------------------------------------------------------------------
 * Code to run when the load file button widget is pressed.
 */
void animation_editor::press_load_button() {
    if(!check_new_unsaved_changes(load_widget_pos)) {
        open_load_dialog();
    }
}


/* ----------------------------------------------------------------------------
 * Code to run when the mob radius button widget is pressed.
 */
void animation_editor::press_mob_radius_button() {
    mob_radius_visible = !mob_radius_visible;
    string state_str = (mob_radius_visible ? "visible" : "invisible");
    status_text = "The object radius is now " + state_str + ".";
}


/* ----------------------------------------------------------------------------
 * Code to run when the Pikmin silhouette button widget is pressed.
 */
void animation_editor::press_pikmin_silhouette_button() {
    pikmin_silhouette_visible = !pikmin_silhouette_visible;
    string state_str = (pikmin_silhouette_visible ? "visible" : "invisible");
    status_text = "The Pikmin silhouette is now " + state_str + ".";
}


/* ----------------------------------------------------------------------------
 * Code to run when the play animation button widget is pressed.
 */
void animation_editor::press_play_animation_button() {
    if(cur_anim->frames.empty()) {
        anim_playing = false;
    } else {
        anim_playing = !anim_playing;
        if(
            !cur_anim->frames.empty() &&
            cur_frame_nr == INVALID
        ) {
            cur_frame_nr = 0;
        }
        cur_frame_time = 0;
        if(anim_playing) {
            status_text = "Animation playback started.";
        } else {
            status_text = "Animation playback stopped.";
        }
    }
}


/* ----------------------------------------------------------------------------
 * Code to run when the quit button widget is pressed.
 */
void animation_editor::press_quit_button() {
    if(!check_new_unsaved_changes(quit_widget_pos)) {
        status_text = "Bye!";
        leave();
    }
}


/* ----------------------------------------------------------------------------
 * Code to run when the reload button widget is pressed.
 */
void animation_editor::press_reload_button() {
    if(!can_reload) return;
    if(!check_new_unsaved_changes(reload_widget_pos)) {
        load_animation_database(false);
    }
}


/* ----------------------------------------------------------------------------
 * Code to run when the save button widget is pressed.
 */
void animation_editor::press_save_button() {
    if(!can_save) return;
    save_animation_database();
}


/* ----------------------------------------------------------------------------
 * Callback for when the user picks an animation from the picker.
 */
void animation_editor::pick_animation(
    const string &name, const string &category, const bool is_new
) {
    if(is_new) {
        anims.animations.push_back(new animation(name));
        anims.sort_alphabetically();
        made_new_changes = true;
        status_text = "Created animation \"" + name + "\".";
    }
    cur_anim = anims.animations[anims.find_animation(name)];
    cur_frame_nr = (cur_anim->frames.size()) ? 0 : INVALID;
    cur_frame_time = 0;
}


/* ----------------------------------------------------------------------------
 * Callback for when the user picks a sprite from the picker.
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
            made_new_changes = true;
            status_text = "Created sprite \"" + name + "\".";
        }
    }
    cur_sprite = anims.sprites[anims.find_sprite(name)];
    cur_hitbox = NULL;
    cur_hitbox_nr = INVALID;
    if(is_new) {
        //New sprite. Suggest file name.
        cur_sprite->set_bitmap(last_spritesheet_used, point(), point());
    }
}


/* ----------------------------------------------------------------------------
 * Renames an animation to the given name.
 */
void animation_editor::rename_animation(animation* a, const string &new_name) {
    //Check if it's valid.
    if(!a) {
        return;
    }
    
    string old_name = a->name;
    
    //Check if the name is the same.
    if(new_name == old_name) {
        status_text.clear();
        return;
    }
    
    //Check if the name is empty.
    if(new_name.empty()) {
        status_text = "You need to specify the animation's new name!";
        return;
    }
    
    //Check if the name already exists.
    for(size_t a = 0; a < anims.animations.size(); ++a) {
        if(anims.animations[a]->name == new_name) {
            status_text = "That animation name is already being used!";
            return;
        }
    }
    
    //Rename!
    a->name = new_name;
    anims.sort_alphabetically();
    
    made_new_changes = true;
    status_text =
        "Renamed animation \"" + old_name + "\" to \"" + new_name + "\".";
}


/* ----------------------------------------------------------------------------
 * Renames a body part to the given name.
 */
void animation_editor::rename_body_part(body_part* p, const string &new_name) {
    //Check if it's valid.
    if(!p) {
        return;
    }
    
    string old_name = p->name;
    
    //Check if the name is the same.
    if(new_name == old_name) {
        status_text.clear();
        return;
    }
    
    //Check if the name is empty.
    if(new_name.empty()) {
        status_text = "You need to specify the body part's new name!";
        return;
    }
    
    //Check if the name already exists.
    for(size_t b = 0; b < anims.body_parts.size(); ++b) {
        if(anims.body_parts[b]->name == new_name) {
            status_text = "That body part name is already being used!";
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
    p->name = new_name;
    update_hitboxes();
    
    made_new_changes = true;
    status_text =
        "Renamed body part \"" + old_name + "\" to \"" + new_name + "\".";
}


/* ----------------------------------------------------------------------------
 * Renames a sprite to the given name.
 */
void animation_editor::rename_sprite(sprite* s, const string &new_name) {
    //Check if it's valid.
    if(!s) {
        return;
    }
    
    string old_name = s->name;
    
    //Check if the name is the same.
    if(new_name == old_name) {
        status_text.clear();
        return;
    }
    
    //Check if the name is empty.
    if(new_name.empty()) {
        status_text = "You need to specify the sprite's new name!";
        return;
    }
    
    //Check if the name already exists.
    for(size_t s = 0; s < anims.sprites.size(); ++s) {
        if(anims.sprites[s]->name == new_name) {
            status_text = "That name is already being used!";
            return;
        }
    }
    
    //Rename!
    s->name = new_name;
    for(size_t a = 0; a < anims.animations.size(); ++a) {
        animation* a_ptr = anims.animations[a];
        for(size_t f = 0; f < a_ptr->frames.size(); ++f) {
            if(a_ptr->frames[f].sprite_name == old_name) {
                a_ptr->frames[f].sprite_name = new_name;
            }
        }
    }
    anims.sort_alphabetically();
    
    made_new_changes = true;
    status_text =
        "Renamed sprite \"" + old_name + "\" to \"" + new_name + "\".";
}


/* ----------------------------------------------------------------------------
 * Resizes all sprites, hitboxes, etc. by a multiplier.
 */
void animation_editor::resize_everything(const float mult) {
    if(mult == 0.0f) {
        status_text = "Can't resize everything to size 0!";
        return;
    }
    if(mult == 1.0f) {
        status_text = "Resizing everything by 1 wouldn't make a difference!";
        return;
    }
    
    for(size_t s = 0; s < anims.sprites.size(); ++s) {
        resize_sprite(anims.sprites[s], mult);
    }
    
    made_new_changes = true;
    status_text = "Resized everything by " + f2s(mult) + ".";
}


/* ----------------------------------------------------------------------------
 * Resizes a sprite by a multiplier.
 */
void animation_editor::resize_sprite(sprite* s, const float mult) {
    if(mult == 0.0f) {
        status_text = "Can't resize a sprite to size 0!";
        return;
    }
    if(mult == 1.0f) {
        status_text = "Resizing a sprite by 1 wouldn't make a difference!";
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
    
    made_new_changes = true;
    status_text = "Resized sprite by " + f2s(mult) + ".";
}


/* ----------------------------------------------------------------------------
 * Saves the animation database onto the mob's file.
 */
void animation_editor::save_animation_database() {
    data_node file_node = data_node("", "");
    
    data_node* animations_node = new data_node("animations", "");
    file_node.add(animations_node);
    
    for(size_t a = 0; a < anims.animations.size(); ++a) {
        data_node* anim_node = new data_node(anims.animations[a]->name, "");
        animations_node->add(anim_node);
        
        if(anims.animations[a]->loop_frame > 0) {
            anim_node->add(
                new data_node(
                    "loop_frame", i2s(anims.animations[a]->loop_frame)
                )
            );
        }
        if(anims.animations[a]->hit_rate != 100) {
            anim_node->add(
                new data_node("hit_rate", i2s(anims.animations[a]->hit_rate))
            );
        }
        data_node* frames_node = new data_node("frames", "");
        anim_node->add(frames_node);
        
        for(
            size_t f = 0; f < anims.animations[a]->frames.size();
            ++f
        ) {
            frame* f_ptr = &anims.animations[a]->frames[f];
            
            data_node* frame_node =
                new data_node(f_ptr->sprite_name, "");
            frames_node->add(frame_node);
            
            frame_node->add(
                new data_node("duration", f2s(f_ptr->duration))
            );
            if(f_ptr->signal != INVALID) {
                frame_node->add(
                    new data_node("signal", i2s(f_ptr->signal))
                );
            }
        }
    }
    
    data_node* sprites_node = new data_node("sprites", "");
    file_node.add(sprites_node);
    
    for(size_t s = 0; s < anims.sprites.size(); ++s) {
        sprite* s_ptr = anims.sprites[s];
        data_node* sprite_node = new data_node(anims.sprites[s]->name, "");
        sprites_node->add(sprite_node);
        
        sprite_node->add(new data_node("file",      s_ptr->file));
        sprite_node->add(new data_node("file_pos",  p2s(s_ptr->file_pos)));
        sprite_node->add(new data_node("file_size", p2s(s_ptr->file_size)));
        if(s_ptr->offset.x != 0.0 || s_ptr->offset.y != 0.0) {
            sprite_node->add(new data_node("offset", p2s(s_ptr->offset)));
        }
        if(s_ptr->scale.x != 1.0 || s_ptr->scale.y != 1.0) {
            sprite_node->add(new data_node("scale", p2s(s_ptr->scale)));
        }
        if(s_ptr->angle != 0.0) {
            sprite_node->add(new data_node("angle", f2s(s_ptr->angle)));
        }
        
        if(
            loaded_mob_type &&
            loaded_mob_type->category->id == MOB_CATEGORY_PIKMIN
        ) {
            sprite_node->add(
                new data_node("top_visible", b2s(s_ptr->top_visible))
            );
            sprite_node->add(
                new data_node("top_pos", p2s(s_ptr->top_pos))
            );
            sprite_node->add(
                new data_node("top_size", p2s(s_ptr->top_size))
            );
            sprite_node->add(
                new data_node("top_angle", f2s(s_ptr->top_angle))
            );
        }
        
        if(!s_ptr->hitboxes.empty()) {
            data_node* hitboxes_node =
                new data_node("hitboxes", "");
            sprite_node->add(hitboxes_node);
            
            for(size_t h = 0; h < s_ptr->hitboxes.size(); ++h) {
                hitbox* h_ptr = &s_ptr->hitboxes[h];
                
                data_node* hitbox_node =
                    new data_node(h_ptr->body_part_name, "");
                hitboxes_node->add(hitbox_node);
                
                hitbox_node->add(
                    new data_node("coords", p2s(h_ptr->pos, &h_ptr->z))
                );
                hitbox_node->add(
                    new data_node("height", f2s(h_ptr->height))
                );
                hitbox_node->add(
                    new data_node("radius", f2s(h_ptr->radius))
                );
                hitbox_node->add(
                    new data_node("type", i2s(h_ptr->type))
                );
                hitbox_node->add(
                    new data_node("value", f2s(h_ptr->value))
                );
                if(
                    h_ptr->type == HITBOX_TYPE_NORMAL &&
                    h_ptr->can_pikmin_latch
                ) {
                    hitbox_node->add(
                        new data_node(
                            "can_pikmin_latch", b2s(h_ptr->can_pikmin_latch)
                        )
                    );
                }
                if(!h_ptr->hazards_str.empty()) {
                    hitbox_node->add(
                        new data_node("hazards", h_ptr->hazards_str)
                    );
                }
                if(
                    h_ptr->type == HITBOX_TYPE_ATTACK &&
                    h_ptr->knockback_outward
                ) {
                    hitbox_node->add(
                        new data_node(
                            "knockback_outward",
                            b2s(h_ptr->knockback_outward)
                        )
                    );
                }
                if(
                    h_ptr->type == HITBOX_TYPE_ATTACK &&
                    h_ptr->knockback_angle != 0
                ) {
                    hitbox_node->add(
                        new data_node(
                            "knockback_angle", f2s(h_ptr->knockback_angle)
                        )
                    );
                }
                if(
                    h_ptr->type == HITBOX_TYPE_ATTACK &&
                    h_ptr->knockback != 0
                ) {
                    hitbox_node->add(
                        new data_node("knockback", f2s(h_ptr->knockback))
                    );
                }
                if(
                    h_ptr->type == HITBOX_TYPE_ATTACK &&
                    h_ptr->wither_chance > 0
                ) {
                    hitbox_node->add(
                        new data_node(
                            "wither_chance", i2s(h_ptr->wither_chance)
                        )
                    );
                }
            }
        }
    }
    
    data_node* body_parts_node = new data_node("body_parts", "");
    file_node.add(body_parts_node);
    
    for(size_t b = 0; b < anims.body_parts.size(); ++b) {
        data_node* body_part_node =
            new data_node(anims.body_parts[b]->name, "");
        body_parts_node->add(body_part_node);
        
    }
    
    if(!file_node.save_file(file_path)) {
        show_message_box(
            NULL, "Save failed!",
            "Could not save the animation!",
            (
                "An error occured while saving the animation to the file \"" +
                file_path + "\". Make sure that the folder it is saving to "
                "exists and it is not read-only, and try again."
            ).c_str(),
            NULL,
            ALLEGRO_MESSAGEBOX_WARN
        );
        status_text = "Could not save the animation file!";
    } else {
        status_text = "Saved file successfully.";
    }
    made_new_changes = false;
}


/* ----------------------------------------------------------------------------
 * Sets all sprite scales to the value specified in the textbox.
 */
void animation_editor::set_all_sprite_scales(const float scale) {
    if(scale == 0) {
        status_text = "The scales can't be 0!";
        return;
    }
    
    for(size_t s = 0; s < anims.sprites.size(); ++s) {
        sprite* s_ptr = anims.sprites[s];
        s_ptr->scale.x = scale;
        s_ptr->scale.y = scale;
    }
    
    made_new_changes = true;
    status_text = "Set all sprite scales to " + f2s(scale) + ".";
}


static const float FLOOD_FILL_ALPHA_THRESHOLD = 0.008;

/* ----------------------------------------------------------------------------
 * Performs a flood fill on the bitmap sprite, to see what parts
 * contain non-alpha pixels, based on a starting position.
 * bmp:              Locked bitmap to check.
 * selection_pixels: Array that controls which pixels are selected or not.
 * x, y:             Image coordinates to start on.
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
    if(al_get_pixel(bmp, x, y).a < FLOOD_FILL_ALPHA_THRESHOLD) {
        return;
    }
    
    struct int_point {
        int x;
        int y;
        int_point(point p) :
            x(p.x),
            y(p.y) { }
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
            al_get_pixel(bmp, p.x, p.y).a < FLOOD_FILL_ALPHA_THRESHOLD
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
                    FLOOD_FILL_ALPHA_THRESHOLD
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
                    FLOOD_FILL_ALPHA_THRESHOLD
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
                FLOOD_FILL_ALPHA_THRESHOLD
            ) {
                pixels_left.push(int_point(col.x, col.y - 1));
            }
            if(
                col.y < bmp_h - 1 &&
                !selection_pixels[(col.y + 1) * bmp_w + col.x] &&
                al_get_pixel(bmp, col.x, col.y + 1).a >=
                FLOOD_FILL_ALPHA_THRESHOLD
            ) {
                pixels_left.push(int_point(col.x, col.y + 1));
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Unloads the editor from memory.
 */
void animation_editor::unload() {
    editor::unload();
    
    anims.destroy();
    
    unload_mob_types(false);
    unload_spike_damage_types();
    unload_hazards();
    unload_liquids();
    unload_spray_types();
    unload_status_types(false);
    unload_custom_particle_generators();
}


/* ----------------------------------------------------------------------------
 * Updates the current hitbox's transformation controller, based on whether
 * we're using the side view or not.
 */
void animation_editor::update_cur_hitbox_tc() {
    if(!cur_hitbox) return;
    if(side_view) {
        cur_hitbox_tc.set_center(
            point(
                cur_hitbox->pos.x,
                (-(cur_hitbox->height / 2.0)) - cur_hitbox->z
            )
        );
        cur_hitbox_tc.set_size(
            point(cur_hitbox->radius * 2, cur_hitbox->height)
        );
    } else {
        cur_hitbox_tc.set_center(cur_hitbox->pos);
        cur_hitbox_tc.set_size(
            point(cur_hitbox->radius * 2, cur_hitbox->radius * 2)
        );
    }
}


/* ----------------------------------------------------------------------------
 * Updates the current sprite's transformation controller.
 */
void animation_editor::update_cur_sprite_tc() {
    if(!cur_sprite) return;
    cur_sprite_tc.set_center(cur_sprite->offset);
    cur_sprite_tc.set_size(
        point(
            cur_sprite->file_size.x * cur_sprite->scale.x,
            cur_sprite->file_size.y * cur_sprite->scale.y
        )
    );
    cur_sprite_tc.set_angle(cur_sprite->angle);
}


/* ----------------------------------------------------------------------------
 * Updates the history list, by adding a new entry or bumping it up.
 */
void animation_editor::update_history(const string &n) {
    //First, check if it exists.
    size_t pos = INVALID;
    
    for(size_t h = 0; h < history.size(); ++h) {
        if(history[h] == n) {
            pos = h;
            break;
        }
    }
    
    if(pos == 0) {
        //Already #1? Never mind.
        return;
    } else if(pos == INVALID) {
        //If it doesn't exist, create it and add it to the top.
        history.insert(history.begin(), n);
    } else {
        //Otherwise, remove it from its spot and bump it to the top.
        history.erase(history.begin() + pos);
        history.insert(history.begin(), n);
    }
    
    if(history.size() > HISTORY_SIZE) {
        history.erase(history.begin() + history.size() - 1);
    }
}


/* ----------------------------------------------------------------------------
 * Update every frame's hitbox instances in light of new hitbox info.
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
                        name, INVALID, NULL, point(), 0,
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
        [this] (hitbox h1, hitbox h2) -> bool {
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
