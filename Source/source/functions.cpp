/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
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
#include <signal.h>
#include <sstream>
#include <stdlib.h>
#include <typeinfo>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "functions.h"

#include "const.h"
#include "drawing.h"
#include "editors/animation_editor/editor.h"
#include "game.h"
#include "init.h"
#include "menus.h"
#include "utils/backtrace.h"
#include "utils/string_utils.h"


//Calls al_fwrite, but with an std::string instead of a c-string.
void al_fwrite(ALLEGRO_FILE* f, string s) { al_fwrite(f, s.c_str(), s.size()); }


//Converts a color to its string representation.
string c2s(const ALLEGRO_COLOR &c) {
    return i2s(c.r * 255) + " " + i2s(c.g * 255) + " " + i2s(c.b * 255) +
           (c.a == 1 ? "" : " " + i2s(c.a * 255));
}


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
 * Clears the textures of the area's sectors from memory.
 */
void clear_area_textures() {
    for(size_t s = 0; s < game.cur_area_data.sectors.size(); ++s) {
        sector* s_ptr = game.cur_area_data.sectors[s];
        if(
            s_ptr->texture_info.bitmap &&
            s_ptr->texture_info.bitmap != game.bmp_error
        ) {
            game.textures.detach(s_ptr->texture_info.file_name);
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

    if(game.display) {
        ALLEGRO_BITMAP* backbuffer = al_get_backbuffer(game.display);
        if(backbuffer) {
            al_save_bitmap(
                (
                    USER_DATA_FOLDER_PATH + "/" +
                    "Crash_" + get_current_time(true) + ".png"
                ).c_str(),
                backbuffer
            );
        }
    }
    
    string error_str = "Program crash!\n";
    error_str +=
        "  Reason: " + reason + ".\n"
        "  Info: " + info + "\n"
        "  Time: " + get_current_time(false) + ".\n";
    if(game.errors_reported_so_far > 0) {
        error_str += "  Error log has messages!\n";
    }
    error_str +=
        "  Game state: " + game.get_cur_state_name() + ". delta_t: " +
        (
            game.delta_t == 0.0f ? "0" :
            f2s(game.delta_t) + " (" + f2s(1 / game.delta_t) + " FPS)"
        ) + ".\n"
        "  Mob count: " + i2s(game.states.gameplay_st->mobs.all.size()) + ". Particle count: " +
        i2s(game.states.gameplay_st->particles.get_count()) + ".\n" +
        "  Bitmaps loaded: " + i2s(game.bitmaps.get_list_size()) + " (" +
        i2s(game.bitmaps.get_total_calls()) + " total calls).\n" +
        "  Current area: ";
        
    if(!game.cur_area_data.name.empty()) {
        error_str +=
            game.cur_area_data.name + ", version " +
            game.cur_area_data.version + ".\n";
    } else {
        error_str += "none.\n";
    }
    
    error_str += "  Current leader: ";
    
    if(game.states.gameplay_st->cur_leader_ptr) {
        error_str +=
            game.states.gameplay_st->cur_leader_ptr->type->name + ", at " +
            p2s(game.states.gameplay_st->cur_leader_ptr->pos) +
            ", state history: " +
            game.states.gameplay_st->cur_leader_ptr->fsm.cur_state->name;
        for(size_t h = 0; h < STATE_HISTORY_SIZE; ++h) {
            error_str +=
                " " +
                game.states.gameplay_st->cur_leader_ptr->fsm.prev_state_names[h];
        }
        error_str += "\n  10 closest Pikmin to that leader:\n";
        
        vector<pikmin*> closest_pikmin = game.states.gameplay_st->mobs.pikmin_list;
        sort(
            closest_pikmin.begin(), closest_pikmin.end(),
        [] (pikmin * p1, pikmin * p2) -> bool {
            return
            dist(game.states.gameplay_st->cur_leader_ptr->pos, p1->pos).to_float() <
            dist(game.states.gameplay_st->cur_leader_ptr->pos, p2->pos).to_float();
        }
        );
        
        size_t closest_p_amount = std::min(closest_pikmin.size(), (size_t) 10);
        for(size_t p = 0; p < closest_p_amount; ++p) {
            error_str +=
                "    " + closest_pikmin[p]->type->name + ", at " +
                p2s(closest_pikmin[p]->pos) + ", history: " +
                closest_pikmin[p]->fsm.cur_state->name;
            for(size_t h = 0; h < STATE_HISTORY_SIZE; ++h) {
                error_str += " " + closest_pikmin[p]->fsm.prev_state_names[h];
            }
            error_str += "\n";
        }
    } else {
        error_str += "none.";
    }
    
    log_error(error_str);
    
    show_message_box(
        NULL, "Program crash!",
        "Pikifen has crashed!",
        "Sorry about that! To help fix this problem, please read the "
        "FAQ & troubleshooting section of the readme file. Thanks!",
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
    size_t n_points =
        game.cur_area_data.weather_condition.blackout_strength.size();
        
    if(n_points == 0) {
        return 0;
    } else if(n_points == 1) {
        return game.cur_area_data.weather_condition.blackout_strength[0].second;
    }
    
    for(size_t p = 0; p < n_points - 1; ++p) {
        auto cur_ptr =
            &game.cur_area_data.weather_condition.blackout_strength[p];
        auto next_ptr =
            &game.cur_area_data.weather_condition.blackout_strength[p + 1];
            
        if(
            game.states.gameplay_st->day_minutes >= cur_ptr->first &&
            game.states.gameplay_st->day_minutes < next_ptr->first
        ) {
        
            return
                interpolate_number(
                    game.states.gameplay_st->day_minutes,
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
    
    for(size_t m = 0; m < game.states.gameplay_st->mobs.all.size(); ++m) {
        mob* m_ptr = game.states.gameplay_st->mobs.all[m];
        
        if(!m_ptr->fsm.cur_state) continue;
        
        dist d = dist(game.mouse_cursor_w, m_ptr->pos);
        if(!closest_mob_to_cursor || d < closest_mob_to_cursor_dist) {
            closest_mob_to_cursor = m_ptr;
            closest_mob_to_cursor_dist = d;
        }
    }
    
    return closest_mob_to_cursor;
}


/* ----------------------------------------------------------------------------
 * Returns a string representing the current date and time.
 * filename_friendly: If true, slashes become dashes,
 *   and semicolons become dots.
 */
string get_current_time(const bool filename_friendly) {
    time_t tt;
    time(&tt);
    struct tm t = *localtime(&tt);
    return
        i2s(t.tm_year + 1900) +
        (filename_friendly ? "-" : "/") +
        leading_zero(t.tm_mon + 1) +
        (filename_friendly ? "-" : "/") +
        leading_zero(t.tm_mday) +
        (filename_friendly ? "_" : " ") +
        leading_zero(t.tm_hour) +
        (filename_friendly ? "." : ":") +
        leading_zero(t.tm_min) +
        (filename_friendly ? "." : ":") +
        leading_zero(t.tm_sec);
}


/* ----------------------------------------------------------------------------
 * Returns the daylight effect color for the current time and weather.
 */
ALLEGRO_COLOR get_daylight_color() {
    size_t n_points = game.cur_area_data.weather_condition.daylight.size();
    
    if(n_points == 0) {
        return al_map_rgba(255, 255, 255, 0);
    } else if(n_points == 1) {
        return game.cur_area_data.weather_condition.daylight[0].second;
    }
    
    for(size_t p = 0; p < n_points - 1; ++p) {
        auto cur_ptr = &game.cur_area_data.weather_condition.daylight[p];
        auto next_ptr = &game.cur_area_data.weather_condition.daylight[p + 1];
        
        if(
            game.states.gameplay_st->day_minutes >= cur_ptr->first &&
            game.states.gameplay_st->day_minutes < next_ptr->first
        ) {
        
            return
                interpolate_color(
                    game.states.gameplay_st->day_minutes,
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
    size_t n_points = game.cur_area_data.weather_condition.fog_color.size();
    
    if(n_points == 0) {
        return al_map_rgba(255, 255, 255, 0);
    } else if(n_points == 1) {
        return game.cur_area_data.weather_condition.fog_color[0].second;
    }
    
    for(size_t p = 0; p < n_points - 1; ++p) {
        auto cur_ptr = &game.cur_area_data.weather_condition.fog_color[p];
        auto next_ptr = &game.cur_area_data.weather_condition.fog_color[p + 1];
        
        if(
            game.states.gameplay_st->day_minutes >= cur_ptr->first &&
            game.states.gameplay_st->day_minutes < next_ptr->first
        ) {
        
            return
                interpolate_color(
                    game.states.gameplay_st->day_minutes,
                    cur_ptr->first, next_ptr->first,
                    cur_ptr->second, next_ptr->second
                );
        }
    }
    
    //If anything goes wrong, return a failsafe.
    return al_map_rgba(255, 255, 255, 0);
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
    
    if(ret_h) *ret_h = std::max(0, (int) ((fh + 1) * n_lines) - 1);
    
    if(ret_w) {
        int largest_w = 0;
        for(size_t l = 0; l < lines.size(); ++l) {
            largest_w =
                std::max(
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
    size_t n_points = game.cur_area_data.weather_condition.sun_strength.size();
    
    if(n_points == 0) {
        return 1.0f;
    } else if(n_points == 1) {
        return game.cur_area_data.weather_condition.sun_strength[0].second;
    }
    
    for(size_t p = 0; p < n_points - 1; ++p) {
        auto cur_ptr =
            &game.cur_area_data.weather_condition.sun_strength[p];
        auto next_ptr =
            &game.cur_area_data.weather_condition.sun_strength[p + 1];
            
        if(
            game.states.gameplay_st->day_minutes >= cur_ptr->first &&
            game.states.gameplay_st->day_minutes < next_ptr->first
        ) {
        
            return
                interpolate_number(
                    game.states.gameplay_st->day_minutes,
                    cur_ptr->first, next_ptr->first,
                    cur_ptr->second, next_ptr->second
                ) / 255.0f;
        }
    }
    
    //If anything goes wrong, return a failsafe.
    return 1.0f;
}

/* ----------------------------------------------------------------------------
 * Given a string representation of mob script variables,
 * returns a map, where every key is a variable, and every value is the
 * variable's value.
 */
map<string, string> get_var_map(const string &vars_string) {
    map<string, string> final_map;
    vector<string> raw_vars = semicolon_list_to_vector(vars_string);
    
    for(size_t v = 0; v < raw_vars.size(); ++v) {
        vector<string> raw_parts = split(raw_vars[v], "=");
        if(raw_parts.size() < 2) {
            continue;
        }
        final_map[trim_spaces(raw_parts[0])] = trim_spaces(raw_parts[1]);
    }
    return final_map;
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
vector<std::pair<size_t, string> > get_weather_table(data_node* node) {
    vector<std::pair<size_t, string> > table;
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
            std::pair<size_t, string> p1,
            std::pair<size_t, string> p2
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
    
    std::cout << s;
    
    if(game.errors_reported_so_far == 0) {
        s =
            "\n" +
            get_current_time(false) +
            "; Pikifen version " +
            i2s(VERSION_MAJOR) + "." + i2s(VERSION_MINOR) +
            "." + i2s(VERSION_REV) + ", game version " +
            game.config.version + "\n" + s;
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
    
    game.errors_reported_so_far++;
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
 * text:           Text to print. Can use line breaks.
 * total_duration: Total amount of time in which the text is present.
 * fade_duration:  When closing, fade out in the last N seconds.
 */
void print_info(
    const string &text, const float total_duration, const float fade_duration
) {
    game.creator_tools.info_print_text = text;
    game.creator_tools.info_print_duration = total_duration;
    game.creator_tools.info_print_fade_duration = fade_duration;
    game.creator_tools.info_print_timer.start(total_duration);
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
    al_show_native_file_dialog(game.display, dialog);
    
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
 * The result pointer returns FILE_DIALOG_RES_SUCCESS on success,
 * FILE_DIALOG_RES_WRONG_FOLDER if the one or more choices do not belong to
 * the specified folder, and FILE_DIALOG_RES_CANCELED if the user canceled.
 * The folder argument is the folder to lock to, without the ending slash.
 * The other arguments are the same you'd pass to prompt_file_dialog().
 * The list of choices that are returned only have the file name, not the
 * rest of the path. Choices can also be contained inside subfolders of the
 * specified folder.
 */
vector<string> prompt_file_dialog_locked_to_folder(
    const string &folder, const string &title,
    const string &patterns, const int mode, FILE_DIALOG_RESULTS* result
) {
    vector<string> f =
        prompt_file_dialog(folder + "/", title, patterns, mode);
        
    if(f.empty() || f[0].empty()) {
        *result = FILE_DIALOG_RES_CANCELED;
        return vector<string>();
    }
    
    for(size_t fi = 0; fi < f.size(); ++fi) {
        size_t folder_pos = f[0].find(folder);
        if(folder_pos == string::npos) {
            //This isn't in the specified folder!
            *result = FILE_DIALOG_RES_WRONG_FOLDER;
            return vector<string>();
        } else {
            f[fi] =
                f[fi].substr(folder_pos + folder.size() + 1, string::npos);
        }
    }
    
    *result = FILE_DIALOG_RES_SUCCESS;
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
        new data_node("enabled", b2s(game.creator_tools.enabled))
    );
    
    for(unsigned char k = 0; k < 20; k++) {
        string tool_key;
        if(k < 10) {
            //The first ten indexes are the F2 - F11 keys.
            tool_key = "f" + i2s(k + 2);
        } else {
            //The second ten indexes are the 0 - 9 keys.
            tool_key = i2s(k - 10);
        }
        string tool_name = CREATOR_TOOL_NAMES[game.creator_tools.keys[k]];
        
        file.add(new data_node(tool_key, tool_name));
    }
    
    file.add(
        new data_node(
            "area_image_mobs", b2s(game.creator_tools.area_image_mobs)
        )
    );
    file.add(
        new data_node(
            "area_image_shadows", b2s(game.creator_tools.area_image_shadows)
        )
    );
    file.add(
        new data_node(
            "area_image_size", i2s(game.creator_tools.area_image_size)
        )
    );
    file.add(
        new data_node(
            "change_speed_multiplier", f2s(game.creator_tools.change_speed_mult)
        )
    );
    file.add(
        new data_node(
            "mob_hurting_percentage", f2s(game.creator_tools.mob_hurting_ratio * 100)
        )
    );
    
    file.add(
        new data_node(
            "auto_start_option", game.creator_tools.auto_start_option
        )
    );
    file.add(
        new data_node(
            "auto_start_mode", game.creator_tools.auto_start_mode
        )
    );
    
    file.save_file(CREATOR_TOOLS_FILE_PATH, true, true);
}


/* ----------------------------------------------------------------------------
 * Saves the player's options.
 */
void save_options() {
    data_node file("", "");
    
    //Save the standard options.
    game.options.save(&file);
    
    //Also add the animation editor history.
    for(
        size_t h = 0; h < game.states.animation_editor_st->history.size(); ++h
    ) {
        file.add(
            new data_node(
                "animation_editor_history_" + i2s(h + 1),
                game.states.animation_editor_st->history[h]
            )
        );
    }
    
    //Finally, save.
    file.save_file(OPTIONS_FILE_PATH, true, true);
}


/* ----------------------------------------------------------------------------
 * Saves the current backbuffer onto a file.
 * In other words, dumps a screenshot.
 */
void save_screenshot() {
    string base_file_name = "Screenshot_" + get_current_time(true);
    
    //Check if a file with this name already exists.
    vector<string> files = folder_to_vector(USER_DATA_FOLDER_PATH, false);
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
        (USER_DATA_FOLDER_PATH + "/" + final_file_name + ".png").c_str(),
        al_get_backbuffer(game.display)
    );
}


/* ----------------------------------------------------------------------------
 * Returns a vector with all items inside a semicolon-separated list.
 * s:   The string containing the list.
 * sep: Separator to use, in case you need something else. Default is semicolon.
 */
vector<string> semicolon_list_to_vector(const string &s, const string &sep) {
    vector<string> parts = split(s, sep);
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
 * Spews out a Pikmin from a given point. Used by Onions and converters.
 */
void spew_pikmin_seed(
    const point pos, const float z, pikmin_type* pik_type,
    const float angle, const float horizontal_speed, const float vertical_speed
) {
    pikmin* new_pikmin =
        (
            (pikmin*)
            create_mob(
                game.mob_categories.get(MOB_CATEGORY_PIKMIN),
                pos, pik_type, angle, ""
            )
        );
    new_pikmin->z = z;
    new_pikmin->speed.x = cos(angle) * horizontal_speed;
    new_pikmin->speed.y = sin(angle) * horizontal_speed;
    new_pikmin->speed_z = vertical_speed;
    new_pikmin->fsm.set_state(PIKMIN_STATE_SEED);
    new_pikmin->maturity = 0;
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
    if(!text.empty()) {
        game.states.gameplay_st->msg_box = new msg_box_info(text, speaker_bmp);
    } else {
        delete game.states.gameplay_st->msg_box;
        game.states.gameplay_st->msg_box = NULL;
    }
}


#if defined(_WIN32)

string strsignal(const int signum) {
    switch(signum) {
    case SIGINT: {
        return "SIGINT";
        break;
    } case SIGILL: {
        return "SIGILL";
        break;
    } case SIGFPE: {
        return "SIGFPE";
        break;
    } case SIGSEGV: {
        return "SIGSEGV";
        break;
    } case SIGTERM: {
        return "SIGTERM";
        break;
    } case SIGBREAK: {
        return "SIGBREAK";
        break;
    } case SIGABRT: {
        return "SIGABRT";
        break;
    } case SIGABRT_COMPAT: {
        return "SIGABRT_COMPAT";
        break;
    } default: {
        return "Unknown";
        break;
    }
    }
}

#endif //if defined(_WIN32)


/* ----------------------------------------------------------------------------
 * Returns a string that's a join of the strings in the specified vector,
 * but only past a certain position. The strings are joined with a space
 * character.
 * v:   The vector of strings.
 * pos: Use the string at this position and onward.
 */
string vector_tail_to_string(const vector<string> &v, const size_t pos) {
    string result = v[pos];
    for(size_t p = pos + 1; p < v.size(); ++p) {
        result += " " + v[p];
    }
    return result;
}
