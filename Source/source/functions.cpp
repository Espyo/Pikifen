/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Globally accessible functions.
 */

#define _USE_MATH_DEFINES

//Disable warning about localtime being deprecated.
#pragma warning(disable : 4996)

#include <algorithm>
#include <iostream>
#include <math.h>
#include <sstream>
#include <stdlib.h>
#include <signal.h>
#include <typeinfo>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "backtrace.h"
#include "editors/animation_editor/editor.h"
#include "const.h"
#include "drawing.h"
#include "functions.h"
#include "init.h"
#include "menus.h"
#include "utils/string_utils.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Does sector s1 cast a shadow onto sector s2?
 */
bool casts_shadow(sector* s1, sector* s2) {
    if(!s1 || !s2) return false;
    if(s1->is_bottomless_pit) return false;
    if(s2->is_bottomless_pit) return false;
    if(s1->z > s2->z && s1->always_cast_shadow) return true;
    if(s1->z <= s2->z + SECTOR_STEP) return false;
    return true;
}


/* ----------------------------------------------------------------------------
 * Returns the color that was provided, but with the alpha changed.
 * color: The color to change the alpha on.
 * a:     The new alpha, [0-255].
 */
ALLEGRO_COLOR change_alpha(const ALLEGRO_COLOR &c, const unsigned char a) {
    ALLEGRO_COLOR c2;
    c2.r = c.r; c2.g = c.g; c2.b = c.b;
    c2.a = a / 255.0;
    return c2;
}


/* ----------------------------------------------------------------------------
 * Returns the color provided, but darker or lighter by l amount.
 * color: The color to change the lighting on.
 * l:     Lighting amount, positive or negative.
 */
ALLEGRO_COLOR change_color_lighting(const ALLEGRO_COLOR &c, const float l) {
    ALLEGRO_COLOR c2;
    c2.r = clamp(c.r + l, 0.0f, 1.0f);
    c2.g = clamp(c.g + l, 0.0f, 1.0f);
    c2.b = clamp(c.b + l, 0.0f, 1.0f);
    c2.a = c.a;
    return c2;
}


/* ----------------------------------------------------------------------------
 * Changes the game's state.
 */
void change_game_state(unsigned int new_state) {
    if(cur_game_state_nr != INVALID) {
        game_states[cur_game_state_nr]->unload();
    }
    cur_game_state_nr = new_state;
    game_states[cur_game_state_nr]->load();
    
    //Because during the loading screens, there is no activity, on the
    //next frame, the game will assume the time between that and the last
    //non-loading frame is normal. This could be something like 2 seconds.
    //Let's reset the delta_t, then.
    reset_delta_t = true;
    
}


/* ----------------------------------------------------------------------------
 * Clears the textures of the area's sectors from memory.
 */
void clear_area_textures() {
    for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
        sector* s_ptr = cur_area_data.sectors[s];
        if(
            s_ptr->texture_info.bitmap &&
            s_ptr->texture_info.bitmap != bmp_error
        ) {
            textures.detach(s_ptr->texture_info.file_name);
            s_ptr->texture_info.bitmap = NULL;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Purposely crashes the engine, reporting as much information as possible to
 * the logs. Used when a fatal problem occurs.
 * reason:      Explanation of the type of crash (assert, SIGSEGV, etc.).
 * info:        Any extra information to report to the logs.
 * exit_status: Program exit status.
 */
void crash(const string &reason, const string &info, const int exit_status) {

    if(display) {
        ALLEGRO_BITMAP* backbuffer = al_get_backbuffer(display);
        if(backbuffer) {
            al_save_bitmap(
                ("Crash " + get_current_time(false) + ".png").c_str(),
                backbuffer
            );
        }
    }
    
    string error_str = "Program crash!\n";
    error_str +=
        "  Reason: " + reason + ".\n"
        "  Info: " + info + "\n"
        "  Time: " + get_current_time(true) + ".\n";
    if(errors_reported_today > 0) {
        error_str += "  Error log has messages!\n";
    }
    error_str +=
        "  Game state: " + (
            cur_game_state_nr == INVALID ? "None" : i2s(cur_game_state_nr)
        ) + ". delta_t: " + (
            delta_t == 0.0f ? "0" :
            f2s(delta_t) + " (" + f2s(1 / delta_t) + " FPS)"
        ) + ".\n"
        "  Mob count: " + i2s(mobs.size()) + ". "
        "Bitmaps loaded: " + i2s(bitmaps.get_list_size()) + " (" +
        i2s(bitmaps.get_total_calls()) + " total calls).";
        
    log_error(error_str);
    
    show_message_box(
        NULL, "Program crash!",
        "Pikifen has crashed!",
        "Sorry about that! Please read the readme file to know what you "
        "can do to help me fix it. Thanks!",
        NULL,
        ALLEGRO_MESSAGEBOX_ERROR
    );
    
    exit(exit_status);
}


/* ----------------------------------------------------------------------------
 * Stores the names of all files in a folder into a vector.
 * folder_name: Name of the folder.
 * folders:     If true, only read folders. If false, only read files.
 */
vector<string> folder_to_vector(
    string folder_name, const bool folders, bool* folder_found
) {
    vector<string> v;
    
    if(folder_name.empty()) {
        if(folder_found) *folder_found = false;
        return v;
    }
    
    //Normalize the folder's path.
    folder_name = standardize_path(folder_name);
    
    ALLEGRO_FS_ENTRY* folder =
        al_create_fs_entry(folder_name.c_str());
    if(!folder || !al_open_directory(folder)) {
        if(folder_found) *folder_found = false;
        return v;
    }
    
    
    ALLEGRO_FS_ENTRY* entry = NULL;
    while((entry = al_read_directory(folder)) != NULL) {
        if(
            folders ==
            ((al_get_fs_entry_mode(entry) & ALLEGRO_FILEMODE_ISDIR) > 0)
        ) {
        
            string entry_name =
                standardize_path(al_get_fs_entry_name(entry));
                
            //Only save what's after the final slash.
            size_t pos = entry_name.find_last_of("/");
            if(pos != string::npos) {
                entry_name =
                    entry_name.substr(pos + 1, entry_name.size() - pos - 1);
            }
            
            v.push_back(entry_name);
        }
        al_destroy_fs_entry(entry);
    }
    al_close_directory(folder);
    al_destroy_fs_entry(folder);
    
    
    sort(v.begin(), v.end(), [] (string s1, string s2) -> bool {
        return str_to_lower(s1) < str_to_lower(s2);
    });
    
    if(folder_found) *folder_found = true;
    return v;
}


/* ----------------------------------------------------------------------------
 * Returns the blackout effect's strength
 * for the current time and weather.
 */
unsigned char get_blackout_strength() {
    size_t n_points = cur_area_data.weather_condition.blackout_strength.size();
    
    if(n_points == 0) {
        return 0;
    } else if(n_points == 1) {
        return cur_area_data.weather_condition.blackout_strength[0].second;
    }
    
    for(size_t p = 0; p < n_points - 1; ++p) {
        auto cur_ptr = &cur_area_data.weather_condition.blackout_strength[p];
        auto next_ptr =
            &cur_area_data.weather_condition.blackout_strength[p + 1];
            
        if(day_minutes >= cur_ptr->first && day_minutes < next_ptr->first) {
        
            return
                interpolate_number(
                    day_minutes,
                    cur_ptr->first, next_ptr->first,
                    cur_ptr->second, next_ptr->second
                );
        }
    }
    
    //If anything goes wrong, return a failsafe.
    return 0;
}


/* ----------------------------------------------------------------------------
 * Returns the mob that is closest to the mouse cursor.
 */
mob* get_closest_mob_to_cursor() {
    dist closest_mob_to_cursor_dist = 0;
    mob* closest_mob_to_cursor = NULL;
    
    for(size_t m = 0; m < mobs.size(); ++m) {
        mob* m_ptr = mobs[m];
        
        if(!m_ptr->fsm.cur_state) continue;
        
        dist d = dist(mouse_cursor_w, m_ptr->pos);
        if(!closest_mob_to_cursor || d < closest_mob_to_cursor_dist) {
            closest_mob_to_cursor = m_ptr;
            closest_mob_to_cursor_dist = d;
        }
    }
    
    return closest_mob_to_cursor;
}


/* ----------------------------------------------------------------------------
 * Returns a string representing the current date and time.
 */
string get_current_time(const bool slashes_for_day) {
    time_t tt;
    time(&tt);
    struct tm t = *localtime(&tt);
    return
        i2s(t.tm_year + 1900) +
        (slashes_for_day ? "/" : "-") +
        leading_zero(t.tm_mon + 1) +
        (slashes_for_day ? "/" : "-") +
        leading_zero(t.tm_mday) + " " +
        leading_zero(t.tm_hour) + ":" +
        leading_zero(t.tm_min) + ":" +
        leading_zero(t.tm_sec);
}


/* ----------------------------------------------------------------------------
 * Returns the daylight effect color for the current time and weather.
 */
ALLEGRO_COLOR get_daylight_color() {
    size_t n_points = cur_area_data.weather_condition.daylight.size();
    
    if(n_points == 0) {
        return al_map_rgba(255, 255, 255, 0);
    } else if(n_points == 1) {
        return cur_area_data.weather_condition.daylight[0].second;
    }
    
    for(size_t p = 0; p < n_points - 1; ++p) {
        auto cur_ptr = &cur_area_data.weather_condition.daylight[p];
        auto next_ptr = &cur_area_data.weather_condition.daylight[p + 1];
        
        if(day_minutes >= cur_ptr->first && day_minutes < next_ptr->first) {
        
            return
                interpolate_color(
                    day_minutes,
                    cur_ptr->first, next_ptr->first,
                    cur_ptr->second, next_ptr->second
                );
        }
    }
    
    //If anything goes wrong, return a failsafe.
    return al_map_rgba(255, 255, 255, 0);
}


/* ----------------------------------------------------------------------------
 * Returns the fog color for the current time and weather.
 */
ALLEGRO_COLOR get_fog_color() {
    size_t n_points = cur_area_data.weather_condition.fog_color.size();
    
    if(n_points == 0) {
        return al_map_rgba(255, 255, 255, 0);
    } else if(n_points == 1) {
        return cur_area_data.weather_condition.fog_color[0].second;
    }
    
    for(size_t p = 0; p < n_points - 1; ++p) {
        auto cur_ptr = &cur_area_data.weather_condition.fog_color[p];
        auto next_ptr = &cur_area_data.weather_condition.fog_color[p + 1];
        
        if(day_minutes >= cur_ptr->first && day_minutes < next_ptr->first) {
        
            return
                interpolate_color(
                    day_minutes,
                    cur_ptr->first, next_ptr->first,
                    cur_ptr->second, next_ptr->second
                );
        }
    }
    
    //If anything goes wrong, return a failsafe.
    return al_map_rgba(255, 255, 255, 0);
}


/* ----------------------------------------------------------------------------
 * Returns the highest height a thrown mob can reach, given the Z speed
 * of the throw. This is only the theoritical max; framerate may tamper
 * with the exact value.
 */
float get_max_throw_height(const float throw_strength) {
    /* Formula from
     * http://www.dummies.com/education/science/physics/
     * how-to-calculate-the-maximum-height-of-a-projectile/
     */
    return (-pow(throw_strength, 2)) / (2 * GRAVITY_ADDER);
}


/* ----------------------------------------------------------------------------
 * Returns the width and height of a block of multi-line text.
 * Lines are split by a single "\n" character.
 * These are the dimensions of a bitmap
 * that would hold a drawing by draw_text_lines().
 * font:  The text's font.
 * text:  The text.
 * ret_w: The width gets returned here, if not NULL.
 * ret_h: The height gets returned here, if not NULL.
 */
void get_multiline_text_dimensions(
    const ALLEGRO_FONT* const font, const string &text, int* ret_w, int* ret_h
) {
    vector<string> lines = split(text, "\n", true);
    int fh = al_get_font_line_height(font);
    size_t n_lines = lines.size();
    
    if(ret_h) *ret_h = max(0, (int) ((fh + 1) * n_lines) - 1);
    
    if(ret_w) {
        int largest_w = 0;
        for(size_t l = 0; l < lines.size(); ++l) {
            largest_w =
                max(
                    largest_w, al_get_text_width(font, lines[l].c_str())
                );
        }
        
        *ret_w = largest_w;
    }
}


/* ----------------------------------------------------------------------------
 * Returns the sun strength for the current time and weather.
 */
float get_sun_strength() {
    size_t n_points = cur_area_data.weather_condition.sun_strength.size();
    
    if(n_points == 0) {
        return 1.0f;
    } else if(n_points == 1) {
        return cur_area_data.weather_condition.sun_strength[0].second;
    }
    
    for(size_t p = 0; p < n_points - 1; ++p) {
        auto cur_ptr = &cur_area_data.weather_condition.sun_strength[p];
        auto next_ptr = &cur_area_data.weather_condition.sun_strength[p + 1];
        
        if(day_minutes >= cur_ptr->first && day_minutes < next_ptr->first) {
        
            return
                interpolate_number(
                    day_minutes,
                    cur_ptr->first, next_ptr->first,
                    cur_ptr->second, next_ptr->second
                ) / 255.0f;
        }
    }
    
    //If anything goes wrong, return a failsafe.
    return 1.0f;
}

/* ----------------------------------------------------------------------------
 * Returns the Z speed for a mob throw, given a height strength multiplier.
 */
float get_throw_z_speed(const float strength_multiplier) {
    return -(GRAVITY_ADDER) * (THROW_STRENGTH_MULTIPLIER * strength_multiplier);
}


/* ----------------------------------------------------------------------------
 * Returns the value of a var on the vars listing of a mob's spawn.
 */
string get_var_value(
    const string &vars_string, const string &var, const string &def
) {
    vector<string> vars = semicolon_list_to_vector(vars_string);
    
    for(size_t v = 0; v < vars.size(); ++v) {
        size_t equals_pos = vars[v].find("=");
        string var_name = trim_spaces(vars[v].substr(0, equals_pos));
        
        if(var_name != var) continue;
        
        return
            trim_spaces(
                vars[v].substr(
                    equals_pos + 1, vars[v].size() - (equals_pos + 1)
                ),
                true
            );
    }
    
    return def;
}


/* ----------------------------------------------------------------------------
 * Given a string representation of mob script variables,
 * returns two vectors: one for the variable names, and one for their values.
 */
void get_var_vectors(
    const string &vars_string,
    vector<string> &var_names, vector<string> &var_values
) {
    vector<string> raw_vars = semicolon_list_to_vector(vars_string);
    
    for(size_t v = 0; v < raw_vars.size(); ++v) {
        vector<string> raw_parts = split(raw_vars[v], "=");
        if(raw_parts.size() == 0) {
            var_names.push_back("");
        } else {
            var_names.push_back(trim_spaces(raw_parts[0]));
        }
        if(raw_parts.size() == 1) {
            var_values.push_back("");
        } else {
            var_values.push_back(trim_spaces(raw_parts[1]));
        }
    }
}


//Maximum length a wall shadow can be.
const float MAX_WALL_SHADOW_LENGTH = 50.0f;
//Minimum length a wall shadow can be.
const float MIN_WALL_SHADOW_LENGTH = 8.0f;
//Wall shadow lengths are the sector height difference multiplied by this.
const float WALL_SHADOW_LENGTH_MULT = 0.2f;

/* ----------------------------------------------------------------------------
 * Returns the length a wall's shadow should be.
 * height_difference: Difference in height between the sector casting the shadow
 * * and the one the shadow is being cast on.
 */
float get_wall_shadow_length(const float height_difference) {
    return
        clamp(
            height_difference * WALL_SHADOW_LENGTH_MULT,
            MIN_WALL_SHADOW_LENGTH,
            MAX_WALL_SHADOW_LENGTH
        );
}


/* ----------------------------------------------------------------------------
 * Auxiliary function that returns a table used in the weather configs.
 */
vector<pair<size_t, string> > get_weather_table(data_node* node) {
    vector<pair<size_t, string> > table;
    size_t n_points = node->get_nr_of_children();
    
    bool have_midnight = false;
    
    for(size_t p = 0; p < n_points; ++p) {
        data_node* point_node = node->get_child(p);
        
        size_t point_time = s2i(point_node->name);
        string point_value = point_node->value;
        
        table.push_back(make_pair(point_time, point_value));
        
        if(point_time == 24 * 60) have_midnight = true;
    }
    
    sort(
        table.begin(), table.end(),
        [] (
            pair<size_t, string> p1,
            pair<size_t, string> p2
    ) -> bool {
        return p1.first < p2.first;
    }
    );
    
    if(!table.empty()) {
        if(!have_midnight) {
            //If there is no data for the last hour,
            //use the data from the first point
            //(this is because the day loops after 24:00;
            //needed for interpolation).
            table.push_back(
                make_pair(24 * 60, table[0].second)
            );
        }
    }
    
    return table;
}


/* ----------------------------------------------------------------------------
 * Returns the interpolation between two colors, given a number in an interval.
 * n: The number.
 * n1, n2: The interval the number falls on.
 * * The closer to n1, the closer the final color is to c1.
 * c1, c2: Colors.
 */
ALLEGRO_COLOR interpolate_color(
    const float n, const float n1, const float n2,
    const ALLEGRO_COLOR &c1, const ALLEGRO_COLOR &c2
) {
    float progress = (float) (n - n1) / (float) (n2 - n1);
    return
        al_map_rgba_f(
            c1.r + progress * (c2.r - c1.r),
            c1.g + progress * (c2.g - c1.g),
            c1.b + progress * (c2.b - c1.b),
            c1.a + progress * (c2.a - c1.a)
        );
}


/* ----------------------------------------------------------------------------
 * Prints something onto the error log.
 * s: String that represents the error.
 * d: If not null, this will be used to obtain the file name
 *   and line that caused the error.
 */
void log_error(string s, data_node* d) {
    if(d) {
        s += " (" + d->file_name;
        if(d->line_nr != 0) s += " line " + i2s(d->line_nr);
        s += ")";
    }
    s += "\n";
    
    cout << s;
    
    if(errors_reported_today == 0) {
        s =
            "\n" +
            get_current_time(true) +
            "; Pikifen version " +
            i2s(VERSION_MAJOR) + "." + i2s(VERSION_MINOR) +
            "." + i2s(VERSION_REV) + "\n" + s;
    }
    
    string prev_error_log;
    string line;
    ALLEGRO_FILE* file_i = al_fopen(ERROR_LOG_FILE_PATH.c_str(), "r");
    if(file_i) {
        while(!al_feof(file_i)) {
            getline(file_i, line);
            prev_error_log += line + "\n";
        }
        prev_error_log.erase(prev_error_log.size() - 1);
        al_fclose(file_i);
    }
    
    ALLEGRO_FILE* file_o = al_fopen(ERROR_LOG_FILE_PATH.c_str(), "w");
    if(file_o) {
        al_fwrite(file_o, prev_error_log + s);
        al_fclose(file_o);
    }
    
    errors_reported_today++;
}


/* ----------------------------------------------------------------------------
 * Converts a point to a string.
 * If z is present, the third word is placed there.
 */
string p2s(const point &p, float* z) {
    return f2s(p.x) + " " + f2s(p.y) + (z ? " " + f2s(*z) : "");
}


/* ----------------------------------------------------------------------------
 * Prints a bit of info onto the screen, for some seconds.
 */
void print_info(const string &text) {
    info_print_text = text;
    info_print_timer.start();
}


/* ----------------------------------------------------------------------------
 * Creates and opens an Allegro native file dialog, and returns
 * the user's choice(s).
 * The arguments are the same you'd pass to al_create_native_file_dialog().
 */
vector<string> prompt_file_dialog(
    const string &initial_path, const string &title,
    const string &patterns, const int mode
) {
    ALLEGRO_FILECHOOSER* dialog =
        al_create_native_file_dialog(
            initial_path.c_str(), title.c_str(),
            patterns.c_str(), mode
        );
    al_show_native_file_dialog(display, dialog);
    
    //Reset the locale, which gets set by Allegro's native dialogs...
    //and breaks s2f().
    setlocale(LC_ALL, "C");
    
    vector<string> result;
    size_t n_choices = al_get_native_file_dialog_count(dialog);
    for(size_t c = 0; c < n_choices; ++c) {
        result.push_back(
            standardize_path(
                al_get_native_file_dialog_path(dialog, c)
            )
        );
    }
    
    al_destroy_native_file_dialog(dialog);
    return result;
}


/* ----------------------------------------------------------------------------
 * Creates and opens an Allegro native file dialog, and returns
 * the user's choice(s), but confines the results to a specific folder.
 * The result pointer returns 0 on success, 1 if the one or more choices
 * do not belong to the specified folder, and 2 if the user canceled.
 * The folder argument is the folder to lock to, without the ending slash.
 * The other arguments are the same you'd pass to prompt_file_dialog().
 * The list of choices that are returned only have the file name, not the
 * rest of the path. Choices can also be contained inside subfolders of the
 * specified folder.
 */
vector<string> prompt_file_dialog_locked_to_folder(
    const string &folder, const string &title,
    const string &patterns, const int mode, unsigned char* result
) {
    vector<string> f =
        prompt_file_dialog(folder + "/", title, patterns, mode);
        
    if(f.empty() || f[0].empty()) {
        *result = 2;
        return vector<string>();
    }
    
    for(size_t fi = 0; fi < f.size(); ++fi) {
        size_t folder_pos = f[0].find(folder);
        if(folder_pos == string::npos) {
            //This isn't in the specified folder!
            *result = 1;
            return vector<string>();
        } else {
            f[fi] =
                f[fi].substr(folder_pos + folder.size() + 1, string::npos);
        }
    }
    
    *result = 0;
    return f;
}


/* ----------------------------------------------------------------------------
 * Basically, it destroys and recreates a bitmap.
 * The main purpose of this is to update its mipmap.
 * b: The bitmap.
 */
ALLEGRO_BITMAP* recreate_bitmap(ALLEGRO_BITMAP* b) {
    ALLEGRO_BITMAP* fixed_mipmap = al_clone_bitmap(b);
    al_destroy_bitmap(b);
    return fixed_mipmap;
}


/* ----------------------------------------------------------------------------
 * Reports a fatal error to the user and shuts down the program.
 */
void report_fatal_error(const string &s, data_node* dn) {
    log_error(s, dn);
    
    show_message_box(
        NULL, "Fatal error!",
        "Pikifen has encountered a fatal error!",
        s.c_str(),
        NULL,
        ALLEGRO_MESSAGEBOX_ERROR
    );
    
    exit(-1);
    
}


/* ----------------------------------------------------------------------------
 * Converts a string to a point.
 * If z is present, the third word is placed there.
 */
point s2p(const string &s, float* z) {
    vector<string> words = split(s);
    point p;
    if(words.size() >= 1) {
        p.x = s2f(words[0]);
    }
    if(words.size() >= 2) {
        p.y = s2f(words[1]);
    }
    if(z && words.size() >= 3) {
        *z = s2f(words[2]);
    }
    return p;
}


/* ----------------------------------------------------------------------------
 * Saves the creator tools settings.
 */
void save_creator_tools() {
    data_node file("", "");
    
    file.add(
        new data_node("enabled", b2s(creator_tools_enabled))
    );
    
    for(unsigned char k = 0; k < 20; k++) {
        string tool_key;
        string tool_name;
        if(k < 10) {
            //The first ten indexes are the F2 - F11 keys.
            tool_key = "f" + i2s(k + 2);
        } else {
            //The second ten indexes are the 0 - 9 keys.
            tool_key = i2s(k - 10);
        }
        
        if(creator_tool_keys[k] == CREATOR_TOOL_AREA_IMAGE) {
            tool_name = "area_image";
        } else if(creator_tool_keys[k] == CREATOR_TOOL_CHANGE_SPEED) {
            tool_name = "change_speed";
        } else if(creator_tool_keys[k] == CREATOR_TOOL_GEOMETRY_INFO) {
            tool_name = "geometry_info";
        } else if(creator_tool_keys[k] == CREATOR_TOOL_HITBOXES) {
            tool_name = "hitboxes";
        } else if(creator_tool_keys[k] == CREATOR_TOOL_HURT_MOB) {
            tool_name = "hurt_mob";
        } else if(creator_tool_keys[k] == CREATOR_TOOL_MOB_INFO) {
            tool_name = "mob_info";
        } else if(creator_tool_keys[k] == CREATOR_TOOL_NEW_PIKMIN) {
            tool_name = "new_pikmin";
        } else if(creator_tool_keys[k] == CREATOR_TOOL_TELEPORT) {
            tool_name = "teleport";
        }
        
        file.add(new data_node(tool_key, tool_name));
    }
    
    file.add(
        new data_node(
            "area_image_file_name", creator_tool_area_image_name
        )
    );
    file.add(
        new data_node(
            "area_image_mobs", b2s(creator_tool_area_image_mobs)
        )
    );
    file.add(
        new data_node(
            "area_image_shadows", b2s(creator_tool_area_image_shadows)
        )
    );
    file.add(
        new data_node(
            "area_image_size", i2s(creator_tool_area_image_size)
        )
    );
    file.add(
        new data_node(
            "change_speed_multiplier", f2s(creator_tool_change_speed_mult)
        )
    );
    file.add(
        new data_node(
            "mob_hurting_percentage", f2s(creator_tool_mob_hurting_ratio * 100)
        )
    );
    
    file.add(
        new data_node(
            "auto_start_option", creator_tool_auto_start_option
        )
    );
    file.add(
        new data_node(
            "auto_start_mode", creator_tool_auto_start_mode
        )
    );
    
    file.save_file(CREATOR_TOOLS_FILE_PATH, true, true);
}


/* ----------------------------------------------------------------------------
 * Saves the player's options.
 */
void save_options() {
    data_node file("", "");
    
    //First, group the controls by action and player.
    map<string, string> grouped_controls;
    
    for(unsigned char p = 0; p < MAX_PLAYERS; ++p) {
        string prefix = "p" + i2s((p + 1)) + "_";
        for(size_t b = 0; b < N_BUTTONS; ++b) {
            string option_name = buttons.list[b].option_name;
            if(option_name.empty()) continue;
            grouped_controls[prefix + option_name].clear();
        }
    }
    
    //Write down their control strings.
    for(size_t p = 0; p < MAX_PLAYERS; p++) {
        size_t n_controls = controls[p].size();
        for(size_t c = 0; c < n_controls; ++c) {
            string name = "p" + i2s(p + 1) + "_";
            
            for(size_t b = 0; b < N_BUTTONS; ++b) {
                if(buttons.list[b].option_name.empty()) continue;
                if(controls[p][c].action == buttons.list[b].id) {
                    name += buttons.list[b].option_name;
                    break;
                }
            }
            
            grouped_controls[name] += controls[p][c].stringify() + ";";
        }
    }
    
    //Save controls.
    for(auto c = grouped_controls.begin(); c != grouped_controls.end(); ++c) {
        //Remove the final character, which is always an extra semicolon.
        if(c->second.size()) c->second.erase(c->second.size() - 1);
        
        file.add(new data_node(c->first, c->second));
    }
    
    for(unsigned char p = 0; p < MAX_PLAYERS; ++p) {
        file.add(
            new data_node(
                "p" + i2s((p + 1)) + "_mouse_moves_cursor",
                b2s(mouse_moves_cursor[p])
            )
        );
    }
    
    //Other options.
    file.add(
        new data_node(
            "animation_editor_mmb_pan", b2s(animation_editor_mmb_pan)
        )
    );
    file.add(
        new data_node(
            "area_editor_backup_interval", f2s(area_editor_backup_interval)
        )
    );
    file.add(
        new data_node(
            "area_editor_grid_interval", i2s(area_editor_grid_interval)
        )
    );
    file.add(
        new data_node(
            "area_editor_mmb_pan", b2s(area_editor_mmb_pan)
        )
    );
    file.add(
        new data_node(
            "area_editor_show_edge_length", b2s(area_editor_show_edge_length)
        )
    );
    file.add(
        new data_node(
            "area_editor_undo_limit", i2s(area_editor_undo_limit)
        )
    );
    file.add(
        new data_node(
            "area_editor_view_mode", i2s(area_editor_view_mode)
        )
    );
    file.add(
        new data_node(
            "draw_cursor_trail", b2s(draw_cursor_trail)
        )
    );
    file.add(new data_node("fps", i2s(game_fps)));
    file.add(new data_node("fullscreen", b2s(intended_scr_fullscreen)));
    file.add(
        new data_node("joystick_min_deadzone", f2s(joystick_min_deadzone))
    );
    file.add(
        new data_node("joystick_max_deadzone", f2s(joystick_max_deadzone))
    );
    file.add(new data_node("max_particles", i2s(max_particles)));
    file.add(new data_node("middle_zoom_level", f2s(zoom_mid_level)));
    file.add(new data_node("mipmaps", b2s(mipmaps_enabled)));
    file.add(new data_node("pretty_whistle", b2s(pretty_whistle)));
    file.add(
        new data_node(
            "resolution", i2s(intended_scr_w) + " " + i2s(intended_scr_h)
        )
    );
    file.add(new data_node("smooth_scaling", b2s(smooth_scaling)));
    file.add(new data_node("window_position_hack", b2s(window_position_hack)));
    
    for(size_t h = 0; h < ANIMATION_EDITOR_HISTORY_SIZE; ++h) {
        file.add(
            new data_node(
                "animation_editor_history_" + i2s(h + 1),
                animation_editor_history[h]
            )
        );
    }
    
    file.save_file(OPTIONS_FILE_PATH, true, true);
}


/* ----------------------------------------------------------------------------
 * Saves the current backbuffer onto a file.
 * In other words, dumps a screenshot.
 */
void save_screenshot() {
    string base_file_name = "Screenshot " + get_current_time(false);
    
    //Check if a file with this name already exists.
    vector<string> files = folder_to_vector(".", false);
    size_t variant_nr = 1;
    string final_file_name = base_file_name;
    bool valid_name = false;
    
    do {
    
        if(
            find(files.begin(), files.end(), final_file_name + ".png")
            == files.end()
        ) {
            //File name not found.
            //Go ahead and create a screenshot with this name.
            valid_name = true;
        } else {
            variant_nr++;
            final_file_name = base_file_name + " " + i2s(variant_nr);
        }
        
    } while(!valid_name);
    
    al_save_bitmap(
        (final_file_name + ".png").c_str(),
        al_get_backbuffer(display)
    );
}


/* ----------------------------------------------------------------------------
 * Returns a vector with all items inside a semicolon-separated list.
 */
vector<string> semicolon_list_to_vector(const string &s) {
    vector<string> parts = split(s, ";");
    for(size_t p = 0; p < parts.size(); ++p) {
        parts[p] = trim_spaces(parts[p]);
    }
    return parts;
}


/* ----------------------------------------------------------------------------
 * Shows a native message box. It is better to call this rather than
 * al_show_native_message_box() directly because it does not reset the locale
 * after it is done.
 * The parameters are the same ones you'd pass to the Allegro function.
 */
int show_message_box(
    ALLEGRO_DISPLAY* display, char const* title, char const* heading,
    char const* text, char const* buttons, int flags
) {
    int ret =
        al_show_native_message_box(
            display, title, heading, text, buttons, flags
        );
    //Reset the locale, which gets set by Allegro's native dialogs...
    //and breaks s2f().
    setlocale(LC_ALL, "C");
    
    return ret;
}


/* ----------------------------------------------------------------------------
 * Handles a system signal.
 */
void signal_handler(const int signum) {
    volatile static bool already_handling_signal = false;
    
    if(already_handling_signal) {
        //This stops an infinite loop if there's a signal raise
        //inside this function. It shouldn't happen, but better be safe.
        exit(signum);
    }
    already_handling_signal = true;
    
    string bt_str = "Backtrace:\n";
    vector<string> bt = get_backtrace();
    for(size_t s = 0; s < bt.size(); ++s) {
        bt_str += "    " + bt[s] + "\n";
    }
    if(bt_str.back() == '\n') {
        bt_str.pop_back();
    }
    string signal_name(strsignal(signum));
    string type_str = "Signal " + i2s(signum) + " (" + signal_name + ")";
    
    crash(type_str, bt_str, signum);
}


/* ----------------------------------------------------------------------------
 * Standardizes a path, making it use forward slashes instead of backslashes,
 * and removing excess slashes at the end.
 */
string standardize_path(const string &path) {
    string res = replace_all(path, "\\", "/");
    if(res.back() == '/') res.pop_back();
    return res;
}


/* ----------------------------------------------------------------------------
 * Starts the display of a text message.
 * If the text is empty, it closes the message box.
 * Any newline characters or slashes followed by n ("\n") will be used to
 * separate the message into lines.
 * text:        Text to display.
 * speaker_bmp: Bitmap representing the speaker.
 */
void start_message(string text, ALLEGRO_BITMAP* speaker_bmp) {
    text = replace_all(text, "\\n", "\n");
    if(text.size()) if(text.back() == '\n') text.pop_back();
    cur_message = text;
    cur_message_char = 0;
    cur_message_char_timer.start();
    cur_message_speaker = speaker_bmp;
    cur_message_stopping_chars.clear();
    //First character. Makes it easier.
    cur_message_stopping_chars.push_back(0);
    cur_message_section = 0;
    
    vector<string> lines = split(text, "\n");
    size_t char_count = 0;
    for(size_t l = 0; l < lines.size(); ++l) {
        //+1 because of the new line character.
        char_count += lines[l].size() + 1;
        if((l + 1) % 3 == 0) cur_message_stopping_chars.push_back(char_count);
    }
    
    if(cur_message_stopping_chars.size() > 1) {
        //Remove one because the last line doesn't have a new line character.
        //Even if it does, it's invisible.
        cur_message_stopping_chars.back()--;
    }
    cur_message_stopping_chars.push_back(cur_message.size());
}


/* ----------------------------------------------------------------------------
 * Updates the history list for the animation editor,
 * adding a new entry or bumping it up.
 */
void update_animation_editor_history(const string &n) {
    //First, check if it exists.
    size_t pos = INVALID;
    
    for(size_t h = 0; h < animation_editor_history.size(); ++h) {
        if(animation_editor_history[h] == n) {
            pos = h;
            break;
        }
    }
    
    if(pos == 0) {
        //Already #1? Never mind.
        return;
    } else if(pos == INVALID) {
        //If it doesn't exist, create it and add it to the top.
        animation_editor_history.insert(animation_editor_history.begin(), n);
    } else {
        //Otherwise, remove it from its spot and bump it to the top.
        animation_editor_history.erase(animation_editor_history.begin() + pos);
        animation_editor_history.insert(animation_editor_history.begin(), n);
    }
    
    if(animation_editor_history.size() > ANIMATION_EDITOR_HISTORY_SIZE) {
        animation_editor_history.erase(
            animation_editor_history.begin() +
            animation_editor_history.size() - 1
        );
    }
}


//Calls al_fwrite, but with an std::string instead of a c-string.
void al_fwrite(ALLEGRO_FILE* f, string s) { al_fwrite(f, s.c_str(), s.size()); }


//Converts a color to its string representation.
string c2s(const ALLEGRO_COLOR &c) {
    return i2s(c.r * 255) + " " + i2s(c.g * 255) + " " + i2s(c.b * 255) +
           (c.a == 1 ? "" : " " + i2s(c.a * 255));
}


//Converts a string to an Allegro color.
//Components are separated by spaces, and the final one (alpha) is optional.
ALLEGRO_COLOR s2c(const string &s) {
    string s2 = s;
    s2 = trim_spaces(s2);
    
    unsigned char alpha = 255;
    vector<string> components = split(s2);
    if(components.size() >= 2) alpha = s2i(components[1]);
    
    if(s2 == "nothing") return al_map_rgba(0,   0,   0,   0);
    if(s2 == "none")    return al_map_rgba(0,   0,   0,   0);
    if(s2 == "black")   return al_map_rgba(0,   0,   0,   alpha);
    if(s2 == "gray")    return al_map_rgba(128, 128, 128, alpha);
    if(s2 == "grey")    return al_map_rgba(128, 128, 128, alpha);
    if(s2 == "white")   return map_alpha(alpha);
    if(s2 == "yellow")  return al_map_rgba(255, 255, 0,   alpha);
    if(s2 == "orange")  return al_map_rgba(255, 128, 0,   alpha);
    if(s2 == "brown")   return al_map_rgba(128, 64,  0,   alpha);
    if(s2 == "red")     return al_map_rgba(255, 0,   0,   alpha);
    if(s2 == "violet")  return al_map_rgba(255, 0,   255, alpha);
    if(s2 == "purple")  return al_map_rgba(128, 0,   255, alpha);
    if(s2 == "blue")    return al_map_rgba(0,   0,   255, alpha);
    if(s2 == "cyan")    return al_map_rgba(0,   255, 255, alpha);
    if(s2 == "green")   return al_map_rgba(0,   255, 0,   alpha);
    
    ALLEGRO_COLOR c =
        al_map_rgba(
            ((components.size() > 0) ? s2i(components[0]) : 0),
            ((components.size() > 1) ? s2i(components[1]) : 0),
            ((components.size() > 2) ? s2i(components[2]) : 0),
            ((components.size() > 3) ? s2i(components[3]) : 255)
        );
    return c;
}


#if defined(_WIN32)

string strsignal(const int signum) {
    if(signum == SIGINT) {
        return "SIGINT";
    } else if(signum == SIGILL) {
        return "SIGILL";
    } else if(signum == SIGFPE) {
        return "SIGFPE";
    } else if(signum == SIGSEGV) {
        return "SIGSEGV";
    } else if(signum == SIGTERM) {
        return "SIGTERM";
    } else if(signum == SIGBREAK) {
        return "SIGBREAK";
    } else if(signum == SIGABRT) {
        return "SIGABRT";
    } else if(signum == SIGABRT_COMPAT) {
        return "SIGABRT_COMPAT";
    } else {
        return "Unknown";
    }
}

#endif //if defined(_WIN32)
