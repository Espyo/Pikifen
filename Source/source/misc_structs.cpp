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

#include <algorithm>
#include <climits>

#include "misc_structs.h"

#include "const.h"
#include "drawing.h"
#include "functions.h"
#include "game.h"
#include "load.h"
#include "utils/string_utils.h"


/* ----------------------------------------------------------------------------
 * Creates a new asset file name list struct.
 */
asset_file_names_struct::asset_file_names_struct() :
    area_name_font("Area_name_font.png"),
    bubble_box("Bubble_box.png"),
    checkbox_check("Checkbox_check.png"),
    cursor("Cursor.png"),
    cursor_invalid("Cursor_invalid.png"),
    counter_font("Counter_font.png"),
    editor_icons("Editor_icons.png"),
    enemy_spirit("Enemy_spirit.png"),
    icon("Icon.png"),
    idle_glow("Idle_glow.png"),
    main_font("Font.png"),
    main_menu("Main_menu.jpg"),
    mouse_cursor("Mouse_cursor.png"),
    mouse_wd_icon("Mouse_wheel_down_icon.png"),
    mouse_wu_icon("Mouse_wheel_up_icon.png"),
    notification("Notification.png"),
    pikmin_silhouette("Pikmin_silhouette.png"),
    pikmin_spirit("Pikmin_spirit.png"),
    rock("Rock.png"),
    shadow("Shadow.png"),
    smack("Smack.png"),
    smoke("Smoke.png"),
    sparkle("Sparkle.png"),
    spotlight("Spotlight.png"),
    swarm_arrow("Swarm_arrow.png"),
    value_font("Value_font.png"),
    wave_ring("Wave_ring.png") {
    
    for(unsigned char i = 0; i < 3; ++i) {
        mouse_button_icon[i] =
            "Mouse_button_" + i2s(i + 1) + "_icon.png";
    }
}


/* ----------------------------------------------------------------------------
 * Loads the asset file names from a file.
 * file:
 *   File to load from.
 */
void asset_file_names_struct::load(data_node* file) {
    reader_setter rs(file);
    
    rs.set("area_name_font", area_name_font);
    rs.set("bubble_box", bubble_box);
    rs.set("checkbox_check", checkbox_check);
    rs.set("cursor", cursor);
    rs.set("cursor_invalid", cursor_invalid);
    rs.set("counter_font", counter_font);
    rs.set("editor_icons", editor_icons);
    rs.set("enemy_spirit", enemy_spirit);
    rs.set("icon", icon);
    rs.set("idle_glow", idle_glow);
    rs.set("main_font", main_font);
    rs.set("main_menu", main_menu);
    rs.set("mouse_cursor", mouse_cursor);
    rs.set("mouse_wheel_down_icon", mouse_wd_icon);
    rs.set("mouse_wheel_up_icon", mouse_wu_icon);
    rs.set("notification", notification);
    rs.set("pikmin_silhouette", pikmin_silhouette);
    rs.set("pikmin_spirit", pikmin_spirit);
    rs.set("shadow", shadow);
    rs.set("smack", smack);
    rs.set("smoke", smoke);
    rs.set("sparkle", sparkle);
    rs.set("spotlight", spotlight);
    rs.set("swarm_arrow", swarm_arrow);
    rs.set("value_font", value_font);
    rs.set("wave_ring", wave_ring);
    
    for(unsigned char i = 0; i < 3; ++i) {
        rs.set(
            "mouse_button_" + i2s(i + 1) + "_icon",
            mouse_button_icon[i]
        );
    }
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty bitmap effect info struct.
 */
bitmap_effect_info::bitmap_effect_info() :
    translation(0, 0),
    rotation(0),
    scale(1, 1),
    tint_color(al_map_rgb(255, 255, 255)),
    glow_color(al_map_rgb(0, 0, 0)) {
    
}



/* ----------------------------------------------------------------------------
 * Creates a bitmap manager.
 * base_dir:
 *   Base directory its files belong to.
 */
bmp_manager::bmp_manager(const string &base_dir) :
    base_dir(base_dir),
    total_calls(0) {
    
}


/* ----------------------------------------------------------------------------
 * Deletes all bitmaps loaded and clears the list.
 */
void bmp_manager::clear() {
    for(auto &b : list) {
        if(b.second.b != game.bmp_error) {
            al_destroy_bitmap(b.second.b);
        }
    }
    list.clear();
    total_calls = 0;
}


/* ----------------------------------------------------------------------------
 * Marks a bitmap to have one less call.
 * If it has 0 calls, it's automatically cleared.
 * it:
 *   Iterator from the map for the bitmap.
 */
void bmp_manager::detach(map<string, bmp_info>::iterator it) {
    if(it == list.end()) return;
    
    it->second.calls--;
    total_calls--;
    if(it->second.calls == 0) {
        if(it->second.b != game.bmp_error) {
            al_destroy_bitmap(it->second.b);
        }
        list.erase(it);
    }
}


/* ----------------------------------------------------------------------------
 * Marks a bitmap to have one less call.
 * If it has 0 calls, it's automatically cleared.
 * name:
 *   Bitmap's file name.
 */
void bmp_manager::detach(const string &name) {
    if(name.empty()) return;
    detach(list.find(name));
}


/* ----------------------------------------------------------------------------
 * Marks a bitmap to have one less call.
 * If it has 0 calls, it's automatically cleared.
 * bmp:
 *   Bitmap to detach.
 */
void bmp_manager::detach(ALLEGRO_BITMAP* bmp) {
    if(!bmp || bmp == game.bmp_error) return;
    
    auto it = list.begin();
    for(; it != list.end(); ++it) {
        if(it->second.b == bmp) break;
    }
    
    detach(it);
}


/* ----------------------------------------------------------------------------
 * Returns the specified bitmap, by name.
 * name:
 *   Name of the bitmap to get.
 * node:
 *   If not NULL, blame this data node if the file doesn't exist.
 * report_errors:
 *   Only issues errors if this is true.
 */
ALLEGRO_BITMAP* bmp_manager::get(
    const string &name, data_node* node,
    const bool report_errors
) {
    if(name.empty()) return load_bmp("", node, report_errors);
    
    if(list.find(name) == list.end()) {
        ALLEGRO_BITMAP* b =
            load_bmp(base_dir + "/" + name, node, report_errors);
        list[name] = bmp_info(b);
        total_calls++;
        return b;
    } else {
        list[name].calls++;
        total_calls++;
        return list[name].b;
    }
};


/* ----------------------------------------------------------------------------
 * Returns the size of the list. Used for debugging.
 */
size_t bmp_manager::get_list_size() const {
    return list.size();
}



/* ----------------------------------------------------------------------------
 * Returns the total number of calls. Used for debugging.
 */
long bmp_manager::get_total_calls() const {
    return total_calls;
}


/* ----------------------------------------------------------------------------
 * Creates a structure with information about a bitmap, for the manager.
 * b:
 *   The bitmap.
 */
bmp_manager::bmp_info::bmp_info(ALLEGRO_BITMAP* b) :
    b(b),
    calls(1) {
    
}


/* ----------------------------------------------------------------------------
 * Adds a new button to the list.
 * id:
 *   Its ID.
 * name:
 *   Its name.
 * option_name:
 *   The name of its property in the options file.
 * default_control_str:
 *   A string representing the default controls for this button.
 */
void button_manager::add(
    const size_t id, const string &name, const string &option_name,
    const string &default_control_str
) {
    button_manager::button b;
    b.id = id;
    b.name = name;
    b.option_name = option_name;
    b.default_control_str = default_control_str;
    
    list.push_back(b);
}


/* ----------------------------------------------------------------------------
 * Creates a camera info struct.
 */
camera_info::camera_info() :
    target_zoom(1.0f),
    zoom(1.0f) {
    
}


/* ----------------------------------------------------------------------------
 * Instantly places the camera at the specified coordinates.
 * new_pos:
 *   Coordinates to place the camera at.
 */
void camera_info::set_pos(const point &new_pos) {
    pos = new_pos;
    target_pos = new_pos;
}


/* ----------------------------------------------------------------------------
 * Instantly places the camera at the specified zoom.
 * new_zoom:
 *   Zoom to set to.
 */
void camera_info::set_zoom(const float new_zoom) {
    zoom = new_zoom;
    target_zoom = new_zoom;
}


const float CAMERA_SMOOTHNESS_MULT = 4.5f;

/* ----------------------------------------------------------------------------
 * Ticks one frame of camera movement.
 * delta_t:
 *   How many seconds to tick by.
 */
void camera_info::tick(const float delta_t) {
    pos.x += (target_pos.x - pos.x) * (CAMERA_SMOOTHNESS_MULT * delta_t);
    pos.y += (target_pos.y - pos.y) * (CAMERA_SMOOTHNESS_MULT * delta_t);
    zoom += (target_zoom - zoom) * (CAMERA_SMOOTHNESS_MULT * delta_t);
}


const float CAMERA_BOX_MARGIN = 128.0f;

/* ----------------------------------------------------------------------------
 * Updates the camera's visibility box, based on the screen_to_world_transform
 * transformation.
 */
void camera_info::update_box() {
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
    box[0].x -= CAMERA_BOX_MARGIN;
    box[0].y -= CAMERA_BOX_MARGIN;
    box[1].x += CAMERA_BOX_MARGIN;
    box[1].y += CAMERA_BOX_MARGIN;
}


const float fade_manager::FADE_DURATION = 0.15f;

/* ----------------------------------------------------------------------------
 * Creates a fade manager.
 */
fade_manager::fade_manager() :
    time_left(0),
    fade_in(false),
    on_end(nullptr) {
    
}


/* ----------------------------------------------------------------------------
 * Draws the fade overlay, if there is a fade in progress.
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


/* ----------------------------------------------------------------------------
 * Returns the percentage of progress left in the current fade.
 */
float fade_manager::get_perc_left() const {
    return time_left / FADE_DURATION;
}


/* ----------------------------------------------------------------------------
 * Returns whether the current fade is a fade in or fade out.
 */
bool fade_manager::is_fade_in() const {
    return fade_in;
}


/* ----------------------------------------------------------------------------
 * Returns whether or not a fade is currently in progress.
 */
bool fade_manager::is_fading() const {
    return time_left > 0;
}


/* ----------------------------------------------------------------------------
 * Sets up the start of a fade.
 * is_fade_in:
 *   If true, this fades in. If false, fades out.
 * on_end:
 *   Code to run when the fade finishes.
 */
void fade_manager::start_fade(
    const bool is_fade_in, const std::function<void()> &on_end
) {
    time_left = FADE_DURATION;
    fade_in = is_fade_in;
    this->on_end = on_end;
}


/* ----------------------------------------------------------------------------
 * Ticks the fade manager by one frame.
 * time:
 *   How many seconds to tick by.
 */
void fade_manager::tick(const float time) {
    if(time_left == 0) return;
    time_left -= time;
    if(time_left <= 0) {
        time_left = 0;
        if(on_end) on_end();
    }
}



/* ----------------------------------------------------------------------------
 * Creates a font list struct.
 */
font_list::font_list() :
    area_name(nullptr),
    builtin(nullptr),
    counter(nullptr),
    main(nullptr),
    value(nullptr) {
    
}



/* ----------------------------------------------------------------------------
 * Creates a new HUD item.
 * center:
 *   Center coordinates, in screen dimension ratio.
 * size:
 *   Dimensions, in screen dimension ratio.
 */
hud_item::hud_item(const point center, const point size) :
    center(center),
    size(size) {
    
}


/* ----------------------------------------------------------------------------
 * Initializes a HUD item manager.
 * item_total:
 *   How many HUD items exist in total.
 */
hud_item_manager::hud_item_manager(const size_t item_total) {
    items.assign(item_total, hud_item());
}


/* ----------------------------------------------------------------------------
 * Retrieves the data necessary for the drawing routine.
 * Returns false if this element shouldn't be drawn.
 * id:
 *   ID of the HUD item.
 * center:
 *   Pointer to place the final center coordinates in, if any.
 * size:
 *   Pointer to place the final dimensions in, if any.
 */
bool hud_item_manager::get_draw_data(
    const size_t id, point* center, point* size
) const {
    const hud_item* h = &items[id];
    if(h->size.x <= 0 || h->size.y <= 0) return false;
    if(h->center.x + h->size.x / 2.0f < 0)    return false;
    if(h->center.x - h->size.x / 2.0f > 1.0f) return false;
    if(h->center.y + h->size.y / 2.0f < 0)    return false;
    if(h->center.y - h->size.y / 2.0f > 1.0f) return false;
    
    if(center) {
        center->x = h->center.x * game.win_w;
        center->y = h->center.y * game.win_h;
    }
    if(size) {
        size->x = h->size.x * game.win_w;
        size->y = h->size.y * game.win_h;
    }
    
    return true;
}


/* ----------------------------------------------------------------------------
 * Sets a HUD item's data.
 * id:
 *   ID of the HUD item.
 * x:
 *   Horizontal coordinate, in screen width percentage (0 to 100, normally).
 * y:
 *   Vertical coordinate, in screen height percentage (0 to 100, normally).
 * w:
 *   Total width, in screen width percentage (0 to 100, normally).
 * h:
 *   Total height, in screen height percentage (0 to 100, normally).
 */
void hud_item_manager::set_item(
    const size_t id,
    const float x, const float y, const float w, const float h
) {
    items[id] =
        hud_item(point(x / 100.0f, y / 100.0f), point(w / 100.0f, h / 100.0f));
}


/* ----------------------------------------------------------------------------
 * Returns whether the mouse cursor is inside the specified item or not.
 * id:
 *   ID of the item to check.
 */
bool hud_item_manager::is_mouse_in(const size_t id) {
    const hud_item* h = &items[id];
    if(
        game.mouse_cursor_s.x >
        h->center.x * game.win_w + h->size.x * game.win_w
    ) {
        return false;
    }
    if(
        game.mouse_cursor_s.x <
        h->center.x * game.win_w - h->size.x * game.win_w
    ) {
        return false;
    }
    if(
        game.mouse_cursor_s.y >
        h->center.y * game.win_h + h->size.y * game.win_h
    ) {
        return false;
    }
    if(
        game.mouse_cursor_s.y <
        h->center.y * game.win_h - h->size.y * game.win_h
    ) {
        return false;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Ticks the manager one frame in time.
 * time:
 *   Seconds to tick by.
 */
void hud_item_manager::tick(const float time) {
}


/* ----------------------------------------------------------------------------
 * Creates a maker tool info struct.
 */
maker_tools_info::maker_tools_info() :
    area_image_shadows(true),
    area_image_size(2048),
    area_image_mobs(true),
    change_speed(false),
    change_speed_mult(2.0f),
    geometry_info(false),
    hitboxes(false),
    info_lock(nullptr),
    info_print_duration(5.0f),
    info_print_fade_duration(3.0f),
    last_pikmin_type(nullptr),
    mob_hurting_ratio(0.5),
    use_perf_mon(false) {
    
    info_print_timer = timer(1.0f, [this] () { info_print_text.clear(); });
    for(size_t k = 0; k < 20; ++k) {
        keys[k] = MAKER_TOOL_NONE;
    }
}


/* ----------------------------------------------------------------------------
 * Initializes a movement struct with all movements set to 0.
 */
movement_struct::movement_struct() :
    right(0),
    up(0),
    left(0),
    down(0) {
    
}


/* ----------------------------------------------------------------------------
 * Returns the values of the coordinates, magnitude, and angle,
 * but "cleaned" up.
 * All parameters are mandatory.
 * coords:
 *   Final X and Y coordinates.
 * angle:
 *   Angle compared to the center.
 * magnitude:
 *   Magnitude from the center.
 */
void movement_struct::get_clean_info(
    point* coords, float* angle, float* magnitude
) const {
    get_raw_info(coords, angle, magnitude);
    *magnitude =
        clamp(
            *magnitude,
            game.options.joystick_min_deadzone,
            game.options.joystick_max_deadzone
        );
    *magnitude =
        interpolate_number(
            *magnitude,
            game.options.joystick_min_deadzone,
            game.options.joystick_max_deadzone,
            0.0f, 1.0f
        );
    *coords = angle_to_coordinates(*angle, *magnitude);
}



/* ----------------------------------------------------------------------------
 * Returns the values of the coordinates, magnitude, and angle,
 * exactly as they are right now, without being "cleaned".
 * Don't use this one for normal gameplay, please.
 * All parameters are mandatory.
 * coords:
 *   X and Y coordinates.
 * angle:
 *   Angle compared to the center.
 * magnitude:
 *   Magnitude from the center.
 */
void movement_struct::get_raw_info(
    point* coords, float* angle, float* magnitude
) const {
    *coords = point(right - left, down - up);
    coordinates_to_angle(*coords, angle, magnitude);
}


/* ----------------------------------------------------------------------------
 * Creates a message box information struct.
 * text:
 *   Text to display.
 * speaker_icon:
 *   Bitmap representing who is talking, if not NULL.
 */
msg_box_info::msg_box_info(const string &text, ALLEGRO_BITMAP* speaker_icon):
    cur_char(0),
    cur_section(0),
    speaker_icon(speaker_icon) {
    
    message = unescape_string(text);
    if(message.size() && message.back() == '\n') {
        message.pop_back();
    }
    
    char_timer =
        timer(
            game.config.message_char_interval,
    [this] () {
        char_timer.start();
        cur_char++;
    }
        );
    char_timer.start();
    
    //Push the first character as a stopping character. Makes life easier.
    stopping_chars.push_back(0);
    
    vector<string> lines = split(text, "\n");
    size_t char_count = 0;
    for(size_t l = 0; l < lines.size(); ++l) {
        //+1 because of the new line character.
        char_count += lines[l].size() + 1;
        if((l + 1) % 3 == 0) stopping_chars.push_back(char_count);
    }
    
    if(stopping_chars.size() > 1) {
        //Remove one because the last line doesn't have a new line character.
        //Even if it does, it's invisible.
        stopping_chars.back()--;
    }
    stopping_chars.push_back(message.size());
}


/* ----------------------------------------------------------------------------
 * Handles the user having pressed the button to continue the message,
 * or to skip to showing everything in the current section.
 * Returns true if the message box continues active, false if it's over.
 */
bool msg_box_info::advance() {
    size_t stopping_char =
        stopping_chars[cur_section + 1];
    if(cur_char == stopping_char) {
        if(stopping_char == message.size()) {
            return false;
        } else {
            cur_section++;
        }
    } else {
        cur_char = stopping_char;
    }
    return true;
}


/* ----------------------------------------------------------------------------
 * Returns what lines of text should be shown right now.
 */
vector<string> msg_box_info::get_current_lines() const {
    string text_to_show =
        message.substr(
            stopping_chars[cur_section],
            cur_char - stopping_chars[cur_section]
        );
    return split(text_to_show, "\n");
}


/* ----------------------------------------------------------------------------
 * Ticks one frame of gameplay.
 * delta_t:
 *   Seconds to tick by.
 */
void msg_box_info::tick(const float delta_t) {
    if(cur_char < stopping_chars[cur_section + 1]) {
        if(char_timer.duration == 0.0f) {
            //Display everything right away.
            cur_char = stopping_chars[cur_section + 1];
        } else {
            char_timer.tick(game.delta_t);
        }
    }
}


/* ----------------------------------------------------------------------------
 * Creates a performance monitor.
 */
performance_monitor_struct::performance_monitor_struct() :
    cur_state(PERF_MON_STATE_LOADING),
    paused(false),
    cur_state_start_time(0.0),
    cur_measurement_start_time(0.0),
    frame_samples(0) {
    
    reset();
}


/* ----------------------------------------------------------------------------
 * Enters the given state of the monitoring process.
 * state:
 *   New state.
 */
void performance_monitor_struct::enter_state(const PERF_MON_STATES state) {
    if(paused) return;
    
    cur_state = state;
    cur_state_start_time = al_get_time();
    cur_page = page();
    
    if(cur_state == PERF_MON_STATE_FRAME) {
        frame_samples++;
    }
}


/* ----------------------------------------------------------------------------
 * Finishes the latest measurement.
 */
void performance_monitor_struct::finish_measurement() {
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


/* ----------------------------------------------------------------------------
 * Leaves the current state of the monitoring process.
 */
void performance_monitor_struct::leave_state() {
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


/* ----------------------------------------------------------------------------
 * Resets all of the performance monitor's information.
 */
void performance_monitor_struct::reset() {
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


/* ----------------------------------------------------------------------------
 * Saves a log file with all known stats, if there is anything to save.
 */
void performance_monitor_struct::save_log() {
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
        "; Pikifen version " +
        i2s(VERSION_MAJOR) + "." + i2s(VERSION_MINOR) +
        "." + i2s(VERSION_REV) + ", game version " +
        game.config.version;
        
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
    ALLEGRO_FILE* file_i = al_fopen(PERFORMANCE_LOG_FILE_PATH.c_str(), "r");
    if(file_i) {
        string line;
        while(!al_feof(file_i)) {
            getline(file_i, line);
            prev_log += line + "\n";
        }
        prev_log.erase(prev_log.size() - 1);
        al_fclose(file_i);
    }
    
    ALLEGRO_FILE* file_o = al_fopen(PERFORMANCE_LOG_FILE_PATH.c_str(), "w");
    if(file_o) {
        al_fwrite(file_o, prev_log + s);
        al_fclose(file_o);
    }
}


/* ----------------------------------------------------------------------------
 * Sets the name of the area that was monitored.
 * name:
 *   Name of the area.
 */
void performance_monitor_struct::set_area_name(const string &name) {
    area_name = name;
}


/* ----------------------------------------------------------------------------
 * Sets whether monitoring is currently paused or not.
 * paused:
 *   Pause value.
 */
void performance_monitor_struct::set_paused(const bool paused) {
    this->paused = paused;
}


/* ----------------------------------------------------------------------------
 * Starts measuring a certain point in the loading procedure.
 * name:
 *   Name of the measurement.
 */
void performance_monitor_struct::start_measurement(const string &name) {
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


/* ----------------------------------------------------------------------------
 * Creates a performance monitor page struct.
 */
performance_monitor_struct::page::page() :
    duration(0.0) {
}


/* ----------------------------------------------------------------------------
 * Writes a page of information to a string.
 * s:
 *   String to write to.
 */
void performance_monitor_struct::page::write(string &s) {
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


/* ----------------------------------------------------------------------------
 * Writes a measurement in a human-friendly format onto a string.
 * str:
 *   The string to write to.
 * name:
 *   The name of this measurement.
 * dur:
 *   How long it lasted for, in seconds.
 * total:
 *   How long the entire procedure lasted for.
 */
void performance_monitor_struct::page::write_measurement(
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


/* ----------------------------------------------------------------------------
 * Creates a "reader setter".
 * dn:
 *   Pointer to the base data node.
 */
reader_setter::reader_setter(data_node* dn) :
    node(dn) {
    
}


/* ----------------------------------------------------------------------------
 * Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 * child:
 *   Name of the child node.
 * var:
 *   The var to set. This is an Allegro color.
 * child_node:
 *   If not-NULL, the node from whence the value came is placed here.
 *   NULL is placed if the property does not exist or has no value.
 */
void reader_setter::set(
    const string &child, ALLEGRO_COLOR &var, data_node** child_node
) {
    data_node* n = node->get_child_by_name(child);
    if(!n->value.empty()) {
        if(child_node) *child_node = n;
        var = s2c(n->value);
    } else {
        if(child_node) *child_node = NULL;
    }
}


/* ----------------------------------------------------------------------------
 * Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 * child:
 *   Name of the child node.
 * var:
 *   The var to set. This is a string.
 * child_node:
 *   If not-NULL, the node from whence the value came is placed here.
 *   NULL is placed if the property does not exist or has no value.
 */
void reader_setter::set(
    const string &child, string &var, data_node** child_node
) {
    data_node* n = node->get_child_by_name(child);
    if(!n->value.empty()) {
        if(child_node) *child_node = n;
        var = n->value;
    } else {
        if(child_node) *child_node = NULL;
    }
}


/* ----------------------------------------------------------------------------
 * Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 * child:
 *   Name of the child node.
 * var:
 *   The var to set. This is an integer.
 * child_node:
 *   If not-NULL, the node from whence the value came is placed here.
 *   NULL is placed if the property does not exist or has no value.
 */
void reader_setter::set(
    const string &child, size_t &var, data_node** child_node
) {
    data_node* n = node->get_child_by_name(child);
    if(!n->value.empty()) {
        if(child_node) *child_node = n;
        var = s2i(n->value);
    } else {
        if(child_node) *child_node = NULL;
    }
}


/* ----------------------------------------------------------------------------
 * Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 * child:
 *   Name of the child node.
 * var:
 *   The var to set. This is an integer.
 * child_node:
 *   If not-NULL, the node from whence the value came is placed here.
 *   NULL is placed if the property does not exist or has no value.
 */
void reader_setter::set(
    const string &child, int &var, data_node** child_node
) {
    data_node* n = node->get_child_by_name(child);
    if(!n->value.empty()) {
        if(child_node) *child_node = n;
        var = s2i(n->value);
    } else {
        if(child_node) *child_node = NULL;
    }
}


/* ----------------------------------------------------------------------------
 * Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 * child:
 *   Name of the child node.
 * var:
 *   The var to set. This is an unsigned char.
 * child_node:
 *   If not-NULL, the node from whence the value came is placed here.
 *   NULL is placed if the property does not exist or has no value.
 */
void reader_setter::set(
    const string &child, unsigned char &var, data_node** child_node
) {
    data_node* n = node->get_child_by_name(child);
    if(!n->value.empty()) {
        if(child_node) *child_node = n;
        var = s2i(n->value);
    } else {
        if(child_node) *child_node = NULL;
    }
}


/* ----------------------------------------------------------------------------
 * Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 * child:
 *   Name of the child node.
 * var:
 *   The var to set. This is a boolean.
 * child_node:
 *   If not-NULL, the node from whence the value came is placed here.
 *   NULL is placed if the property does not exist or has no value.
 */
void reader_setter::set(
    const string &child, bool &var, data_node** child_node
) {
    data_node* n = node->get_child_by_name(child);
    if(!n->value.empty()) {
        if(child_node) *child_node = n;
        var = s2b(n->value);
    } else {
        if(child_node) *child_node = NULL;
    }
}


/* ----------------------------------------------------------------------------
 * Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 * child:
 *   Name of the child node.
 * var:
 *   The var to set. This is a float.
 * child_node:
 *   If not-NULL, the node from whence the value came is placed here.
 *   NULL is placed if the property does not exist or has no value.
 */
void reader_setter::set(
    const string &child, float &var, data_node** child_node
) {
    data_node* n = node->get_child_by_name(child);
    if(!n->value.empty()) {
        if(child_node) *child_node = n;
        var = s2f(n->value);
    } else {
        if(child_node) *child_node = NULL;
    }
}


/* ----------------------------------------------------------------------------
 * Reads a child node's value, and uses it to set a variable.
 * Will not do anything if the child's value is empty.
 * child:
 *   Name of the child node.
 * var:
 *   The var to set. This is a point.
 * child_node:
 *   If not-NULL, the node from whence the value came is placed here.
 *   NULL is placed if the property does not exist or has no value.
 */
void reader_setter::set(
    const string &child, point &var, data_node** child_node
) {
    data_node* n = node->get_child_by_name(child);
    if(!n->value.empty()) {
        if(child_node) *child_node = n;
        var = s2p(n->value);
    } else {
        if(child_node) *child_node = NULL;
    }
}



/* ----------------------------------------------------------------------------
 * Creates a structure with sample info.
 * s:
 *   Allegro sample.
 * mixer:
 *   Allegro mixer.
 */
sample_struct::sample_struct(ALLEGRO_SAMPLE* s, ALLEGRO_MIXER* mixer) :
    sample(s),
    instance(NULL) {
    
    if(!s) return;
    instance = al_create_sample_instance(s);
    al_attach_sample_instance_to_mixer(instance, mixer);
}


/* ----------------------------------------------------------------------------
 * Destroys a structure with sample info.
 */
void sample_struct::destroy() {
    //TODO uncommenting this is causing a crash.
    //al_detach_sample_instance(instance);
    al_destroy_sample_instance(instance);
    al_destroy_sample(sample);
}


/* ----------------------------------------------------------------------------
 * Play the sample.
 * max_override_pos:
 *   Override the currently playing sound
 *   only if it's already in this position, or beyond.
 *   This is in seconds. 0 means always override. -1 means never override.
 * loop:
 *   Loop the sound?
 * gain:
 *   Volume, 0 - 1.
 * pan:
 *   Panning, 0 - 1 (0.5 is centered).
 * speed:
 *   Playing speed.
 */
void sample_struct::play(
    const float max_override_pos, const bool loop, const float gain,
    const float pan, const float speed
) {
    if(!sample || !instance) return;
    
    if(max_override_pos != 0 && al_get_sample_instance_playing(instance)) {
        float secs = al_get_sample_instance_position(instance) / (float) 44100;
        if(
            (secs < max_override_pos && max_override_pos > 0) ||
            max_override_pos == -1
        ) {
            return;
        }
    }
    
    al_set_sample_instance_playmode(
        instance, (loop ? ALLEGRO_PLAYMODE_LOOP : ALLEGRO_PLAYMODE_ONCE)
    );
    al_set_sample_instance_gain(instance, gain);
    al_set_sample_instance_pan(instance, pan);
    al_set_sample_instance_speed(instance, speed);
    
    al_set_sample_instance_position(instance, 0);
    al_set_sample_instance_playing( instance, true);
}


/* ----------------------------------------------------------------------------
 * Stops a playing sample instance.
 */
void sample_struct::stop() {
    al_set_sample_instance_playing(instance, false);
}


/* ----------------------------------------------------------------------------
 * Creates a "script var reader".
 * vars:
 *   Map of variables to read from.
 */
script_var_reader::script_var_reader(map<string, string> &vars) :
    vars(vars) {
    
}


/* ----------------------------------------------------------------------------
 * Assigns an Allegro color to the value of a given variable, if it exists.
 * Returns true if it exists, false if not.
 * name:
 *   Name of the variable to read.
 * dest:
 *   Destination for the value.
 */
bool script_var_reader::get(const string &name, ALLEGRO_COLOR &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = s2c(v->second);
    return true;
}


/* ----------------------------------------------------------------------------
 * Assigns a string to the value of a given variable, if it exists.
 * Returns true if it exists, false if not.
 * name:
 *   Name of the variable to read.
 * dest:
 *   Destination for the value.
 */
bool script_var_reader::get(const string &name, string &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = v->second;
    return true;
}


/* ----------------------------------------------------------------------------
 * Assigns a size_t to the value of a given variable, if it exists.
 * Returns true if it exists, false if not.
 * name:
 *   Name of the variable to read.
 * dest:
 *   Destination for the value.
 */
bool script_var_reader::get(const string &name, size_t &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = s2i(v->second);
    return true;
}


/* ----------------------------------------------------------------------------
 * Assigns an int to the value of a given variable, if it exists.
 * Returns true if it exists, false if not.
 * name:
 *   Name of the variable to read.
 * dest:
 *   Destination for the value.
 */
bool script_var_reader::get(const string &name, int &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = s2i(v->second);
    return true;
}


/* ----------------------------------------------------------------------------
 * Assigns an unsigned char to the value of a given variable, if it exists.
 * Returns true if it exists, false if not.
 * name:
 *   Name of the variable to read.
 * dest:
 *   Destination for the value.
 */
bool script_var_reader::get(const string &name, unsigned char &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = s2i(v->second);
    return true;
}


/* ----------------------------------------------------------------------------
 * Assigns a bool to the value of a given variable, if it exists.
 * Returns true if it exists, false if not.
 * name:
 *   Name of the variable to read.
 * dest:
 *   Destination for the value.
 */
bool script_var_reader::get(const string &name, bool &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = s2b(v->second);
    return true;
}


/* ----------------------------------------------------------------------------
 * Assigns a float to the value of a given variable, if it exists.
 * Returns true if it exists, false if not.
 * name:
 *   Name of the variable to read.
 * dest:
 *   Destination for the value.
 */
bool script_var_reader::get(const string &name, float &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = s2f(v->second);
    return true;
}


/* ----------------------------------------------------------------------------
 * Assigns a point to the value of a given variable, if it exists.
 * Returns true if it exists, false if not.
 * name:
 *   Name of the variable to read.
 * dest:
 *   Destination for the value.
 */
bool script_var_reader::get(const string &name, point &dest) const {
    auto v = vars.find(name);
    if(v == vars.end()) {
        return false;
    }
    dest = s2p(v->second);
    return true;
}



/* ----------------------------------------------------------------------------
 * Returns the name of a sector type, given its number.
 * Returns an empty string on error.
 * nr:
 *   Number of the sector type.
 */
string sector_types_manager::get_name(const unsigned char nr) const {
    if(nr < names.size()) return names[nr];
    return "";
}


/* ----------------------------------------------------------------------------
 * Returns the number of a sector type, given its name.
 * Returns 255 on error.
 * name:
 *   Name of the sector type.
 */
unsigned char sector_types_manager::get_nr(const string &name) const {
    for(unsigned char n = 0; n < names.size(); ++n) {
        if(names[n] == name) return n;
    }
    return 255;
}


/* ----------------------------------------------------------------------------
 * Returns the number of sector types registered.
 */
unsigned char sector_types_manager::get_nr_of_types() const {
    return names.size();
}


/* ----------------------------------------------------------------------------
 * Registers a new type of sector.
 * nr:
 *   Its ID number.
 * name:
 *   Its name.
 */
void sector_types_manager::register_type(
    const unsigned char nr, const string &name
) {
    if(nr >= names.size()) {
        names.insert(names.end(), (nr + 1) - names.size(), "");
    }
    names[nr] = name;
}






/* ----------------------------------------------------------------------------
 * Clears the list of registered subgroup types.
 */
void subgroup_type_manager::clear() {
    for(size_t t = 0; t < types.size(); ++t) {
        delete types[t];
    }
    types.clear();
}


/* ----------------------------------------------------------------------------
 * Returns the first registered subgroup type.
 */
subgroup_type* subgroup_type_manager::get_first_type() const {
    return types.front();
}


/* ----------------------------------------------------------------------------
 * Returns the subgroup type that comes after the given type.
 * sgt:
 *   Subgroup type to iterate from.
 */
subgroup_type* subgroup_type_manager::get_next_type(
    subgroup_type* sgt
) const {
    for(size_t t = 0; t < types.size(); ++t) {
        if(types[t] == sgt) {
            return get_next_in_vector(types, t);
        }
    }
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Returns the subgroup type that comes before the given type.
 * sgt:
 *   Subgroup type to iterate from.
 */
subgroup_type* subgroup_type_manager::get_prev_type(
    subgroup_type* sgt
) const {
    for(size_t t = 0; t < types.size(); ++t) {
        if(types[t] == sgt) {
            return get_prev_in_vector(types, t);
        }
    }
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Returns the type of subgroup corresponding to the parameters.
 * Returns NULL if not found.
 * category:
 *   The category of subgroup type. Pikmin, leader, bomb-rock, etc.
 * specific_type:
 *   Specific type of mob, if you want to specify further.
 */
subgroup_type* subgroup_type_manager::get_type(
    const SUBGROUP_TYPE_CATEGORIES category,
    mob_type* specific_type
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
    return NULL;
}


/* ----------------------------------------------------------------------------
 * Registers a new type of subgroup.
 * category:
 *   The category of subgroup type. Pikmin, leader, bomb-rock, etc.
 * specific_type:
 *   Specific type of mob, if you want to specify further.
 * icon:
 *   If not NULL, use this icon to represent this subgroup.
 */
void subgroup_type_manager::register_type(
    const SUBGROUP_TYPE_CATEGORIES category,
    mob_type* specific_type,
    ALLEGRO_BITMAP* icon
) {
    subgroup_type* new_sg_type = new subgroup_type();
    
    new_sg_type->category = category;
    new_sg_type->specific_type = specific_type;
    new_sg_type->icon = icon;
    
    types.push_back(new_sg_type);
}


/* ----------------------------------------------------------------------------
 * Creates a system asset list struct.
 */
system_asset_list::system_asset_list():
    bmp_bubble_box(nullptr),
    bmp_checkbox_check(nullptr),
    bmp_cursor(nullptr),
    bmp_cursor_invalid(nullptr),
    bmp_enemy_spirit(nullptr),
    bmp_icon(nullptr),
    bmp_idle_glow(nullptr),
    bmp_mouse_cursor(nullptr),
    bmp_mouse_wd_icon(nullptr),
    bmp_mouse_wu_icon(nullptr),
    bmp_notification(nullptr),
    bmp_pikmin_silhouette(nullptr),
    bmp_pikmin_spirit(nullptr),
    bmp_rock(nullptr),
    bmp_shadow(nullptr),
    bmp_smack(nullptr),
    bmp_smoke(nullptr),
    bmp_sparkle(nullptr),
    bmp_spotlight(nullptr),
    bmp_swarm_arrow(nullptr),
    bmp_wave_ring(nullptr) {
    
    bmp_mouse_button_icon[0] = NULL;
    bmp_mouse_button_icon[1] = NULL;
    bmp_mouse_button_icon[2] = NULL;
}



/* ----------------------------------------------------------------------------
 * Creates a timer.
 * duration:
 *   How long before it ticks, in seconds.
 * on_end:
 *   Code to run when time ends.
 */
timer::timer(float duration, const std::function<void()> &on_end) :
    time_left(0),
    duration(duration),
    on_end(on_end) {
    
    
}


/* ----------------------------------------------------------------------------
 * Destroys a timer.
 */
timer::~timer() {
    //TODO Valgrind detects a leak with on_end...
}


/* ----------------------------------------------------------------------------
 * Returns the ratio of time left (i.e. 0 if done, 1 if all time is left).
 */
float timer::get_ratio_left() const {
    return time_left / duration;
}



/* ----------------------------------------------------------------------------
 * Starts a timer.
 * can_restart:
 *   If false, calling this while the timer is still ticking down
 *   will not do anything.
 */
void timer::start(const bool can_restart) {
    if(!can_restart && time_left > 0) return;
    time_left = duration;
}


/* ----------------------------------------------------------------------------
 * Starts a timer, but sets a new duration.
 * new_duration:
 *   Its new duration.
 */
void timer::start(const float new_duration) {
    duration = new_duration;
    start();
}


/* ----------------------------------------------------------------------------
 * Stops a timer, without executing the on_end callback.
 */
void timer::stop() {
    time_left = 0.0f;
}


/* ----------------------------------------------------------------------------
 * Ticks a timer.
 * amount:
 *   Time to tick.
 */
void timer::tick(const float amount) {
    if(time_left == 0.0f) return;
    time_left = std::max(time_left - amount, 0.0f);
    if(time_left == 0.0f && on_end) {
        on_end();
    }
}


/* ----------------------------------------------------------------------------
 * Creates a whistle struct.
 */
whistle_struct::whistle_struct() :
    fade_radius(0.0f),
    fade_timer(WHISTLE_FADE_TIME),
    next_dot_timer(WHISTLE_DOT_INTERVAL),
    next_ring_timer(WHISTLE_RINGS_INTERVAL),
    radius(0.0f),
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
            sum_and_wrap(ring_prev_color, 1, N_WHISTLE_RING_COLORS);
    };
    
}


/* ----------------------------------------------------------------------------
 * Stuff to do when a leader starts whistling.
 */
void whistle_struct::start_whistling() {
    for(unsigned char d = 0; d < 6; ++d) {
        dot_radius[d] = -1;
    }
    fade_timer.start();
    fade_radius = 0;
    whistling = true;
}


/* ----------------------------------------------------------------------------
 * Stuff to do when a leader stops whistling.
 */
void whistle_struct::stop_whistling() {
    whistling = false;
    fade_timer.start();
    fade_radius = radius;
    radius = 0;
}


/* ----------------------------------------------------------------------------
 * Ticks one frame of gameplay.
 * delta_t:
 *   How many seconds to tick by.
 * whistle_range:
 *   How far the whistle can reach from the cursor center.
 * leader_to_cursor_dist:
 *   Distance between the leader and the cursor.
 */
void whistle_struct::tick(
    const float delta_t,
    const float whistle_range, const dist &leader_to_cursor_dist
) {
    fade_timer.tick(delta_t);
    
    if(whistling) {
        //Create rings.
        next_ring_timer.tick(delta_t);
        
        if(game.options.pretty_whistle) {
            next_dot_timer.tick(delta_t);
        }
        
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
        rings[r] += WHISTLE_RING_SPEED * delta_t;
        if(leader_to_cursor_dist < rings[r]) {
            rings.erase(rings.begin() + r);
            ring_colors.erase(ring_colors.begin() + r);
        } else {
            r++;
        }
    }
}
