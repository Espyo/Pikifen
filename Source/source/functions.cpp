/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
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
#include <iomanip>
#include <iostream>
#include <math.h>
#include <sstream>
#include <stdlib.h>
#include <typeinfo>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "backtrace.h"
#include "editors/animation_editor.h"
#include "const.h"
#include "data_file.h"
#include "drawing.h"
#include "functions.h"
#include "init.h"
#include "menus.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Boxes a string so that it becomes a specific size.
 * Truncates if it's too big, pads with spaces if it's too small.
 */
string box_string(const string &s, const size_t size) {
    string spaces = string(size, ' ');
    return (s + spaces).substr(0, size);
}


/* ----------------------------------------------------------------------------
 * Does sector s1 cast a shadow onto sector s2?
 */
bool casts_shadow(sector* s1, sector* s2) {
    if(!s1 || !s2) return false;
    if(s1->type == SECTOR_TYPE_BOTTOMLESS_PIT) return false;
    if(s2->type == SECTOR_TYPE_BOTTOMLESS_PIT) return false;
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
 * Limits the given number to the given range, inclusive.
 */
float clamp(const float number, const float minimum, const float maximum) {
    return min(maximum, max(minimum, number));
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
 * Returns the angle and magnitude of vector coordinates.
 * coordinates: The coordinates.
 * angle:       Variable to return the angle to.
 * magnitude:   Variable to return the magnitude to.
 */
void coordinates_to_angle(
    const point &coordinates, float* angle, float* magnitude
) {
    if(angle) {
        *angle = atan2(coordinates.y, coordinates.x);
    }
    if(magnitude) {
        *magnitude = dist(point(0, 0), coordinates).to_float();
    }
}


/* ----------------------------------------------------------------------------
 * Returns a random number, between 0 and 1, but it's deterministic
 * if you use the same seed. i.e., if you feed it X, it
 * will always return Y. Because of its simplicity and predictability,
 * it should only be used for tiny details with unimportant randomness.
 * seed: The seed number.
 */
float deterministic_random(const unsigned int seed) {
    //This was built pretty much ad-hoc.
    return
        (
            ((seed * 1234567890L + (seed << 4)) % (seed ^ 981524)) % 65536
        ) / 65535.0f;
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
    folder_name = replace_all(folder_name, "\\", "/");
    if(folder_name.back() == '/') folder_name.pop_back();
    
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
        
            string entry_name = al_get_fs_entry_name(entry);
            if(folders) {
                //If we're using folders, remove the trailing slash,
                //lest the string be fully deleted.
                if(
                    entry_name[entry_name.size() - 1] == '/' ||
                    entry_name[entry_name.size() - 1] == '\\'
                ) {
                    entry_name.erase(entry_name.size() - 1);
                }
            }
            
            //Only save what's after the final slash.
            entry_name = replace_all(entry_name, "\\", "/");
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
 * Returns the interpolation of the value between two positions.
 */
float interpolate_number(
    const float p, const float p1, const float p2,
    const float v1, const float v2
) {
    return v1 + ((p - p1) / (float) (p2 - p1)) * (v2 - v1);
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
    
    if(no_error_logs_today) {
        no_error_logs_today = false;
        s =
            "\n" +
            get_current_time(true) +
            "; Pikmin fangame engine version " +
            i2s(VERSION_MAJOR) + "." + i2s(VERSION_MINOR) +
            "." + i2s(VERSION_REV) + "\n" + s;
    }
    
    string prev_error_log;
    string line;
    ALLEGRO_FILE* file_i = al_fopen("Error_log.txt", "r");
    if(file_i) {
        while(!al_feof(file_i)) {
            getline(file_i, line);
            prev_error_log += line + "\n";
        }
        prev_error_log.erase(prev_error_log.size() - 1);
        al_fclose(file_i);
    }
    
    ALLEGRO_FILE* file_o = al_fopen("Error_log.txt", "w");
    if(file_o) {
        al_fwrite(file_o, prev_error_log + s);
        al_fclose(file_o);
    }
}


/* ----------------------------------------------------------------------------
 * Prints a bit of info onto the screen, for some seconds.
 */
void print_info(const string &text) {
    info_print_text = text;
    info_print_timer.start();
}


/* ----------------------------------------------------------------------------
 * Returns a random float between the provided range, inclusive.
 */
float randomf(float min, float max) {
    if(min == max) return min;
    if(min > max) swap(min, max);
    return (float) rand() / ((float) RAND_MAX / (max - min)) + min;
}


/* ----------------------------------------------------------------------------
 * Returns a random integer between the provided range, inclusive.
 */
int randomi(int min, int max) {
    if(min == max) return min;
    if(min > max) swap(min, max);
    return ((rand()) % (max - min + 1)) + min;
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
 * Replaces all instances of x with y.
 */
string replace_all(string s, string search, string replacement) {
    size_t pos = s.find(search);
    while(pos != string::npos) {
        s.replace(pos, search.size(), replacement);
        pos = s.find(search, pos + replacement.size());
    };
    
    return s;
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
            "draw_cursor_trail", b2s(draw_cursor_trail)
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
            "fps", i2s(game_fps)
        )
    );
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
    file.add(new data_node("resolution", i2s(scr_w) + " " + i2s(scr_h)));
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
    
    file.save_file("Options.txt");
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
    
    al_save_bitmap(
        ("Crash " + get_current_time(false) + ".png").c_str(),
        al_get_backbuffer(display)
    );
    
    string error_str =
        "Program crash!\n"
        "  Time: " + get_current_time(true) + ". "
        "Signal: " + i2s(signum) + ". Backtrace:\n";
    vector<string> bt = get_backtrace();
    for(size_t s = 0; s < bt.size(); ++s) {
        error_str += "    " + bt[s] + "\n";
    }
    error_str +=
        "  Game state number: " + i2s(cur_game_state_nr) + "\n"
        "  Number of mobs: " + i2s(mobs.size()) + "\n"
        "  Bitmaps loaded: " + i2s(bitmaps.get_list_size()) + " (" +
        i2s(bitmaps.get_total_calls()) + " total calls)" + "\n"
        "  Delta_t: " + f2s(delta_t) + " (" +
        f2s(1 / delta_t) + " FPS)";
        
    log_error(error_str);
    
    al_show_native_message_box(
        NULL, "Program crash!",
        "The Pikmin fangame engine has crashed!",
        "Sorry about that! Please read the readme file to know what you "
        "can do to help me fix it. Thanks!",
        NULL,
        ALLEGRO_MESSAGEBOX_ERROR
    );
    
    exit(signum);
}


/* ----------------------------------------------------------------------------
 * Splits a string into several substrings, by the specified delimiter.
 * text:        The string to split.
 * del:         The delimiter. Default is space.
 * inc_empty:   If true, include empty substrings on the vector.
 *   i.e. if two delimiters come together in a row,
 *   keep an empty substring between.
 * inc_del:     If true, include the delimiters on the vector as a substring.
 */
vector<string> split(
    string text, const string &del, const bool inc_empty, const bool inc_del
) {
    vector<string> v;
    size_t pos;
    size_t del_size = del.size();
    
    do {
        pos = text.find(del);
        if (pos != string::npos) {  //If it DID find the delimiter.
            //Get the text between the start and the delimiter.
            string sub = text.substr(0, pos);
            
            //Add the text before the delimiter to the vector.
            if(sub != "" || inc_empty)
                v.push_back(sub);
                
            //Add the delimiter to the vector, but only if requested.
            if(inc_del)
                v.push_back(del);
                
            //Delete everything before the delimiter,
            //including the delimiter itself, and search again.
            text.erase(text.begin(), text.begin() + pos + del_size);
        }
    } while (pos != string::npos);
    
    //Text after the final delimiter.
    //(If there is one. If not, it's just the whole string.)
    
    //If it's a blank string,
    //only add it if we want empty strings.
    if (text != "" || inc_empty) {
        v.push_back(text);
    }
    
    return v;
}


/* ----------------------------------------------------------------------------
 * Starts the display of a text message.
 * If the text is empty, it closes the message box.
 * text:        Text to display.
 * speaker_bmp: Bitmap representing the speaker.
 */
void start_message(string text, ALLEGRO_BITMAP* speaker_bmp) {
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
 * Converts an entire string into lowercase.
 */
string str_to_lower(string s) {
    unsigned short n_characters = s.size();
    for(unsigned short c = 0; c < n_characters; ++c) {
        s[c] = tolower(s[c]);
    }
    return s;
}


/* ----------------------------------------------------------------------------
 * Converts an entire string into uppercase.
 */
string str_to_upper(string s) {
    unsigned short n_characters = s.size();
    for(unsigned short c = 0; c < n_characters; ++c) {
        s[c] = toupper(s[c]);
    }
    return s;
}


/* ----------------------------------------------------------------------------
 * Sums a number to another (even if negative), and then
 * wraps that number across a limit, applying a modulus operation.
 * nr:         Base number.
 * sum:        Number to add (or subtract).
 * wrap_limit: Wrap between [0 - wrap_limit[.
 */
int sum_and_wrap(const int nr, const int sum, const int wrap_limit) {
    int final_nr = nr + sum;
    while(final_nr < 0) {
        final_nr += wrap_limit;
    }
    return final_nr % wrap_limit;
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


//Converts a boolean to a string, returning either "true" or "false".
string b2s(const bool b) { return b ? "true" : "false"; }


//Converts a color to its string representation.
string c2s(const ALLEGRO_COLOR &c) {
    return i2s(c.r * 255) + " " + i2s(c.g * 255) + " " + i2s(c.b * 255) +
           (c.a == 1 ? "" : " " + i2s(c.a * 255));
}


//Converts a float to a string, with 2 decimal places.
string f2s(const float f) {
    std::stringstream s;
    s << std::fixed << ::setprecision(2) << f;
    return s.str();
}


//Converts a point to a string.
string p2s(const point &p) {
    return f2s(p.x) + " " + f2s(p.y);
}


//Converts a string to a boolean, judging by
//the English language words that represent true and false.
bool s2b(const string &s) {
    string s2 = s;
    s2 = str_to_lower(s2);
    s2 = trim_spaces(s2);
    if(s2 == "yes" || s2 == "true" || s2 == "y" || s2 == "t") return true;
    else return (s2i(s2) != 0);
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


//Converts a string to a float,
//trimming the spaces and accepting commas or points.
double s2f(const string &s) {
    string s2 = trim_spaces(s);
    replace(s2.begin(), s2.end(), ',', '.');
    return atof(s2.c_str());
}


//Converts a string to an integer.
int s2i(const string &s) { return s2f(s); }


//Converts a string to a point.
point s2p(const string &s) {
    vector<string> words = split(s);
    point p;
    if(words.size() >= 1) {
        p.x = s2f(words[0]);
    }
    if(words.size() >= 2) {
        p.y = s2f(words[1]);
    }
    return p;
}
