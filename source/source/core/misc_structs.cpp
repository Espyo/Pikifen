/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Miscellaneous structures, too small
 * to warrant their own files.
 */

#undef _CMATH_

#include <algorithm>
#include <climits>
#include <iostream>

#include "misc_structs.h"

#include "../util/allegro_utils.h"
#include "../util/general_utils.h"
#include "../util/string_utils.h"
#include "const.h"
#include "drawing.h"
#include "game.h"
#include "load.h"
#include "misc_functions.h"


namespace GAMEPLAY_MSG_BOX {

//How quickly the advance button icon fades, in alpha (0-1) per second.
const float ADVANCE_BUTTON_FADE_SPEED = 4.0f;

//How many pixels of margin between the message box and screen borders.
const float MARGIN = 16.0f;

//How long to protect the player from misinputs for.
const float MISINPUT_PROTECTION_DURATION = 0.75f;

//How many pixels of padding between the message box borders and text.
const float PADDING = 8.0f;

//How long each token animates for when being shown.
const float TOKEN_ANIM_DURATION = 0.5f;

//How much to move a token in the X direction when animating it.
const float TOKEN_ANIM_X_AMOUNT = 7.0f;

//How much to move a token in the Y direction when animating it.
const float TOKEN_ANIM_Y_AMOUNT = 3.0f;

//How long to swipe the current section's tokens away for.
const float TOKEN_SWIPE_DURATION = 0.45f;

//How much to move a token in the X direction when swiping it away.
const float TOKEN_SWIPE_X_AMOUNT = -2.0f;

//How much to move a token in the Y direction when swiping it away.
const float TOKEN_SWIPE_Y_AMOUNT = -15.0f;

}


namespace NOTIFICATION {

//How quickly it fades, in alpha per second.
const float FADE_SPEED = 4.0f;

}


namespace WHISTLE {

//R, G, and B components for each dot color.
const unsigned char DOT_COLORS[N_DOT_COLORS][3] = {
    {214, 25,  25 }, //Red.
    {242, 134, 48 }, //Orange.
    {143, 227, 58 }, //Lime.
    {55,  222, 222}, //Cyan.
    {30,  30,  219}, //Blue.
    {133, 28,  237}, //Purple.
};

//Seconds that need to pass before another dot is added.
const float DOT_INTERVAL = 0.03;

//A whistle dot spins these many radians a second.
const float DOT_SPIN_SPEED = TAU / 4;

//Time the whistle animations take to fade out.
const float FADE_TIME = 0.13f;

//R, G, and B components for each ring color.
const unsigned char RING_COLORS[N_RING_COLORS][3] = {
    {255, 255, 0  },
    {255, 0,   0  },
    {255, 0,   255},
    {128, 0,   255},
    {0,   0,   255},
    {0,   255, 255},
    {0,   255, 0  },
    {128, 255, 0  }
};

//Whistle rings move these many units per second.
const float RING_SPEED = 600.0f;

//Seconds that need to pass before another whistle ring appears.
const float RINGS_INTERVAL = 0.1f;

}


namespace MAKER_TOOLS {

//Internal names of each maker tool.
const string NAMES[N_MAKER_TOOLS] = {
    "",
    "area_image",
    "change_speed",
    "collision",
    "geometry_info",
    "hitboxes",
    "hud",
    "hurt_mob",
    "mob_info",
    "new_pikmin",
    "path_info",
    "set_song_pos_near_loop",
    "teleport"
};

}


/**
 * @brief Loads the system content names from a file.
 *
 * @param file File to load from.
 */
void SystemContentNames::load(DataNode* file) {
    ReaderSetter gra_rs(file->getChildByName("graphics"));
    
    gra_rs.set("bright_circle", bmp_bright_circle);
    gra_rs.set("bright_ring", bmp_bright_ring);
    gra_rs.set("bubble_box", bmp_bubble_box);
    gra_rs.set("button_box", bmp_button_box);
    gra_rs.set("checkbox_check", bmp_checkbox_check);
    gra_rs.set("checkbox_no_check", bmp_checkbox_no_check);
    gra_rs.set("cursor", bmp_cursor);
    gra_rs.set("discord_icon", bmp_discord_icon);
    gra_rs.set("editor_icons", bmp_editor_icons);
    gra_rs.set("enemy_spirit", bmp_enemy_spirit);
    gra_rs.set("focus_box", bmp_focus_box);
    gra_rs.set("frame_box", bmp_frame_box);
    gra_rs.set("github_icon", bmp_github_icon);
    gra_rs.set("hard_bubble", bmp_hard_bubble);
    gra_rs.set("icon", bmp_icon);
    gra_rs.set("idle_glow", bmp_idle_glow);
    gra_rs.set("key_box", bmp_key_box);
    gra_rs.set("leader_silhouette_side", bmp_leader_silhouette_side);
    gra_rs.set("leader_silhouette_top", bmp_leader_silhouette_top);
    gra_rs.set("medal_bronze", bmp_medal_bronze);
    gra_rs.set("medal_gold", bmp_medal_gold);
    gra_rs.set("medal_none", bmp_medal_none);
    gra_rs.set("medal_platinum", bmp_medal_platinum);
    gra_rs.set("medal_silver", bmp_medal_silver);
    gra_rs.set("menu_icons", bmp_menu_icons);
    gra_rs.set("mission_clear", bmp_mission_clear);
    gra_rs.set("mission_fail", bmp_mission_fail);
    gra_rs.set("more", bmp_more);
    gra_rs.set("mouse_cursor", bmp_mouse_cursor);
    gra_rs.set("notification", bmp_notification);
    gra_rs.set("pikmin_spirit", bmp_pikmin_spirit);
    gra_rs.set("player_input_icons", bmp_player_input_icons);
    gra_rs.set("random", bmp_random);
    gra_rs.set("rock", bmp_rock);
    gra_rs.set("shadow", bmp_shadow);
    gra_rs.set("shadow_square", bmp_shadow_square);
    gra_rs.set("smack", bmp_smack);
    gra_rs.set("smoke", bmp_smoke);
    gra_rs.set("sparkle", bmp_sparkle);
    gra_rs.set("spotlight", bmp_spotlight);
    gra_rs.set("swarm_arrow", bmp_swarm_arrow);
    gra_rs.set("throw_invalid", bmp_throw_invalid);
    gra_rs.set("throw_preview", bmp_throw_preview);
    gra_rs.set("throw_preview_dashed", bmp_throw_preview_dashed);
    gra_rs.set("title_screen_bg", bmp_title_screen_bg);
    gra_rs.set("wave_ring", bmp_wave_ring);
    
    ReaderSetter fnt_rs(file->getChildByName("fonts"));
    
    fnt_rs.set("area_name", fnt_area_name);
    fnt_rs.set("counter", fnt_counter);
    fnt_rs.set("cursor_counter", fnt_cursor_counter);
    fnt_rs.set("editor_header", fnt_editor_header);
    fnt_rs.set("editor_monospace", fnt_editor_monospace);
    fnt_rs.set("editor_standard", fnt_editor_standard);
    fnt_rs.set("slim", fnt_slim);
    fnt_rs.set("standard", fnt_standard);
    fnt_rs.set("value", fnt_value);
    
    ReaderSetter snd_rs(file->getChildByName("sounds"));
    
    snd_rs.set("attack", sound_attack);
    snd_rs.set("camera", sound_camera);
    snd_rs.set("menu_activate", sound_menu_activate);
    snd_rs.set("menu_back", sound_menu_back);
    snd_rs.set("menu_select", sound_menu_select);
    snd_rs.set("switch_pikmin", sound_switch_pikmin);
    
    ReaderSetter sng_rs(file->getChildByName("songs"));
    
    sng_rs.set("boss", sng_boss);
    sng_rs.set("boss_victory", sng_boss_victory);
    sng_rs.set("editors", sng_editors);
    sng_rs.set("menus", sng_menus);
    
    ReaderSetter ani_rs(file->getChildByName("animations"));
    
    ani_rs.set("sparks", anim_sparks);
    
    ReaderSetter par_rs(file->getChildByName("particle_generators"));
    
    par_rs.set("converter_insertion", part_converter_insertion);
    par_rs.set("ding", part_ding);
    par_rs.set("enemy_death", part_enemy_death);
    par_rs.set("leader_heal", part_leader_heal);
    par_rs.set("leader_land", part_leader_land);
    par_rs.set("onion_generating_inside", part_onion_gen_inside);
    par_rs.set("onion_insertion", part_onion_insertion);
    par_rs.set("pikmin_pluck_dirt", part_pikmin_pluck_dirt);
    par_rs.set("pikmin_seed_landed", part_pikmin_seed_landed);
    par_rs.set("smack", part_smack);
    par_rs.set("spray", part_spray);
    par_rs.set("sprout_evolution", part_sprout_evolution);
    par_rs.set("sprout_regression", part_sprout_regression);
    par_rs.set("throw_trail", part_throw_trail);
    par_rs.set("treasure", part_treasure);
    par_rs.set("wave_ring", part_wave_ring);
}


/**
 * @brief Loads an audio stream for the manager.
 *
 * @param name Name of the audio stream to load.
 * @param node If not nullptr, blame this data node if the file doesn't exist.
 * @param report_errors Only issues errors if this is true.
 * @return The audio stream.
 */
ALLEGRO_AUDIO_STREAM* AudioStreamManager::do_load(
    const string &name, DataNode* node, bool report_errors
) {
    const auto &it = game.content.song_tracks.manifests.find(name);
    string path =
        it != game.content.song_tracks.manifests.end() ?
        it->second.path :
        name;
    ALLEGRO_AUDIO_STREAM* stream = load_audio_stream(path, node, report_errors);
    if(stream) {
        game.register_audio_stream_source(stream);
    }
    return stream;
}


/**
 * @brief Unloads an audio stream for the manager.
 *
 * @param asset Audio stream to unload.
 */
void AudioStreamManager::do_unload(ALLEGRO_AUDIO_STREAM* asset) {
    al_drain_audio_stream(asset);
    game.unregister_audio_stream_source(asset);
    al_destroy_audio_stream(asset);
}


/**
 * @brief Loads a bitmap for the manager.
 *
 * @param name Name of the bitmap to load.
 * @param node If not nullptr, blame this data node if the file doesn't exist.
 * @param report_errors Only issues errors if this is true.
 * @return The bitmap.
 */
ALLEGRO_BITMAP* BitmapManager::do_load(
    const string &name, DataNode* node, bool report_errors
) {
    const auto &it = game.content.bitmaps.manifests.find(name);
    string path =
        it != game.content.bitmaps.manifests.end() ?
        it->second.path :
        name;
    return load_bmp(path, node, report_errors);
}


/**
 * @brief Unloads a bitmap for the manager.
 *
 * @param asset Bitmap to unload.
 */
void BitmapManager::do_unload(ALLEGRO_BITMAP* asset) {
    if(asset != game.bmp_error) {
        al_destroy_bitmap(asset);
    }
}


/**
 * @brief Instantly places the camera at the specified coordinates.
 *
 * @param new_pos Coordinates to place the camera at.
 */
void Camera::set_pos(const Point &new_pos) {
    pos = new_pos;
    target_pos = new_pos;
}


/**
 * @brief Instantly places the camera at the specified zoom.
 *
 * @param new_zoom Zoom to set to.
 */
void Camera::set_zoom(float new_zoom) {
    zoom = new_zoom;
    target_zoom = new_zoom;
}


/**
 * @brief Ticks camera movement by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void Camera::tick(float delta_t) {
    pos.x +=
        (target_pos.x - pos.x) * (GAMEPLAY::CAMERA_SMOOTHNESS_MULT * delta_t);
    pos.y +=
        (target_pos.y - pos.y) * (GAMEPLAY::CAMERA_SMOOTHNESS_MULT * delta_t);
    zoom +=
        (target_zoom - zoom) * (GAMEPLAY::CAMERA_SMOOTHNESS_MULT * delta_t);
}


/**
 * @brief Updates the camera's visibility box,
 * based on the screen_to_world_transform transformation.
 */
void Camera::update_box() {
    box[0] = Point(0.0f);
    box[1] = Point(game.win_w, game.win_h);
    al_transform_coordinates(
        &game.screen_to_world_transform,
        &box[0].x, &box[0].y
    );
    al_transform_coordinates(
        &game.screen_to_world_transform,
        &box[1].x, &box[1].y
    );
    
    game.audio.set_camera_pos(box[0], box[1]);
    
    box[0].x -= GAMEPLAY::CAMERA_BOX_MARGIN;
    box[0].y -= GAMEPLAY::CAMERA_BOX_MARGIN;
    box[1].x += GAMEPLAY::CAMERA_BOX_MARGIN;
    box[1].y += GAMEPLAY::CAMERA_BOX_MARGIN;
}


/**
 * @brief Emits an error in the gameplay "info" window.
 *
 * @param s Full error description.
 */
void ErrorManager::emit_in_gameplay(const string &s) {
    string info_str =
        "\n\n\n"
        "ERROR: " + s + "\n\n"
        "(Saved to \"" + FILE_PATHS_FROM_ROOT::ERROR_LOG + "\".)\n\n";
    print_info(info_str, 30.0f, 3.0f);
}


/**
 * @brief Logs an error to stdout (i.e. the console).
 *
 * @param s Full error description.
 */
void ErrorManager::log_to_console(const string &s) {
    std::cout << s << std::endl;
}


/**
 * @brief Logs an error to the log file.
 *
 * @param s Full error description.
 */
void ErrorManager::log_to_file(const string &s) {
    string prev_error_log;
    string output = "";
    
    //Get the previous contents of the log file, if any.
    ALLEGRO_FILE* file_i =
        al_fopen(FILE_PATHS_FROM_ROOT::ERROR_LOG.c_str(), "r");
    if(file_i) {
        while(!al_feof(file_i)) {
            string line;
            getline(file_i, line);
            prev_error_log += line + "\n";
        }
        prev_error_log.erase(prev_error_log.size() - 1);
        al_fclose(file_i);
    }
    
    //Write this session's header, if necessary.
    if(nr_session_errors == 0) {
        string header;
        if(!prev_error_log.empty()) {
            header += "\n\n";
        }
        header += "Pikifen version " + get_engine_version_string();
        if(!game.config.version.empty()) {
            header +=
                ", " + game.config.name + " version " + game.config.version;
        }
        header += ":\n";
        output += header;
    }
    
    //Log this error.
    vector<string> lines = split(s, "\n");
    output += "  " + get_current_time(false) + ": " + lines[0] + "\n";
    for(size_t l = 1; l < lines.size(); l++) {
        output += "  " + lines[l] + "\n";
    }
    
    //Save it.
    ALLEGRO_FILE* file_o =
        al_fopen(FILE_PATHS_FROM_ROOT::ERROR_LOG.c_str(), "w");
    if(file_o) {
        al_fwrite(file_o, prev_error_log + output);
        al_fclose(file_o);
    }
}


/**
 * @brief Prepares everything for an area load.
 */
void ErrorManager::prepare_area_load() {
    nr_errors_on_area_load = nr_session_errors;
    first_area_load_error.clear();
}


/**
 * @brief Reports an error to the user and logs it.
 *
 * @param s String that represents the error.
 * @param d If not null, this will be used to obtain the file name
 * and line that caused the error.
 */
void ErrorManager::report(const string &s, const DataNode* d) {
    string full_error = s;
    if(d) {
        full_error += " (" + d->filePath;
        if(d->lineNr != 0) full_error += " line " + i2s(d->lineNr);
        full_error += ")";
    }
    
    if(first_area_load_error.empty()) first_area_load_error = full_error;
    
    log_to_console(full_error);
    log_to_file(full_error);
    emit_in_gameplay(full_error);
    
    nr_session_errors++;
}


/**
 * @brief Reports to the gameplay "info" window if any errors happened during
 * area load.
 * This will override whatever is in the "info" window, which is likely
 * the latest error, but that's okay since this information is more important.
 */
void ErrorManager::report_area_load_errors() {
    if(nr_session_errors <= nr_errors_on_area_load) return;
    
    size_t nr_errors_found =
        nr_session_errors - nr_errors_on_area_load;
        
    string info_str =
        "\n\n\n"
        "ERROR: " + first_area_load_error + "\n\n";
    if(nr_errors_found > 1) {
        info_str += "(+" + i2s(nr_errors_found - 1) + " more) ";
    }
    info_str +=
        "(Saved to \"" + FILE_PATHS_FROM_ROOT::ERROR_LOG + "\".)\n\n";
        
    print_info(info_str, 30.0f, 3.0f);
}


/**
 * @brief Returns whether this session has had any error reports.
 *
 * @return Whether it had errors.
 */
bool ErrorManager::session_has_errors() {
    return nr_session_errors > 0;
}


/**
 * @brief Draws the fade overlay, if there is a fade in progress.
 */
void FadeManager::draw() {
    if(is_fading()) {
        unsigned char alpha = (game.fade_mgr.get_perc_left()) * 255;
        al_draw_filled_rectangle(
            0, 0, game.win_w, game.win_h,
            al_map_rgba(
                0, 0, 0, (game.fade_mgr.is_fade_in() ? alpha : 255 - alpha)
            )
        );
    }
}


/**
 * @brief Returns the percentage of progress left in the current fade.
 *
 * @return The percentage.
 */
float FadeManager::get_perc_left() const {
    return time_left / GAME::FADE_DURATION;
}


/**
 * @brief Returns whether the current fade is a fade in or fade out.
 *
 * @return Whether it is a fade in.
 */
bool FadeManager::is_fade_in() const {
    return fade_in;
}


/**
 * @brief Returns whether or not a fade is currently in progress.
 *
 * @return Whether it is in progress.
 */
bool FadeManager::is_fading() const {
    return time_left > 0;
}


/**
 * @brief Sets up the start of a fade.
 *
 * @param is_fade_in If true, this fades in. If false, fades out.
 * @param on_end Code to run when the fade finishes.
 */
void FadeManager::start_fade(
    bool is_fade_in, const std::function<void()> &on_end
) {
    time_left = GAME::FADE_DURATION;
    fade_in = is_fade_in;
    this->on_end = on_end;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void FadeManager::tick(float delta_t) {
    if(time_left == 0) return;
    time_left -= delta_t;
    if(time_left <= 0) {
        time_left = 0;
        if(on_end) on_end();
    }
}


/**
 * @brief Constructs a new maker tools info object.
 */
MakerTools::MakerTools() :
    enabled(true),
    area_image_padding(32.0f),
    area_image_shadows(true),
    area_image_size(2048),
    area_image_mobs(true),
    change_speed(false),
    change_speed_mult(2.0f),
    collision(false),
    geometry_info(false),
    hitboxes(false),
    hud(true),
    info_lock(nullptr),
    info_print_duration(5.0f),
    info_print_fade_duration(3.0f),
    last_pikmin_type(nullptr),
    mob_hurting_ratio(0.75),
    path_info(false),
    use_perf_mon(false),
    used_helping_tools(false) {
    
    info_print_timer = Timer(1.0f, [this] () { info_print_text.clear(); });
    for(size_t k = 0; k < 20; k++) {
        keys[k] = MAKER_TOOL_TYPE_NONE;
    }
}


/**
 * @brief Resets the states of the tools so that players can play without any
 * tool affecting the experience.
 */
void MakerTools::reset_for_gameplay() {
    change_speed = false;
    collision = false;
    geometry_info = false;
    hitboxes = false;
    hud = true;
    info_lock = nullptr;
    last_pikmin_type = nullptr;
    path_info = false;
    used_helping_tools = false;
}


/**
 * @brief Hides the OS mouse in the game window.
 */
void MouseCursor::hide() const {
    al_hide_mouse_cursor(game.display);
}


/**
 * @brief Initializes everything.
 */
void MouseCursor::init() {
    hide();
    reset();
    
    save_timer.on_end = [this] () {
        save_timer.start();
        history.push_back(s_pos);
        if(history.size() > GAME::CURSOR_TRAIL_SAVE_N_SPOTS) {
            history.erase(history.begin());
        }
    };
    save_timer.start(GAME::CURSOR_TRAIL_SAVE_INTERVAL);
}


/**
 * @brief Resets the cursor's state.
 */
void MouseCursor::reset() {
    ALLEGRO_MOUSE_STATE mouse_state;
    al_get_mouse_state(&mouse_state);
    game.mouse_cursor.s_pos.x = al_get_mouse_state_axis(&mouse_state, 0);
    game.mouse_cursor.s_pos.y = al_get_mouse_state_axis(&mouse_state, 1);
    game.mouse_cursor.w_pos = game.mouse_cursor.s_pos;
    al_transform_coordinates(
        &game.screen_to_world_transform,
        &game.mouse_cursor.w_pos.x, &game.mouse_cursor.w_pos.y
    );
    history.clear();
}


/**
 * @brief Shows the OS mouse in the game window.
 */
void MouseCursor::show() const {
    al_show_mouse_cursor(game.display);
}


/**
 * @brief Updates the coordinates from an Allegro mouse event.
 *
 * @param ev Event to handle.
 * @param screen_to_world_transform Transformation to use to get the
 * screen coordinates.
 */
void MouseCursor::update_pos(
    const ALLEGRO_EVENT &ev,
    ALLEGRO_TRANSFORM &screen_to_world_transform
) {
    s_pos.x = ev.mouse.x;
    s_pos.y = ev.mouse.y;
    w_pos = s_pos;
    al_transform_coordinates(
        &screen_to_world_transform,
        &w_pos.x, &w_pos.y
    );
}


/**
 * @brief Draws the notification on-screen.
 */
void Notification::draw() const {
    if(visibility == 0.0f) return;
    
    float scale = ease(EASE_METHOD_OUT, visibility);
    
    ALLEGRO_TRANSFORM tra, old_tra;
    al_identity_transform(&tra);
    al_scale_transform(&tra, scale, scale);
    al_translate_transform(
        &tra,
        pos.x * game.cam.zoom,
        pos.y * game.cam.zoom
    );
    al_scale_transform(
        &tra,
        1.0f / game.cam.zoom,
        1.0f / game.cam.zoom
    );
    al_copy_transform(&old_tra, al_get_current_transform());
    al_compose_transform(&tra, &old_tra);
    al_use_transform(&tra);
    
    int bmp_w = al_get_bitmap_width(game.sys_content.bmp_notification);
    int bmp_h = al_get_bitmap_height(game.sys_content.bmp_notification);
    
    float text_box_x1 = -bmp_w * 0.5 + DRAWING::NOTIFICATION_PADDING;
    float text_box_x2 = bmp_w * 0.5 - DRAWING::NOTIFICATION_PADDING;
    float text_box_y1 = -bmp_h - DRAWING::NOTIFICATION_PADDING;
    float text_box_y2 = DRAWING::NOTIFICATION_PADDING;
    
    draw_bitmap(
        game.sys_content.bmp_notification,
        Point(0, -bmp_h * 0.5),
        Point(bmp_w, bmp_h),
        0,
        map_alpha(DRAWING::NOTIFICATION_ALPHA * visibility)
    );
    
    if(input.type != INPUT_TYPE_NONE) {
        text_box_x1 +=
            DRAWING::NOTIFICATION_CONTROL_SIZE + DRAWING::NOTIFICATION_PADDING;
        draw_player_input_icon(
            game.sys_content.fnt_slim, input,
            true,
            Point(
                -bmp_w * 0.5 + DRAWING::NOTIFICATION_PADDING +
                DRAWING::NOTIFICATION_CONTROL_SIZE * 0.5,
                -bmp_h * 0.5
            ),
            Point(
                DRAWING::NOTIFICATION_CONTROL_SIZE,
                DRAWING::NOTIFICATION_CONTROL_SIZE
            ),
            visibility * 255
        );
    }
    
    draw_text(
        text, game.sys_content.fnt_standard,
        Point(
            (text_box_x1 + text_box_x2) * 0.5,
            (text_box_y1 + text_box_y2) * 0.5
        ),
        Point(
            text_box_x2 - text_box_x1,
            text_box_y2 - text_box_y1
        ),
        map_alpha(DRAWING::NOTIFICATION_ALPHA * visibility),
        ALLEGRO_ALIGN_CENTER, V_ALIGN_MODE_CENTER, TEXT_SETTING_FLAG_CANT_GROW
    );
    
    al_use_transform(&old_tra);
}


/**
 * @brief Returns how "present" the notification is.
 * 0 means hidden, 1 is fully visible. Mid values are transition.
 *
 * @return The visibility.
 */
float Notification::get_visibility() const {
    return visibility;
}


/**
 * @brief Resets the whole thing.
 */
void Notification::reset() {
    enabled = true;
    input.type = INPUT_TYPE_NONE;
    text.clear();
    pos = Point();
    visibility = 0.0f;
}


/**
 * @brief Sets the contents to show.
 *
 * @param input Player input icon to show.
 * @param text Text to show.
 * @param pos Where to show it in the game world.
 */
void Notification::set_contents(
    const PlayerInput &input, const string &text, const Point &pos
) {
    this->input = input;
    this->text = text;
    this->pos = pos;
}


/**
 * @brief Sets whether the notification is meant to show or not.
 *
 * @param enabled Whether it's enabled or not.
 */
void Notification::set_enabled(bool enabled) {
    this->enabled = enabled;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void Notification::tick(float delta_t) {
    if(enabled) {
        visibility += NOTIFICATION::FADE_SPEED * delta_t;
    } else {
        visibility -= NOTIFICATION::FADE_SPEED * delta_t;
    }
    visibility = clamp(visibility, 0.0f, 1.0f);
}


/**
 * @brief Constructs a new performance monitor struct object.
 */
PerformanceMonitor::PerformanceMonitor() :
    cur_state(PERF_MON_STATE_LOADING),
    paused(false),
    cur_state_start_time(0.0),
    cur_measurement_start_time(0.0),
    frame_samples(0) {
    
    reset();
}


/**
 * @brief Enters the given state of the monitoring process.
 *
 * @param state New state.
 */
void PerformanceMonitor::enter_state(const PERF_MON_STATE state) {
    if(paused) return;
    
    cur_state = state;
    cur_state_start_time = al_get_time();
    cur_page = Page();
    
    if(cur_state == PERF_MON_STATE_FRAME) {
        frame_samples++;
    }
}


/**
 * @brief Finishes the latest measurement.
 */
void PerformanceMonitor::finish_measurement() {
    if(paused) return;
    
    //Check if we were measuring something.
    engine_assert(
        cur_measurement_start_time != 0.0,
        cur_page.measurements.empty() ?
        "(No measurements)" :
        "Last measurement: " + cur_page.measurements.back().first
    );
    
    double dur = al_get_time() - cur_measurement_start_time;
    bool is_new = true;
    
    for(size_t m = 0; m < cur_page.measurements.size(); m++) {
        if(cur_page.measurements[m].first == cur_measurement_name) {
            cur_page.measurements[m].second += dur;
            is_new = false;
            break;
        }
    }
    if(is_new) {
        cur_page.measurements.push_back(
            std::make_pair(cur_measurement_name, dur)
        );
    }
    
    cur_measurement_start_time = 0.0;
}


/**
 * @brief Leaves the current state of the monitoring process.
 */
void PerformanceMonitor::leave_state() {
    if(paused) return;
    
    cur_page.duration = al_get_time() - cur_state_start_time;
    
    switch(cur_state) {
    case PERF_MON_STATE_LOADING: {
        loading_page = cur_page;
        break;
    }
    case PERF_MON_STATE_FRAME: {
        if(
            frame_fastest_page.duration == 0.0 ||
            cur_page.duration < frame_fastest_page.duration
        ) {
            frame_fastest_page = cur_page;
            
        } else if(
            frame_slowest_page.duration == 0.0 ||
            cur_page.duration > frame_slowest_page.duration
        ) {
            frame_slowest_page = cur_page;
            
        }
        
        if(frame_avg_page.duration == 0.0) {
            frame_avg_page = cur_page;
        } else {
            frame_avg_page.duration += cur_page.duration;
            for(size_t m = 0; m < cur_page.measurements.size(); m++) {
                bool is_new = true;
                for(
                    size_t m2 = 0;
                    m2 < frame_avg_page.measurements.size(); m2++
                ) {
                    if(
                        cur_page.measurements[m].first ==
                        frame_avg_page.measurements[m2].first
                    ) {
                        frame_avg_page.measurements[m2].second +=
                            cur_page.measurements[m].second;
                        is_new = false;
                        break;
                    }
                }
                if(is_new) {
                    frame_avg_page.measurements.push_back(
                        cur_page.measurements[m]
                    );
                }
            }
        }
        break;
        
    }
    }
}


/**
 * @brief Resets all of the performance monitor's information.
 */
void PerformanceMonitor::reset() {
    area_name.clear();
    cur_state = PERF_MON_STATE_LOADING;
    paused = false;
    cur_state_start_time = 0.0;
    cur_measurement_start_time = 0.0;
    cur_measurement_name.clear();
    cur_page = Page();
    frame_samples = 0;
    loading_page = Page();
    frame_avg_page = Page();
    frame_fastest_page = Page();
    frame_slowest_page = Page();
}


/**
 * @brief Saves a log file with all known stats, if there is anything to save.
 */
void PerformanceMonitor::save_log() {
    if(loading_page.measurements.empty()) {
        //Nothing to save.
        return;
    }
    
    //Average out the frames of gameplay.
    frame_avg_page.duration /= (double) frame_samples;
    for(size_t m = 0; m < frame_avg_page.measurements.size(); m++) {
        frame_avg_page.measurements[m].second /= (double) frame_samples;
    }
    
    //Fill out the string.
    string s =
        "\n" +
        get_current_time(false) +
        "; Pikifen version " + get_engine_version_string();
    if(!game.config.version.empty()) {
        s += ", game version " + game.config.version;
    }
    
    s +=
        "\nData from the latest played area, " + area_name + ", with " +
        i2s(frame_samples) + " gameplay frames sampled.\n";
        
    s += "\nLoading times:\n";
    loading_page.write(s);
    
    s += "\nAverage frame processing times:\n";
    frame_avg_page.write(s);
    
    s += "\nFastest frame processing times:\n";
    frame_fastest_page.write(s);
    
    s += "\nSlowest frame processing times:\n";
    frame_slowest_page.write(s);
    
    //Finally, write the string to a file.
    string prev_log;
    ALLEGRO_FILE* file_i =
        al_fopen(FILE_PATHS_FROM_ROOT::PERFORMANCE_LOG.c_str(), "r");
    if(file_i) {
        string line;
        while(!al_feof(file_i)) {
            getline(file_i, line);
            prev_log += line + "\n";
        }
        prev_log.erase(prev_log.size() - 1);
        al_fclose(file_i);
    }
    
    ALLEGRO_FILE* file_o =
        al_fopen(FILE_PATHS_FROM_ROOT::PERFORMANCE_LOG.c_str(), "w");
    if(file_o) {
        al_fwrite(file_o, prev_log + s);
        al_fclose(file_o);
    }
}


/**
 * @brief Sets the name of the area that was monitored.
 *
 * @param name Name of the area.
 */
void PerformanceMonitor::set_area_name(const string &name) {
    area_name = name;
}


/**
 * @brief Sets whether monitoring is currently paused or not.
 *
 * @param paused Pause value.
 */
void PerformanceMonitor::set_paused(bool paused) {
    this->paused = paused;
}


/**
 * @brief Starts measuring a certain point in the loading procedure.
 *
 * @param name Name of the measurement.
 */
void PerformanceMonitor::start_measurement(const string &name) {
    if(paused) return;
    
    //Check if we were already measuring something.
    engine_assert(
        cur_measurement_start_time == 0.0,
        cur_page.measurements.empty() ?
        "(No measurements)" :
        "Last measurement: " + cur_page.measurements.back().first
    );
    
    cur_measurement_start_time = al_get_time();
    cur_measurement_name = name;
}


/**
 * @brief Writes a page of information to a string.
 *
 * @param s String to write to.
 */
void PerformanceMonitor::Page::write(string &s) {
    //Get the total measured time.
    double total_measured_time = 0.0;
    for(size_t m = 0; m < measurements.size(); m++) {
        total_measured_time += measurements[m].second;
    }
    
    //Write each measurement into the string.
    for(size_t m = 0; m < measurements.size(); m++) {
        write_measurement(
            s, measurements[m].first,
            measurements[m].second,
            total_measured_time
        );
    }
    
    //Write the total.
    s +=
        "  TOTAL: " + std::to_string(duration) + "s (" +
        std::to_string(total_measured_time) + "s measured, " +
        std::to_string(duration - total_measured_time) + "s not measured).\n";
}


/**
 * @brief Writes a measurement in a human-friendly format onto a string.
 *
 * @param str The string to write to.
 * @param name The name of this measurement.
 * @param dur How long it lasted for, in seconds.
 * @param total How long the entire procedure lasted for.
 */
void PerformanceMonitor::Page::write_measurement(
    string &str, const string &name, double dur, float total
) {
    float perc = dur / total * 100.0;
    str +=
        "  " + name + "\n" +
        "    " + box_string(std::to_string(dur), 8, "s") +
        " (" + f2s(perc) + "%)\n    ";
    for(unsigned char p = 0; p < 100; p++) {
        if(p < perc) {
            str.push_back('#');
        } else {
            str.push_back('_');
        }
    }
    str += "\n";
}


/**
 * @brief Constructs a new reader setter object.
 *
 * @param dn Pointer to the base data node.
 */
ReaderSetter::ReaderSetter(DataNode* dn) :
    node(dn) {
    
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param child Name of the child node.
 * @param var The var to set. This is an Allegro color.
 * @param child_node If not-nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &child, ALLEGRO_COLOR &var, DataNode** child_node
) {
    DataNode* n = node->getChildByName(child);
    if(!n->value.empty()) {
        if(child_node) *child_node = n;
        var = s2c(n->value);
    } else {
        if(child_node) *child_node = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param child Name of the child node.
 * @param var The var to set. This is a string.
 * @param child_node If not-nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &child, string &var, DataNode** child_node
) {
    DataNode* n = node->getChildByName(child);
    if(!n->value.empty()) {
        if(child_node) *child_node = n;
        var = n->value;
    } else {
        if(child_node) *child_node = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param child Name of the child node.
 * @param var The var to set. This is an integer.
 * @param child_node If not-nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &child, size_t &var, DataNode** child_node
) {
    DataNode* n = node->getChildByName(child);
    if(!n->value.empty()) {
        if(child_node) *child_node = n;
        var = s2i(n->value);
    } else {
        if(child_node) *child_node = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param child Name of the child node.
 * @param var The var to set. This is an integer.
 * @param child_node If not-nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &child, int &var, DataNode** child_node
) {
    DataNode* n = node->getChildByName(child);
    if(!n->value.empty()) {
        if(child_node) *child_node = n;
        var = s2i(n->value);
    } else {
        if(child_node) *child_node = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param child Name of the child node.
 * @param var The var to set. This is an unsigned integer.
 * @param child_node If not-nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &child, unsigned int &var, DataNode** child_node
) {
    DataNode* n = node->getChildByName(child);
    if(!n->value.empty()) {
        if(child_node) *child_node = n;
        var = s2i(n->value);
    } else {
        if(child_node) *child_node = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param child Name of the child node.
 * @param var The var to set. This is an unsigned char.
 * @param child_node If not-nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &child, unsigned char &var, DataNode** child_node
) {
    DataNode* n = node->getChildByName(child);
    if(!n->value.empty()) {
        if(child_node) *child_node = n;
        var = s2i(n->value);
    } else {
        if(child_node) *child_node = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param child Name of the child node.
 * @param var The var to set. This is a boolean.
 * @param child_node If not-nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &child, bool &var, DataNode** child_node
) {
    DataNode* n = node->getChildByName(child);
    if(!n->value.empty()) {
        if(child_node) *child_node = n;
        var = s2b(n->value);
    } else {
        if(child_node) *child_node = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param child Name of the child node.
 * @param var The var to set. This is a float.
 * @param child_node If not-nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &child, float &var, DataNode** child_node
) {
    DataNode* n = node->getChildByName(child);
    if(!n->value.empty()) {
        if(child_node) *child_node = n;
        var = s2f(n->value);
    } else {
        if(child_node) *child_node = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param child Name of the child node.
 * @param var The var to set. This is a double.
 * @param child_node If not-nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &child, double &var, DataNode** child_node
) {
    DataNode* n = node->getChildByName(child);
    if(!n->value.empty()) {
        if(child_node) *child_node = n;
        var = s2f(n->value);
    } else {
        if(child_node) *child_node = nullptr;
    }
}


/**
 * @brief Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 *
 * @param child Name of the child node.
 * @param var The var to set. This is a point.
 * @param child_node If not-nullptr, the node from whence the value came
 * is placed here. nullptr is placed if the property does not exist or has
 * no value.
 */
void ReaderSetter::set(
    const string &child, Point &var, DataNode** child_node
) {
    DataNode* n = node->getChildByName(child);
    if(!n->value.empty()) {
        if(child_node) *child_node = n;
        var = s2p(n->value);
    } else {
        if(child_node) *child_node = nullptr;
    }
}



/**
 * @brief Returns a random float between the provided range, inclusive.
 *
 * @param minimum Minimum value that can be generated, inclusive.
 * @param maximum Maximum value that can be generated, inclusive.
 * @return The random number.
 */
float RngManager::f(float minimum, float maximum) {
    if(minimum == maximum) return minimum;
    if(minimum > maximum) std::swap(minimum, maximum);
    
    std::uniform_real_distribution<float> dist(
        minimum, std::nextafter(maximum, FLT_MAX)
    );
    return dist(main_rng);
}


/**
 * @brief Returns a random integer between the provided range, inclusive.
 *
 * @param minimum Minimum value that can be generated, inclusive.
 * @param maximum Maximum value that can be generated, inclusive.
 * @return The random number.
 */
int RngManager::i(int minimum, int maximum) {
    if(minimum == maximum) return minimum;
    if(minimum > maximum) std::swap(minimum, maximum);
    
    std::uniform_int_distribution<int> dist(minimum, maximum);
    return dist(main_rng);
}


/**
 * @brief Initializes the random number generator, using the current time as
 * the seed.
 */
void RngManager::init() {
    init(time(nullptr));
}


/**
 * @brief Initializes the random number generator with the given seed.
 */
void RngManager::init(unsigned int initial_seed) {
    main_rng = std::mt19937(initial_seed);
}



/**
 * @brief Constructs a new script var reader object.
 *
 * @param vars Map of variables to read from.
 */
ScriptVarReader::ScriptVarReader(map<string, string> &vars) :
    vars(vars) {
    
}


/**
 * @brief Assigns an Allegro color to the value of a given variable,
 * if it exists.
 *
 * @param name Name of the variable to read.
 * @param dest Destination for the value.
 * @return Whether it exists.
 */
bool ScriptVarReader::get(const string &name, ALLEGRO_COLOR &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = s2c(v->second);
    return true;
}


/**
 * @brief Assigns a string to the value of a given variable, if it exists.
 *
 * @param name Name of the variable to read.
 * @param dest Destination for the value.
 * @return Whether it exists.
 */
bool ScriptVarReader::get(const string &name, string &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = v->second;
    return true;
}


/**
 * @brief Assigns a size_t to the value of a given variable, if it exists.
 *
 * @param name Name of the variable to read.
 * @param dest Destination for the value.
 * @return Whether it exists.
 */
bool ScriptVarReader::get(const string &name, size_t &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = s2i(v->second);
    return true;
}


/**
 * @brief Assigns an int to the value of a given variable, if it exists.
 *
 * @param name Name of the variable to read.
 * @param dest Destination for the value.
 * @return Whether it exists.
 */
bool ScriptVarReader::get(const string &name, int &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = s2i(v->second);
    return true;
}


/**
 * @brief Assigns an unsigned char to the value of a given variable,
 * if it exists.
 *
 * @param name Name of the variable to read.
 * @param dest Destination for the value.
 * @return Whether it exists.
 */
bool ScriptVarReader::get(const string &name, unsigned char &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = s2i(v->second);
    return true;
}


/**
 * @brief Assigns a bool to the value of a given variable, if it exists.
 *
 * @param name Name of the variable to read.
 * @param dest Destination for the value.
 * @return Whether it exists.
 */
bool ScriptVarReader::get(const string &name, bool &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = s2b(v->second);
    return true;
}


/**
 * @brief Assigns a float to the value of a given variable, if it exists.
 *
 * @param name Name of the variable to read.
 * @param dest Destination for the value.
 * @return Whether it exists.
 */
bool ScriptVarReader::get(const string &name, float &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = s2f(v->second);
    return true;
}


/**
 * @brief Assigns a point to the value of a given variable, if it exists.
 *
 * @param name Name of the variable to read.
 * @param dest Destination for the value.
 * @return Whether it exists.
 */
bool ScriptVarReader::get(const string &name, Point &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = s2p(v->second);
    return true;
}


/**
 * @brief Loads an audio sample for the manager.
 *
 * @param name Name of the audio sample to load.
 * @param node If not nullptr, blame this data node if the file doesn't exist.
 * @param report_errors Only issues errors if this is true.
 * @return The audio sample.
 */
ALLEGRO_SAMPLE* SampleManager::do_load(
    const string &name, DataNode* node, bool report_errors
) {
    const auto &it = game.content.sounds.manifests.find(name);
    string path =
        it != game.content.sounds.manifests.end() ?
        it->second.path :
        name;
    return load_sample(path, node, report_errors);
}


/**
 * @brief Unloads an audio sample for the manager.
 *
 * @param asset Audio sample to unload.
 */
void SampleManager::do_unload(ALLEGRO_SAMPLE* asset) {
    al_destroy_sample(asset);
}


/**
 * @brief Clears the list of registered subgroup types.
 */
void SubgroupTypeManager::clear() {
    for(size_t t = 0; t < types.size(); t++) {
        delete types[t];
    }
    types.clear();
}


/**
 * @brief Returns the first registered subgroup type.
 *
 * @return The first type.
 */
SubgroupType* SubgroupTypeManager::get_first_type() const {
    return types.front();
}


/**
 * @brief Returns the subgroup type that comes after the given type.
 *
 * @param sgt Subgroup type to iterate from.
 * @return The next type.
 */
SubgroupType* SubgroupTypeManager::get_next_type(
    const SubgroupType* sgt
) const {
    for(size_t t = 0; t < types.size(); t++) {
        if(types[t] == sgt) {
            return get_next_in_vector(types, t);
        }
    }
    return nullptr;
}


/**
 * @brief Returns the subgroup type that comes before the given type.
 *
 * @param sgt Subgroup type to iterate from.
 * @return The previous type.
 */
SubgroupType* SubgroupTypeManager::get_prev_type(
    const SubgroupType* sgt
) const {
    for(size_t t = 0; t < types.size(); t++) {
        if(types[t] == sgt) {
            return get_prev_in_vector(types, t);
        }
    }
    return nullptr;
}


/**
 * @brief Returns the type of subgroup corresponding to the parameters.
 *
 * @param category The category of subgroup type. Pikmin, leader,
 * bomb-rock, etc.
 * @param specific_type Specific type of mob, if you want to specify further.
 * @return The type, or nullptr if not found.
 */
SubgroupType* SubgroupTypeManager::get_type(
    const SUBGROUP_TYPE_CATEGORY category,
    const MobType* specific_type
) const {
    for(size_t t = 0; t < types.size(); t++) {
        SubgroupType* t_ptr = types[t];
        if(
            t_ptr->category == category &&
            t_ptr->specific_type == specific_type
        ) {
            return t_ptr;
        }
    }
    return nullptr;
}


/**
 * @brief Registers a new type of subgroup.
 *
 * @param category The category of subgroup type. Pikmin, leader,
 * bomb-rock, etc.
 * @param specific_type Specific type of mob, if you want to specify further.
 * @param icon If not nullptr, use this icon to represent this subgroup.
 */
void SubgroupTypeManager::register_type(
    const SUBGROUP_TYPE_CATEGORY category,
    MobType* specific_type,
    ALLEGRO_BITMAP* icon
) {
    SubgroupType* new_sg_type = new SubgroupType();
    
    new_sg_type->category = category;
    new_sg_type->specific_type = specific_type;
    new_sg_type->icon = icon;
    
    types.push_back(new_sg_type);
}


/**
 * @brief Constructs a new whistle struct object.
 */
Whistle::Whistle() :
    radius(0.0f),
    fade_radius(0.0f),
    fade_timer(WHISTLE::FADE_TIME),
    next_dot_timer(WHISTLE::DOT_INTERVAL),
    next_ring_timer(WHISTLE::RINGS_INTERVAL),
    ring_prev_color(0),
    whistling(false) {
    
    dot_radius[0] = -1;
    dot_radius[1] = -1;
    dot_radius[2] = -1;
    dot_radius[3] = -1;
    dot_radius[4] = -1;
    dot_radius[5] = -1;
    
    next_dot_timer.on_end = [this] () {
        next_dot_timer.start();
        unsigned char dot = 255;
        for(unsigned char d = 0; d < 6; d++) { //Find WHAT dot to add.
            if(dot_radius[d] == -1) {
                dot = d;
                break;
            }
        }
        
        if(dot != 255) dot_radius[dot] = 0;
    };
    
    next_ring_timer.on_end = [this] () {
        next_ring_timer.start();
        rings.push_back(0);
        ring_colors.push_back(ring_prev_color);
        ring_prev_color =
            sum_and_wrap(ring_prev_color, 1, WHISTLE::N_RING_COLORS);
    };
    
}


/**
 * @brief Stuff to do when a leader starts whistling.
 */
void Whistle::start_whistling() {
    for(unsigned char d = 0; d < 6; d++) {
        dot_radius[d] = -1;
    }
    fade_timer.start();
    fade_radius = 0;
    whistling = true;
}


/**
 * @brief Stuff to do when a leader stops whistling.
 */
void Whistle::stop_whistling() {
    whistling = false;
    fade_timer.start();
    fade_radius = radius;
    radius = 0;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 * @param center What its center is on this frame.
 * @param whistle_range How far the whistle can reach from the cursor center.
 * @param leader_to_cursor_dist Distance between the leader and the cursor.
 */
void Whistle::tick(
    float delta_t, const Point &center,
    float whistle_range, float leader_to_cursor_dist
) {
    this->center = center;
    
    fade_timer.tick(delta_t);
    
    if(whistling) {
        //Create rings.
        next_ring_timer.tick(delta_t);
        next_dot_timer.tick(delta_t);
        
        for(unsigned char d = 0; d < 6; d++) {
            if(dot_radius[d] == -1) continue;
            
            dot_radius[d] += game.config.whistle_growth_speed * delta_t;
            if(radius > 0 && dot_radius[d] > whistle_range) {
                dot_radius[d] = whistle_range;
                
            } else if(fade_radius > 0 && dot_radius[d] > fade_radius) {
                dot_radius[d] = fade_radius;
            }
        }
    }
    
    for(size_t r = 0; r < rings.size(); ) {
        //Erase rings that go beyond the cursor.
        rings[r] += WHISTLE::RING_SPEED * delta_t;
        if(leader_to_cursor_dist < rings[r]) {
            rings.erase(rings.begin() + r);
            ring_colors.erase(ring_colors.begin() + r);
        } else {
            r++;
        }
    }
}
