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

#include "const.h"
#include "drawing.h"
#include "functions.h"
#include "game.h"
#include "load.h"
#include "utils/allegro_utils.h"
#include "utils/general_utils.h"
#include "utils/string_utils.h"


namespace MSG_BOX {

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
    "teleport"
};

}


/**
 * @brief Loads the asset file names from a file.
 *
 * @param file File to load from.
 */
void asset_file_names_t::load(data_node* file) {
    reader_setter grs(file->get_child_by_name("graphics"));
    
    grs.set("area_name_font", bmp_area_name_font);
    grs.set("bright_circle", bmp_bright_circle);
    grs.set("bright_ring", bmp_bright_ring);
    grs.set("bubble_box", bmp_bubble_box);
    grs.set("checkbox_check", bmp_checkbox_check);
    grs.set("checkbox_no_check", bmp_checkbox_no_check);
    grs.set("player_input_icons", bmp_player_input_icons);
    grs.set("counter_font", bmp_counter_font);
    grs.set("cursor", bmp_cursor);
    grs.set("cursor_counter_font", bmp_cursor_counter_font);
    grs.set("editor_icons", bmp_editor_icons);
    grs.set("enemy_spirit", bmp_enemy_spirit);
    grs.set("focus_box", bmp_focus_box);
    grs.set("icon", bmp_icon);
    grs.set("idle_glow", bmp_idle_glow);
    grs.set("leader_silhouette_side", bmp_leader_silhouette_side);
    grs.set("leader_silhouette_top", bmp_leader_silhouette_top);
    grs.set("main_font", bmp_main_font);
    grs.set("main_menu", bmp_main_menu);
    grs.set("medal_bronze", bmp_medal_bronze);
    grs.set("medal_gold", bmp_medal_gold);
    grs.set("medal_none", bmp_medal_none);
    grs.set("medal_platinum", bmp_medal_platinum);
    grs.set("medal_silver", bmp_medal_silver);
    grs.set("menu_icons", bmp_menu_icons);
    grs.set("mission_clear", bmp_mission_clear);
    grs.set("mission_fail", bmp_mission_fail);
    grs.set("more", bmp_more);
    grs.set("mouse_cursor", bmp_mouse_cursor);
    grs.set("notification", bmp_notification);
    grs.set("pikmin_spirit", bmp_pikmin_spirit);
    grs.set("random", bmp_random);
    grs.set("rock", bmp_rock);
    grs.set("shadow", bmp_shadow);
    grs.set("smack", bmp_smack);
    grs.set("smoke", bmp_smoke);
    grs.set("sparkle", bmp_sparkle);
    grs.set("spotlight", bmp_spotlight);
    grs.set("swarm_arrow", bmp_swarm_arrow);
    grs.set("throw_invalid", bmp_throw_invalid);
    grs.set("throw_preview", bmp_throw_preview);
    grs.set("throw_preview_dashed", bmp_throw_preview_dashed);
    grs.set("value_font", bmp_value_font);
    grs.set("wave_ring", bmp_wave_ring);
    
    reader_setter frs(file->get_child_by_name("fonts"));
    
    frs.set("area_name", fnt_area_name);
    frs.set("counter", fnt_counter);
    frs.set("cursor_counter", fnt_cursor_counter);
    frs.set("slim_font", fnt_slim);
    frs.set("standard", fnt_standard);
    frs.set("value", fnt_value);
    
    reader_setter srs(file->get_child_by_name("sounds"));
    
    srs.set("attack", sfx_attack);
    srs.set("camera", sfx_camera);
    srs.set("menu_activate", sfx_menu_activate);
    srs.set("menu_back", sfx_menu_back);
    srs.set("menu_select", sfx_menu_select);
    srs.set("obstacle_cleared", sfx_obstacle_cleared);
    srs.set("pluck", sfx_pluck);
    srs.set("spray", sfx_spray);
    srs.set("switch_pikmin", sfx_switch_pikmin);
    srs.set("throw", sfx_throw);
}


/**
 * @brief Loads an audio stream for the manager.
 *
 * @param name Name of the audio stream to load.
 * @param node If not nullptr, blame this data node if the file doesn't exist.
 * @param report_errors Only issues errors if this is true.
 * @return The audio stream.
 */
ALLEGRO_AUDIO_STREAM* audio_stream_manager::do_load(
    const string &path, data_node* node, const bool report_errors
) {
    return load_audio_stream(path, node, report_errors);
}


/**
 * @brief Unloads an audio stream for the manager.
 *
 * @param asset Audio stream to unload.
 */
void audio_stream_manager::do_unload(ALLEGRO_AUDIO_STREAM* asset) {
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
ALLEGRO_BITMAP* bitmap_manager::do_load(
    const string &path, data_node* node, const bool report_errors
) {
    return load_bmp(path, node, report_errors);
}


/**
 * @brief Unloads a bitmap for the manager.
 *
 * @param asset Bitmap to unload.
 */
void bitmap_manager::do_unload(ALLEGRO_BITMAP* asset) {
    if(asset != game.bmp_error) {
        al_destroy_bitmap(asset);
    }
}


/**
 * @brief Instantly places the camera at the specified coordinates.
 *
 * @param new_pos Coordinates to place the camera at.
 */
void camera_t::set_pos(const point &new_pos) {
    pos = new_pos;
    target_pos = new_pos;
}


/**
 * @brief Instantly places the camera at the specified zoom.
 *
 * @param new_zoom Zoom to set to.
 */
void camera_t::set_zoom(const float new_zoom) {
    zoom = new_zoom;
    target_zoom = new_zoom;
}


/**
 * @brief Ticks camera movement by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void camera_t::tick(const float delta_t) {
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
void camera_t::update_box() {
    box[0] = point(0, 0);
    box[1] = point(game.win_w, game.win_h);
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
 * @brief Returns the index number of an item, given its name.
 *
 * @param name Name of the item.
 * @return The index, or INVALID on error.
 */
size_t enum_name_database::get_idx(const string &name) const {
    for(size_t n = 0; n < names.size(); ++n) {
        if(names[n] == name) return n;
    }
    return INVALID;
}


/**
 * @brief Returns the name of an item, given its index number.
 *
 * @param idx Index number of the item.
 * @return The name, or an empty string on error.
 */
string enum_name_database::get_name(const size_t idx) const {
    if(idx < names.size()) return names[idx];
    return "";
}


/**
 * @brief Returns the number of items registered.
 *
 * @return The amount.
 */
size_t enum_name_database::get_nr_of_items() const {
    return names.size();
}


/**
 * @brief Registers a new item.
 *
 * @param idx Its index number.
 * @param name Its name.
 */
void enum_name_database::register_item(
    const size_t idx, const string &name
) {
    if(idx >= names.size()) {
        names.insert(names.end(), (idx + 1) - names.size(), "");
    }
    names[idx] = name;
}



/**
 * @brief Emits an error in the gameplay "info" window.
 *
 * @param s Full error description.
 */
void error_manager::emit_in_gameplay(const string &s) {
    string info_str =
        "\n\n\n"
        "ERROR: " + s + "\n\n"
        "(Saved to \"" + ERROR_LOG_FILE_PATH + "\".)\n\n";
    print_info(info_str, 30.0f, 3.0f);
}


/**
 * @brief Logs an error to stdout (i.e. the console).
 *
 * @param s Full error description.
 */
void error_manager::log_to_console(const string &s) {
    std::cout << s << std::endl;
}


/**
 * @brief Logs an error to the log file.
 *
 * @param s Full error description.
 */
void error_manager::log_to_file(const string &s) {
    string prev_error_log;
    string output = "";
    
    //Get the previous contents of the log file, if any.
    ALLEGRO_FILE* file_i =
        al_fopen(ERROR_LOG_FILE_PATH.c_str(), "r");
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
    for(size_t l = 1; l < lines.size(); ++l) {
        output += "  " + lines[l] + "\n";
    }
    
    //Save it.
    ALLEGRO_FILE* file_o =
        al_fopen(ERROR_LOG_FILE_PATH.c_str(), "w");
    if(file_o) {
        al_fwrite(file_o, prev_error_log + output);
        al_fclose(file_o);
    }
}


/**
 * @brief Prepares everything for an area load.
 */
void error_manager::prepare_area_load() {
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
void error_manager::report(const string &s, const data_node* d) {
    string full_error = s;
    if(d) {
        full_error += " (" + d->file_path;
        if (d->line_nr != 0) full_error += " line " + i2s(d->line_nr);
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
void error_manager::report_area_load_errors() {
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
        "(Saved to \"" + ERROR_LOG_FILE_PATH + "\".)\n\n";
        
    print_info(info_str, 30.0f, 3.0f);
}


/**
 * @brief Returns whether this session has had any error reports.
 *
 * @return Whether it had errors.
 */
bool error_manager::session_has_errors() {
    return nr_session_errors > 0;
}


/**
 * @brief Draws the fade overlay, if there is a fade in progress.
 */
void fade_manager::draw() {
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
float fade_manager::get_perc_left() const {
    return time_left / GAME::FADE_DURATION;
}


/**
 * @brief Returns whether the current fade is a fade in or fade out.
 *
 * @return Whether it is a fade in.
 */
bool fade_manager::is_fade_in() const {
    return fade_in;
}


/**
 * @brief Returns whether or not a fade is currently in progress.
 *
 * @return Whether it is in progress.
 */
bool fade_manager::is_fading() const {
    return time_left > 0;
}


/**
 * @brief Sets up the start of a fade.
 *
 * @param is_fade_in If true, this fades in. If false, fades out.
 * @param on_end Code to run when the fade finishes.
 */
void fade_manager::start_fade(
    const bool is_fade_in, const std::function<void()> &on_end
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
void fade_manager::tick(const float delta_t) {
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
maker_tools_t::maker_tools_t() :
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
    
    info_print_timer = timer(1.0f, [this] () { info_print_text.clear(); });
    for(size_t k = 0; k < 20; ++k) {
        keys[k] = MAKER_TOOL_TYPE_NONE;
    }
}


/**
 * @brief Resets the states of the tools so that players can play without any
 * tool affecting the experience.
 */
void maker_tools_t::reset_for_gameplay() {
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
void mouse_cursor_t::hide() const {
    al_hide_mouse_cursor(game.display);
}


/**
 * @brief Initializes everything.
 */
void mouse_cursor_t::init() {
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
void mouse_cursor_t::reset() {
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
void mouse_cursor_t::show() const {
    al_show_mouse_cursor(game.display);
}


/**
 * @brief Updates the coordinates from an Allegro mouse event.
 *
 * @param ev Event to handle.
 * @param screen_to_world_transform Transformation to use to get the
 * screen coordinates.
 */
void mouse_cursor_t::update_pos(
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
void notification_t::draw() const {
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
    
    int bmp_w = al_get_bitmap_width(game.sys_assets.bmp_notification);
    int bmp_h = al_get_bitmap_height(game.sys_assets.bmp_notification);
    
    float text_box_x1 = -bmp_w * 0.5 + DRAWING::NOTIFICATION_PADDING;
    float text_box_x2 = bmp_w * 0.5 - DRAWING::NOTIFICATION_PADDING;
    float text_box_y1 = -bmp_h - DRAWING::NOTIFICATION_PADDING;
    float text_box_y2 = DRAWING::NOTIFICATION_PADDING;
    
    draw_bitmap(
        game.sys_assets.bmp_notification,
        point(0, -bmp_h * 0.5),
        point(bmp_w, bmp_h),
        0,
        map_alpha(DRAWING::NOTIFICATION_ALPHA * visibility)
    );
    
    if(input.type != INPUT_TYPE_NONE) {
        text_box_x1 +=
            DRAWING::NOTIFICATION_CONTROL_SIZE + DRAWING::NOTIFICATION_PADDING;
        draw_player_input_icon(
            game.sys_assets.fnt_slim, input,
            true,
            point(
                -bmp_w * 0.5 + DRAWING::NOTIFICATION_PADDING +
                DRAWING::NOTIFICATION_CONTROL_SIZE * 0.5,
                -bmp_h * 0.5
            ),
            point(
                DRAWING::NOTIFICATION_CONTROL_SIZE,
                DRAWING::NOTIFICATION_CONTROL_SIZE
            ),
            visibility * 255
        );
    }
    
    draw_compressed_text(
        game.sys_assets.fnt_standard,
        map_alpha(DRAWING::NOTIFICATION_ALPHA * visibility),
        point(
            (text_box_x1 + text_box_x2) * 0.5,
            (text_box_y1 + text_box_y2) * 0.5
        ),
        ALLEGRO_ALIGN_CENTER,
        TEXT_VALIGN_MODE_CENTER,
        point(
            text_box_x2 - text_box_x1,
            text_box_y2 - text_box_y1
        ),
        text
    );
    
    al_use_transform(&old_tra);
}


/**
 * @brief Returns how "present" the notification is.
 * 0 means hidden, 1 is fully visible. Mid values are transition.
 *
 * @return The visibility.
 */
float notification_t::get_visibility() const {
    return visibility;
}


/**
 * @brief Resets the whole thing.
 */
void notification_t::reset() {
    enabled = true;
    input.type = INPUT_TYPE_NONE;
    text.clear();
    pos = point();
    visibility = 0.0f;
}


/**
 * @brief Sets the contents to show.
 *
 * @param input Player input icon to show.
 * @param text Text to show.
 * @param pos Where to show it in the game world.
 */
void notification_t::set_contents(
    const player_input &input, const string &text, const point &pos
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
void notification_t::set_enabled(const bool enabled) {
    this->enabled = enabled;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void notification_t::tick(const float delta_t) {
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
performance_monitor_t::performance_monitor_t() :
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
void performance_monitor_t::enter_state(const PERF_MON_STATE state) {
    if(paused) return;
    
    cur_state = state;
    cur_state_start_time = al_get_time();
    cur_page = page();
    
    if(cur_state == PERF_MON_STATE_FRAME) {
        frame_samples++;
    }
}


/**
 * @brief Finishes the latest measurement.
 */
void performance_monitor_t::finish_measurement() {
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
    
    for(size_t m = 0; m < cur_page.measurements.size(); ++m) {
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
void performance_monitor_t::leave_state() {
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
            for(size_t m = 0; m < cur_page.measurements.size(); ++m) {
                bool is_new = true;
                for(
                    size_t m2 = 0;
                    m2 < frame_avg_page.measurements.size(); ++m2
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
void performance_monitor_t::reset() {
    area_name.clear();
    cur_state = PERF_MON_STATE_LOADING;
    paused = false;
    cur_state_start_time = 0.0;
    cur_measurement_start_time = 0.0;
    cur_measurement_name.clear();
    cur_page = page();
    frame_samples = 0;
    loading_page = page();
    frame_avg_page = page();
    frame_fastest_page = page();
    frame_slowest_page = page();
}


/**
 * @brief Saves a log file with all known stats, if there is anything to save.
 */
void performance_monitor_t::save_log() {
    if(loading_page.measurements.empty()) {
        //Nothing to save.
        return;
    }
    
    //Average out the frames of gameplay.
    frame_avg_page.duration /= (double) frame_samples;
    for(size_t m = 0; m < frame_avg_page.measurements.size(); ++m) {
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
        al_fopen(PERFORMANCE_LOG_FILE_PATH.c_str(), "r");
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
        al_fopen(PERFORMANCE_LOG_FILE_PATH.c_str(), "w");
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
void performance_monitor_t::set_area_name(const string &name) {
    area_name = name;
}


/**
 * @brief Sets whether monitoring is currently paused or not.
 *
 * @param paused Pause value.
 */
void performance_monitor_t::set_paused(const bool paused) {
    this->paused = paused;
}


/**
 * @brief Starts measuring a certain point in the loading procedure.
 *
 * @param name Name of the measurement.
 */
void performance_monitor_t::start_measurement(const string &name) {
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
void performance_monitor_t::page::write(string &s) {
    //Get the total measured time.
    double total_measured_time = 0.0;
    for(size_t m = 0; m < measurements.size(); ++m) {
        total_measured_time += measurements[m].second;
    }
    
    //Write each measurement into the string.
    for(size_t m = 0; m < measurements.size(); ++m) {
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
void performance_monitor_t::page::write_measurement(
    string &str, const string &name, const double dur, const float total
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
reader_setter::reader_setter(data_node* dn) :
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
void reader_setter::set(
    const string &child, ALLEGRO_COLOR &var, data_node** child_node
) {
    data_node* n = node->get_child_by_name(child);
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
void reader_setter::set(
    const string &child, string &var, data_node** child_node
) {
    data_node* n = node->get_child_by_name(child);
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
void reader_setter::set(
    const string &child, size_t &var, data_node** child_node
) {
    data_node* n = node->get_child_by_name(child);
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
void reader_setter::set(
    const string &child, int &var, data_node** child_node
) {
    data_node* n = node->get_child_by_name(child);
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
void reader_setter::set(
    const string &child, unsigned int &var, data_node** child_node
) {
    data_node* n = node->get_child_by_name(child);
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
void reader_setter::set(
    const string &child, unsigned char &var, data_node** child_node
) {
    data_node* n = node->get_child_by_name(child);
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
void reader_setter::set(
    const string &child, bool &var, data_node** child_node
) {
    data_node* n = node->get_child_by_name(child);
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
void reader_setter::set(
    const string &child, float &var, data_node** child_node
) {
    data_node* n = node->get_child_by_name(child);
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
void reader_setter::set(
    const string &child, double &var, data_node** child_node
) {
    data_node* n = node->get_child_by_name(child);
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
void reader_setter::set(
    const string &child, point &var, data_node** child_node
) {
    data_node* n = node->get_child_by_name(child);
    if(!n->value.empty()) {
        if(child_node) *child_node = n;
        var = s2p(n->value);
    } else {
        if(child_node) *child_node = nullptr;
    }
}



/**
 * @brief Constructs a new script var reader object.
 *
 * @param vars Map of variables to read from.
 */
script_var_reader::script_var_reader(map<string, string> &vars) :
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
bool script_var_reader::get(const string &name, ALLEGRO_COLOR &dest) const {
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
bool script_var_reader::get(const string &name, string &dest) const {
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
bool script_var_reader::get(const string &name, size_t &dest) const {
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
bool script_var_reader::get(const string &name, int &dest) const {
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
bool script_var_reader::get(const string &name, unsigned char &dest) const {
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
bool script_var_reader::get(const string &name, bool &dest) const {
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
bool script_var_reader::get(const string &name, float &dest) const {
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
bool script_var_reader::get(const string &name, point &dest) const {
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
ALLEGRO_SAMPLE* sfx_sample_manager::do_load(
    const string &path, data_node* node, const bool report_errors
) {
    return load_sample(path, node, report_errors);
}


/**
 * @brief Unloads an audio sample for the manager.
 *
 * @param asset Audio sample to unload.
 */
void sfx_sample_manager::do_unload(ALLEGRO_SAMPLE* asset) {
    al_destroy_sample(asset);
}


/**
 * @brief Clears the list of registered subgroup types.
 */
void subgroup_type_manager::clear() {
    for(size_t t = 0; t < types.size(); ++t) {
        delete types[t];
    }
    types.clear();
}


/**
 * @brief Returns the first registered subgroup type.
 *
 * @return The first type.
 */
subgroup_type* subgroup_type_manager::get_first_type() const {
    return types.front();
}


/**
 * @brief Returns the subgroup type that comes after the given type.
 *
 * @param sgt Subgroup type to iterate from.
 * @return The next type.
 */
subgroup_type* subgroup_type_manager::get_next_type(
    const subgroup_type* sgt
) const {
    for(size_t t = 0; t < types.size(); ++t) {
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
subgroup_type* subgroup_type_manager::get_prev_type(
    const subgroup_type* sgt
) const {
    for(size_t t = 0; t < types.size(); ++t) {
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
subgroup_type* subgroup_type_manager::get_type(
    const SUBGROUP_TYPE_CATEGORY category,
    const mob_type* specific_type
) const {
    for(size_t t = 0; t < types.size(); ++t) {
        subgroup_type* t_ptr = types[t];
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
void subgroup_type_manager::register_type(
    const SUBGROUP_TYPE_CATEGORY category,
    mob_type* specific_type,
    ALLEGRO_BITMAP* icon
) {
    subgroup_type* new_sg_type = new subgroup_type();
    
    new_sg_type->category = category;
    new_sg_type->specific_type = specific_type;
    new_sg_type->icon = icon;
    
    types.push_back(new_sg_type);
}


/**
 * @brief Constructs a new timer object.
 *
 * @param duration How long before it reaches the end, in seconds.
 * @param on_end Code to run when time ends.
 */
timer::timer(float duration, const std::function<void()> &on_end) :
    time_left(0),
    duration(duration),
    on_end(on_end) {
    
    
}


/**
 * @brief Destroys the timer object.
 */
timer::~timer() {
    //TODO Valgrind detects a leak with on_end...
}


/**
 * @brief Returns the ratio of time left
 * (i.e. 0 if done, 1 if all time is left).
 *
 * @return The ratio left.
 */
float timer::get_ratio_left() const {
    return time_left / duration;
}



/**
 * @brief Starts a timer.
 *
 * @param can_restart If false, calling this while the timer is still
 * ticking down will not do anything.
 */
void timer::start(const bool can_restart) {
    if(!can_restart && time_left > 0) return;
    time_left = duration;
}


/**
 * @brief Starts a timer, but sets a new duration.
 *
 * @param new_duration Its new duration.
 */
void timer::start(const float new_duration) {
    duration = new_duration;
    start();
}


/**
 * @brief Stops a timer, without executing the on_end callback.
 */
void timer::stop() {
    time_left = 0.0f;
}


/**
 * @brief Ticks time by one frame of logic.
 *
 * @param delta_t How long the frame's tick is, in seconds.
 */
void timer::tick(const float delta_t) {
    if(time_left == 0.0f) return;
    time_left = std::max(time_left - delta_t, 0.0f);
    if(time_left == 0.0f && on_end) {
        on_end();
    }
}


/**
 * @brief Constructs a new whistle struct object.
 */
whistle_t::whistle_t() :
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
        for(unsigned char d = 0; d < 6; ++d) { //Find WHAT dot to add.
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
void whistle_t::start_whistling() {
    for(unsigned char d = 0; d < 6; ++d) {
        dot_radius[d] = -1;
    }
    fade_timer.start();
    fade_radius = 0;
    whistling = true;
}


/**
 * @brief Stuff to do when a leader stops whistling.
 */
void whistle_t::stop_whistling() {
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
void whistle_t::tick(
    const float delta_t, const point &center,
    const float whistle_range, const float leader_to_cursor_dist
) {
    this->center = center;
    
    fade_timer.tick(delta_t);
    
    if(whistling) {
        //Create rings.
        next_ring_timer.tick(delta_t);
        next_dot_timer.tick(delta_t);
        
        for(unsigned char d = 0; d < 6; ++d) {
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
