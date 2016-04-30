/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Globally accessible functions.
 */

#define _USE_MATH_DEFINES

#pragma warning(disable : 4996) //Disables warning about localtime being deprecated.

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <math.h>
#include <sstream>
#include <stdlib.h>
#include <typeinfo>

#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>

#include "animation_editor.h"
#include "area_editor.h"
#include "const.h"
#include "data_file.h"
#include "drawing.h"
#include "functions.h"
#include "menus.h"
#include "vars.h"

/* ----------------------------------------------------------------------------
 * Returns the vector coordinates of an angle.
 * angle:    The angle.
 * magnitue: Its magnitude.
 * *_coord:  Variables to return the coordinates to.
 */
void angle_to_coordinates(const float angle, const float magnitude, float* x_coord, float* y_coord) {
    *x_coord = cos(angle) * magnitude;
    *y_coord = sin(angle) * magnitude;
}


/* ----------------------------------------------------------------------------
 * Checks if two points are colliding via a bounding-box check.
 * c*1: Coordinates of the first point.
 * c*2: Coordinates of the second point.
 * r:   Range of the bounding box.
 */
bool bbox_check(const float cx1, const float cy1, const float cx2, const float cy2, const float r) {
    return (fabs(cx1 - cx2) <= r && fabs(cy1 - cy2) <= r);
}


/* ----------------------------------------------------------------------------
 * Boxes a string so that it becomes a specific size.
 * Truncates if it's too big, pads with spaces if it's too small.
 */
string box_string(const string &s, const size_t size) {
    string spaces = string(size, ' ');
    return (s + spaces).substr(0, size);
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
    c2.r = c.r + l;
    c2.r = min(1.0f, c2.r);
    c2.r = max(0.0f, c2.r);
    c2.g = c.g + l;
    c2.g = min(1.0f, c2.g);
    c2.g = max(0.0f, c2.g);
    c2.b = c.b + l;
    c2.b = min(1.0f, c2.b);
    c2.b = max(0.0f, c2.b);
    c2.a = c.a;
    return c2;
}


/* ----------------------------------------------------------------------------
 * Changes the game's state.
 */
void change_game_state(unsigned int new_state) {
    game_states[cur_game_state_nr]->unload();
    cur_game_state_nr = new_state;
    game_states[cur_game_state_nr]->load();
    
    //Because during the loading screens, there is no activity, on the
    //next frame, the game will assume the time between that and the last
    //non-loading frame is normal. This could be something like 2 seconds.
    //Let's reset the delta_t, then.
    reset_delta_t = true;
    
}


/* ----------------------------------------------------------------------------
 * Returns whether a circle is touching a line segment or not.
 * cx, cy: Coordinates of the circle.
 * cr:     Radius of the circle.
 * x*, y*: Coordinates of the line.
 * li*:    If not NULL, the line intersection coordinates are returned here.
 */
bool circle_intersects_line(const float cx, const float cy, const float cr, const float x1, const float y1, const float x2, const float y2, float* lix, float* liy) {

    //Code by http://www.melloland.com/scripts-and-tutos/collision-detection-between-circles-and-lines
    
    float vx = x2 - x1;
    float vy = y2 - y1;
    float xdiff = x1 - cx;
    float ydiff = y1 - cy;
    float a = vx * vx + vy * vy;
    float b = 2 * ((vx * xdiff) + (vy * ydiff));
    float c = xdiff * xdiff + ydiff * ydiff - cr * cr;
    float quad = b * b - (4 * a * c);
    if (quad >= 0) {
        //An infinite collision is happening, but let's not stop here
        float quadsqrt = sqrt(quad);
        for (int i = -1; i <= 1; i += 2) {
            //Returns the two coordinates of the intersection points
            float t = (i * -b + quadsqrt) / (2 * a);
            float x = x1 + (i * vx * t);
            float y = y1 + (i * vy * t);
            //If one of them is in the boundaries of the segment, it collides
            if (x >= min(x1, x2) && x <= max(x1, x2) && y >= min(y1, y2) && y <= max(y1, y2)) {
                if(lix) *lix = x;
                if(liy) *liy = y;
                return true;
            }
        }
    }
    return false;
}


/* ----------------------------------------------------------------------------
 * Clears the textures of the area's sectors from memory.
 */
void clear_area_textures() {
    for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
        sector* s_ptr = cur_area_data.sectors[s];
        if(s_ptr->texture_info.bitmap && s_ptr->texture_info.bitmap != bmp_error) {
            bitmaps.detach("Textures/" + s_ptr->texture_info.file_name);
            s_ptr->texture_info.bitmap = NULL;
        }
    }
}


/* ----------------------------------------------------------------------------
 * Returns the angle and magnitude of vector coordinates.
 * *_coord:   The coordinates.
 * angle:     Variable to return the angle to.
 * magnitude: Variable to return the magnitude to.
 */
void coordinates_to_angle(const float x_coord, const float y_coord, float* angle, float* magnitude) {
    *angle = atan2(y_coord, x_coord);
    *magnitude = dist(0, 0, x_coord, y_coord).to_float();
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
    return (((seed * 1234567890L + (seed << 4)) % (seed ^ 981524)) % 65536) / 65535.0f;
}


/* ----------------------------------------------------------------------------
 * Prints something onto the error log.
 * s: String that represents the error.
 * d: If not null, this will be used to obtain the file name and line that caused the error.
 */
void error_log(string s, data_node* d) {
    if(d) {
        s += " (" + d->file_name;
        if(d->line_nr != 0) s += " line " + i2s(d->line_nr);
        s += ")";
    }
    s += "\n";
    
    cout << s;
    
    if(no_error_logs_today) {
        no_error_logs_today = false;
        time_t tt;
        time(&tt);
        struct tm t = *localtime(&tt);
        s =
            "\n" +
            i2s(t.tm_year + 1900) + "/" +
            leading_zero(t.tm_mon + 1) + "/" +
            leading_zero(t.tm_mday) + " " +
            leading_zero(t.tm_hour) + ":" +
            leading_zero(t.tm_min) + ":" +
            leading_zero(t.tm_sec) + "; Pikmin fangame engine version " +
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
 * Returns whether or not the string s is inside the vector of strings v.
 */
bool find_in_vector(const vector<string> &v, const string &s) {
    size_t v_size = v.size();
    for(size_t i = 0; i < v_size; ++i) if(v[i] == s) return true;
    return false;
}


/* ----------------------------------------------------------------------------
 * Stores the names of all files in a folder into a vector.
 * folder_name: Name of the folder.
 * folders:     If true, only read folders. If false, only read files.
 */
vector<string> folder_to_vector(string folder_name, const bool folders, bool* folder_found) {
    vector<string> v;
    
    if(folder_name.empty()) {
        if(folder_found) *folder_found = false;
        return v;
    }
    
    //Normalize the folder's path.
    folder_name = replace_all(folder_name, "\\", "/");
    if(folder_name.back() == '/') folder_name.pop_back();
    
    ALLEGRO_FS_ENTRY* folder = NULL;
    folder = al_create_fs_entry(folder_name.c_str());
    if(!folder || !al_open_directory(folder)) {
        if(folder_found) *folder_found = false;
        return v;
    }
    
    
    ALLEGRO_FS_ENTRY* entry = NULL;
    while((entry = al_read_directory(folder)) != NULL) {
        if(
            (folders && (al_get_fs_entry_mode(entry) & ALLEGRO_FILEMODE_ISDIR)) ||
            (!folders && !(al_get_fs_entry_mode(entry) & ALLEGRO_FILEMODE_ISDIR))) {
            
            string entry_name = al_get_fs_entry_name(entry);
            if(folders) {   //If we're using folders, remove the trailing slash, lest the string be fully deleted.
                if(entry_name[entry_name.size() - 1] == '/' || entry_name[entry_name.size() - 1] == '\\') {
                    entry_name.erase(entry_name.size() - 1);
                }
            }
            
            //Only save what's after the final slash.
            entry_name = replace_all(entry_name, "\\", "/");
            size_t pos = entry_name.find_last_of("/");
            
            if(pos != string::npos) entry_name = entry_name.substr(pos + 1, entry_name.size() - pos - 1);
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
 * Generates the images that make up the area.
 */
void generate_area_images() {
    //First, clear all existing area images.
    for(size_t x = 0; x < area_images.size(); ++x) {
        for(size_t y = 0; y < area_images[x].size(); ++y) {
            al_destroy_bitmap(area_images[x][y]);
        }
        area_images[x].clear();
    }
    area_images.clear();
    
    //Now, figure out how big our area is.
    size_t n_sectors = cur_area_data.sectors.size();
    if(n_sectors == 0) return;
    
    float min_x, max_x, min_y, max_y;
    size_t n_vertexes = cur_area_data.vertexes.size();
    min_x = max_x = cur_area_data.vertexes[0]->x;
    min_y = max_y = cur_area_data.vertexes[0]->y;
    
    for(size_t v = 0; v < n_vertexes; ++v) {
        vertex* v_ptr = cur_area_data.vertexes[v];
        min_x = min(v_ptr->x, min_x);
        max_x = max(v_ptr->x, max_x);
        min_y = min(v_ptr->y, min_y);
        max_y = max(v_ptr->y, max_y);
    }
    
    min_x *= area_images_scale;
    max_x *= area_images_scale;
    min_y *= area_images_scale;
    max_y *= area_images_scale;
    area_images_x1 = min_x; area_images_y1 = min_y;
    
    //Create the new bitmaps on the vectors.
    float area_width = max_x - min_x;
    float area_height = max_y - min_y;
    unsigned area_image_cols = ceil(area_width / area_image_size);
    unsigned area_image_rows = ceil(area_height / area_image_size);
    
    for(size_t x = 0; x < area_image_cols; ++x) {
        area_images.push_back(vector<ALLEGRO_BITMAP*>());
        
        for(size_t y = 0; y < area_image_rows; ++y) {
            area_images[x].push_back(al_create_bitmap(area_image_size, area_image_size));
            ALLEGRO_BITMAP* old_bitmap = al_get_target_bitmap();
            al_set_target_bitmap(area_images[x].back());
            al_clear_to_color(al_map_rgba(0, 0, 0, 0));
            al_set_target_bitmap(old_bitmap);
        }
    }
    
    //For every sector, draw it on the area images it belongs on.
    for(size_t s = 0; s < n_sectors; ++s) {
        sector* s_ptr = cur_area_data.sectors[s];
        size_t n_edges = s_ptr->edges.size();
        if(n_edges == 0) continue;
        
        float s_min_x, s_max_x, s_min_y, s_max_y;
        unsigned sector_start_col, sector_end_col, sector_start_row, sector_end_row;
        get_sector_bounding_box(s_ptr, &s_min_x, &s_min_y, &s_max_x, &s_max_y);
        
        s_min_x *= area_images_scale;
        s_max_x *= area_images_scale;
        s_min_y *= area_images_scale;
        s_max_y *= area_images_scale;
        
        sector_start_col = (s_min_x - area_images_x1) / area_image_size;
        sector_end_col =   ceil((s_max_x - area_images_x1) / area_image_size) - 1;
        sector_start_row = (s_min_y - area_images_y1) / area_image_size;
        sector_end_row =   ceil((s_max_y - area_images_y1) / area_image_size) - 1;
        
        al_set_separate_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA, ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA);
        
        for(size_t x = sector_start_col; x <= sector_end_col; ++x) {
            for(size_t y = sector_start_row; y <= sector_end_row; ++y) {
                ALLEGRO_BITMAP* prev_target_bmp = al_get_target_bitmap();
                al_set_target_bitmap(area_images[x][y]); {
                
                    draw_sector(
                        cur_area_data.sectors[s],
                        (x * area_image_size + area_images_x1) / area_images_scale,
                        (y * area_image_size + area_images_y1) / area_images_scale,
                        area_images_scale
                    );
                    
                } al_set_target_bitmap(prev_target_bmp);
            }
        }
        
        al_set_blender(ALLEGRO_ADD, ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA);
        
    }
    
    for(size_t x = 0; x < area_image_cols; ++x) {
        for(size_t y = 0; y < area_image_rows; ++y) {
            //We need to "rebuild" the images, so that the mipmaps get updated.
            //Not doing this caused a month-old bug under OpenGL,
            //where zooming out = fade to black.
            area_images[x][y] = recreate_bitmap(area_images[x][y]);
        }
    }
    
}


/* ----------------------------------------------------------------------------
 * Returns the mob that is closest to the mouse cursor.
 */
mob* get_closest_mob_to_cursor() {
    float mx, my;
    get_mouse_cursor_coordinates(&mx, &my);
    
    dist closest_mob_to_cursor_dist = FLT_MAX;
    mob* closest_mob_to_cursor = NULL;
    
    for(size_t m = 0; m < mobs.size(); ++m) {
        mob* m_ptr = mobs[m];
        
        if(!m_ptr->fsm.cur_state) continue;
        
        dist d = dist(mx, my, m_ptr->x, m_ptr->y);
        if(d < closest_mob_to_cursor_dist) {
            closest_mob_to_cursor = m_ptr;
            closest_mob_to_cursor_dist = d;
        }
    }
    
    return closest_mob_to_cursor;
}


/* ----------------------------------------------------------------------------
 * Returns the daylight effect color for the current time and weather.
 */
ALLEGRO_COLOR get_daylight_color() {
    size_t n_points = cur_area_data.weather_condition.daylight.size();
    
    if(n_points == 1) {
        return cur_area_data.weather_condition.daylight[0].second;
    }
    
    for(size_t p = 0; p < n_points - 1; ++p) {
        auto cur_ptr = &cur_area_data.weather_condition.daylight[p];
        auto next_ptr = &cur_area_data.weather_condition.daylight[p + 1];
        
        if(day_minutes >= cur_ptr->first && day_minutes < next_ptr->first) {
        
            return interpolate_color(
                       day_minutes,
                       cur_ptr->first,
                       next_ptr->first,
                       cur_ptr->second,
                       next_ptr->second
                   );
        }
    }
    
    //If anything goes wrong, return a failsafe.
    return al_map_rgba(255, 255, 255, 0);
}


/* ----------------------------------------------------------------------------
 * Returns the in-world coordinates of the mouse cursor.
 */
void get_mouse_cursor_coordinates(float* x, float* y) {
    *x = mouse_cursor_x;
    *y = mouse_cursor_y;
    ALLEGRO_TRANSFORM t = get_world_to_screen_transform();
    al_invert_transform(&t);
    al_transform_coordinates(&t, x, y);
}


/* ----------------------------------------------------------------------------
 * Returns the width and height of a block of multi-line text.
 * Lines are split by a single "\n" character.
 * These are the dimensions of a bitmap that would hold a drawing by draw_text_lines().
 * font:  The text's font.
 * text:  The text.
 * ret_w: The width gets returned here, if not NULL.
 * ret_h: The height gets returned here, if not NULL.
 */
void get_multiline_text_dimensions(const ALLEGRO_FONT* const font, const string &text, int* ret_w, int* ret_h) {
    vector<string> lines = split(text, "\n", true);
    int fh = al_get_font_line_height(font);
    size_t n_lines = lines.size();
    
    if(ret_h) *ret_h = max(0, (int) ((fh + 1) * n_lines) - 1);
    
    if(ret_w) {
        int largest_w = 0;
        for(size_t l = 0; l < lines.size(); ++l) {
            largest_w = max(largest_w, al_get_text_width(font, lines[l].c_str()));
        }
        
        *ret_w = largest_w;
    }
}


/* ----------------------------------------------------------------------------
 * Returns the sun strength for the current time and weather.
 */
float get_sun_strength() {
    size_t n_points = cur_area_data.weather_condition.sun_strength.size();
    
    if(n_points == 1) {
        return cur_area_data.weather_condition.sun_strength[0].second;
    }
    
    for(size_t p = 0; p < n_points - 1; ++p) {
        auto cur_ptr = &cur_area_data.weather_condition.sun_strength[p];
        auto next_ptr = &cur_area_data.weather_condition.sun_strength[p + 1];
        
        if(day_minutes >= cur_ptr->first && day_minutes < next_ptr->first) {
        
            return interpolate_number(
                       day_minutes,
                       cur_ptr->first,
                       next_ptr->first,
                       cur_ptr->second,
                       next_ptr->second
                   ) / 255.0f;
        }
    }
    
    //If anything goes wrong, return a failsafe.
    return 1.0f;
}


/* ----------------------------------------------------------------------------
 * Returns the value of a var on the vars listing of a mob's spawn.
 */
string get_var_value(const string &vars_string, const string &var, const string &def) {
    vector<string> vars = semicolon_list_to_vector(vars_string);
    
    for(size_t v = 0; v < vars.size(); ++v) {
        size_t equals_pos = vars[v].find("=");
        string var_name = trim_spaces(vars[v].substr(0, equals_pos));
        
        if(var_name != var) continue;
        
        return trim_spaces(vars[v].substr(equals_pos + 1, vars[v].size() - (equals_pos + 1)), true);
    }
    
    return def;
}


/* ----------------------------------------------------------------------------
 * Returns an ALLEGRO_TRANSFORM that transforms world coordinates into screen coordinates.
 */
ALLEGRO_TRANSFORM get_world_to_screen_transform() {
    ALLEGRO_TRANSFORM t;
    al_identity_transform(&t);
    al_translate_transform(
        &t,
        -cam_x + scr_w / 2 * 1 / (cam_zoom),
        -cam_y + scr_h / 2 * 1 / (cam_zoom)
    );
    al_scale_transform(&t, cam_zoom, cam_zoom);
    return t;
}


/* ----------------------------------------------------------------------------
 * Returns the interpolation between two colors, given a number in an interval.
 * n: The number.
 * n1, n2: The interval the number falls on.
 ** The closer to n1, the closer the final color is to c1.
 * c1, c2: Colors.
 */
ALLEGRO_COLOR interpolate_color(const float n, const float n1, const float n2, const ALLEGRO_COLOR &c1, const ALLEGRO_COLOR &c2) {
    float progress = (float) (n - n1) / (float) (n2 - n1);
    return al_map_rgba_f(
               c1.r + progress * (c2.r - c1.r),
               c1.g + progress * (c2.g - c1.g),
               c1.b + progress * (c2.b - c1.b),
               c1.a + progress * (c2.a - c1.a)
           );
}


/* ----------------------------------------------------------------------------
 * Returns the interpolation of the value between two positions.
 */
float interpolate_number(const float p, const float p1, const float p2, const float v1, const float v2) {
    return v1 + ((p - p1) / (float) (p2 - p1)) * (v2 - v1);
}


/* ----------------------------------------------------------------------------
 * Loads an area into memory.
 */
void load_area(const string &name, const bool load_for_editor) {

    cur_area_data.clear();
    
    data_node data_file = load_data_file(AREA_FOLDER + "/" + name + "/Data.txt");
    
    cur_area_data.name = data_file.get_child_by_name("name")->get_value_or_default(name);
    cur_area_data.subtitle = data_file.get_child_by_name("subtitle")->value;
    
    draw_loading_screen(cur_area_data.name, cur_area_data.subtitle, 1.0);
    al_flip_display();
    
    cur_area_data.weather_name = data_file.get_child_by_name("weather")->value;
    if(!load_for_editor) {
        if(weather_conditions.find(cur_area_data.weather_name) == weather_conditions.end()) {
            error_log("Area " + name + " refers to a non-existing weather condition, \"" + cur_area_data.weather_name + "\"!", &data_file);
            cur_area_data.weather_condition = weather();
        } else {
            cur_area_data.weather_condition = weather_conditions[cur_area_data.weather_name];
        }
    }
    
    cur_area_data.bg_bmp_file_name = data_file.get_child_by_name("bg_bmp")->value;
    if(!load_for_editor && !cur_area_data.bg_bmp_file_name.empty()) {
        cur_area_data.bg_bmp = bitmaps.get(cur_area_data.bg_bmp_file_name, &data_file);
    }
    cur_area_data.bg_color = s2c(data_file.get_child_by_name("bg_color")->value);
    cur_area_data.bg_dist = s2f(data_file.get_child_by_name("bg_dist")->get_value_or_default("2"));
    cur_area_data.bg_bmp_zoom = s2f(data_file.get_child_by_name("bg_zoom")->get_value_or_default("1"));
    
    
    data_node geometry_file = load_data_file(AREA_FOLDER + "/" + name + "/Geometry.txt");
    
    //Vertexes.
    size_t n_vertexes = geometry_file.get_child_by_name("vertexes")->get_nr_of_children_by_name("v");
    for(size_t v = 0; v < n_vertexes; ++v) {
        data_node* vertex_data = geometry_file.get_child_by_name("vertexes")->get_child_by_name("v", v);
        vector<string> words = split(vertex_data->value);
        if(words.size() == 2) cur_area_data.vertexes.push_back(new vertex(s2f(words[0]), s2f(words[1])));
    }
    
    //Edges.
    size_t n_edges = geometry_file.get_child_by_name("edges")->get_nr_of_children_by_name("e");
    for(size_t e = 0; e < n_edges; ++e) {
        data_node* edge_data = geometry_file.get_child_by_name("edges")->get_child_by_name("e", e);
        edge* new_edge = new edge();
        
        vector<string> s_nrs = split(edge_data->get_child_by_name("s")->value);
        if(s_nrs.size() < 2) s_nrs.insert(s_nrs.end(), 2, "-1");
        for(size_t s = 0; s < 2; ++s) {
            if(s_nrs[s] == "-1") new_edge->sector_nrs[s] = INVALID;
            else new_edge->sector_nrs[s] = s2i(s_nrs[s]);
        }
        
        vector<string> v_nrs = split(edge_data->get_child_by_name("v")->value);
        if(v_nrs.size() < 2) v_nrs.insert(v_nrs.end(), 2, "0");
        
        new_edge->vertex_nrs[0] = s2i(v_nrs[0]);
        new_edge->vertex_nrs[1] = s2i(v_nrs[1]);
        
        cur_area_data.edges.push_back(new_edge);
    }
    
    //Sectors.
    size_t n_sectors = geometry_file.get_child_by_name("sectors")->get_nr_of_children_by_name("s");
    for(size_t s = 0; s < n_sectors; ++s) {
        data_node* sector_data = geometry_file.get_child_by_name("sectors")->get_child_by_name("s", s);
        sector* new_sector = new sector();
        
        new_sector->type = sector_types.get_nr(sector_data->get_child_by_name("type")->value);
        if(new_sector->type == 255) new_sector->type = SECTOR_TYPE_NORMAL;
        new_sector->brightness = s2f(sector_data->get_child_by_name("brightness")->get_value_or_default(i2s(DEF_SECTOR_BRIGHTNESS)));
        new_sector->tag = sector_data->get_child_by_name("tag")->value;
        new_sector->z = s2f(sector_data->get_child_by_name("z")->value);
        new_sector->fade = s2b(sector_data->get_child_by_name("fade")->value);
        new_sector->always_cast_shadow = s2b(sector_data->get_child_by_name("always_cast_shadow")->value);
        
        new_sector->texture_info.file_name = sector_data->get_child_by_name("texture")->value;
        new_sector->texture_info.rot = s2f(sector_data->get_child_by_name("texture_rotate")->value);
        
        vector<string> scales = split(sector_data->get_child_by_name("texture_scale")->value);
        if(scales.size() >= 2) {
            new_sector->texture_info.scale_x = s2f(scales[0]);
            new_sector->texture_info.scale_y = s2f(scales[0]);
        }
        vector<string> translations = split(sector_data->get_child_by_name("texture_trans")->value);
        if(translations.size() >= 2) {
            new_sector->texture_info.trans_x = s2f(translations[0]);
            new_sector->texture_info.trans_y = s2f(translations[1]);
        }
        
        data_node* hazards_node = sector_data->get_child_by_name("hazards");
        vector<string> hazards_strs = semicolon_list_to_vector(hazards_node->value);
        for(size_t h = 0; h < hazards_strs.size(); ++h) {
            string hazard_name = hazards_strs[h];
            if(hazards.find(hazard_name) == hazards.end()) {
                error_log("Hazard \"" + hazard_name + "\" not found!", hazards_node);
            } else {
                new_sector->hazards.push_back(&(hazards[hazard_name]));
            }
        }
        new_sector->hazards_str = hazards_node->value;
        new_sector->hazard_floor = s2b(sector_data->get_child_by_name("hazards_floor")->get_value_or_default("true"));
        
        cur_area_data.sectors.push_back(new_sector);
    }
    
    //Mobs.
    size_t n_mobs = geometry_file.get_child_by_name("mobs")->get_nr_of_children();
    for(size_t m = 0; m < n_mobs; ++m) {
    
        data_node* mob_node = geometry_file.get_child_by_name("mobs")->get_child(m);
        
        mob_gen* mob_ptr = new mob_gen();
        
        vector<string> coords = split(mob_node->get_child_by_name("p")->value);
        mob_ptr->x = (coords.size() >= 1 ? s2f(coords[0]) : 0);
        mob_ptr->y = (coords.size() >= 2 ? s2f(coords[1]) : 0);
        mob_ptr->angle = s2f(mob_node->get_child_by_name("angle")->get_value_or_default("0"));
        mob_ptr->vars = mob_node->get_child_by_name("vars")->value;
        
        mob_ptr->category = mob_categories.get_nr_from_sname(mob_node->name);
        string mt = mob_node->get_child_by_name("type")->value;
        mob_categories.set_mob_type_ptr(mob_ptr, mt);
        
        bool problem = false;
        
        if(!mob_ptr->type && !load_for_editor) {
            //Error.
            error_log(
                "Unknown \"" + mob_categories.get_sname(mob_ptr->category) + "\" mob type \"" +
                mt + "\"!", mob_node
            );
            problem = true;
        }
        
        if((mob_ptr->category == MOB_CATEGORY_NONE || mob_ptr->category == 255) && !load_for_editor) {
        
            error_log("Unknown mob category \"" + mob_node->name + "\"!", mob_node);
            mob_ptr->category = MOB_CATEGORY_NONE;
            problem = true;
            
        }
        
        if(!problem) cur_area_data.mob_generators.push_back(mob_ptr);
    }
    
    //Path stops.
    size_t n_stops = geometry_file.get_child_by_name("path_stops")->get_nr_of_children();
    for(size_t s = 0; s < n_stops; ++s) {
    
        data_node* path_stop_node = geometry_file.get_child_by_name("path_stops")->get_child(s);
        
        path_stop* s_ptr = new path_stop();
        
        vector<string> words = split(path_stop_node->get_child_by_name("pos")->value);
        s_ptr->x = (words.size() >= 1 ? s2f(words[0]) : 0);
        s_ptr->y = (words.size() >= 2 ? s2f(words[1]) : 0);
        
        data_node* links_node = path_stop_node->get_child_by_name("links");
        size_t n_links = links_node->get_nr_of_children();
        
        for(size_t l = 0; l < n_links; ++l) {
        
            data_node* link_node = links_node->get_child(l);
            path_link l_struct(NULL, INVALID);
            
            l_struct.end_nr = s2i(link_node->value);
            
            s_ptr->links.push_back(l_struct);
            
        }
        
        cur_area_data.path_stops.push_back(s_ptr);
    }
    
    
    //Tree shadows.
    size_t n_shadows = geometry_file.get_child_by_name("tree_shadows")->get_nr_of_children();
    for(size_t s = 0; s < n_shadows; ++s) {
    
        data_node* shadow_node = geometry_file.get_child_by_name("tree_shadows")->get_child(s);
        
        tree_shadow* s_ptr = new tree_shadow();
        
        vector<string> words = split(shadow_node->get_child_by_name("pos")->value);
        s_ptr->x = (words.size() >= 1 ? s2f(words[0]) : 0);
        s_ptr->y = (words.size() >= 2 ? s2f(words[1]) : 0);
        
        words = split(shadow_node->get_child_by_name("size")->value);
        s_ptr->w = (words.size() >= 1 ? s2f(words[0]) : 0);
        s_ptr->h = (words.size() >= 2 ? s2f(words[1]) : 0);
        
        s_ptr->angle = s2f(shadow_node->get_child_by_name("angle")->get_value_or_default("0"));
        s_ptr->alpha = s2i(shadow_node->get_child_by_name("alpha")->get_value_or_default("255"));
        s_ptr->file_name = shadow_node->get_child_by_name("file")->value;
        s_ptr->bitmap = bitmaps.get("Textures/" + s_ptr->file_name, NULL);
        
        words = split(shadow_node->get_child_by_name("sway")->value);
        s_ptr->sway_x = (words.size() >= 1 ? s2f(words[0]) : 0);
        s_ptr->sway_y = (words.size() >= 2 ? s2f(words[1]) : 0);
        
        if(s_ptr->bitmap == bmp_error && !load_for_editor) {
            error_log("Unknown tree shadow texture \"" + s_ptr->file_name + "\"!", shadow_node);
        }
        
        cur_area_data.tree_shadows.push_back(s_ptr);
        
    }
    
    
    //Set up stuff.
    for(size_t e = 0; e < cur_area_data.edges.size(); ++e) {
        cur_area_data.edges[e]->fix_pointers(cur_area_data);
    }
    for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
        cur_area_data.sectors[s]->connect_edges(cur_area_data, s);
    }
    for(size_t v = 0; v < cur_area_data.vertexes.size(); ++v) {
        cur_area_data.vertexes[v]->connect_edges(cur_area_data, v);
    }
    for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
        cur_area_data.path_stops[s]->fix_pointers(cur_area_data);
    }
    for(size_t s = 0; s < cur_area_data.path_stops.size(); ++s) {
        path_stop* s_ptr = cur_area_data.path_stops[s];
        for(size_t l = 0; l < s_ptr->links.size(); ++l) {
            s_ptr->links[l].calculate_dist(s_ptr);
        }
    }
    
    
    //Triangulate everything.
    for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
        sector* s_ptr = cur_area_data.sectors[s];
        s_ptr->triangles.clear();
        triangulate(s_ptr);
    }
    
    
    //Editor guide.
    if(load_for_editor) {
        area_editor* ae = (area_editor*) game_states[cur_game_state_nr];
        ae->set_guide_file_name(    geometry_file.get_child_by_name("guide_file_name")->value);
        ae->set_guide_x(        s2f(geometry_file.get_child_by_name("guide_x")->value));
        ae->set_guide_y(        s2f(geometry_file.get_child_by_name("guide_y")->value));
        ae->set_guide_w(        s2f(geometry_file.get_child_by_name("guide_w")->value));
        ae->set_guide_h(        s2f(geometry_file.get_child_by_name("guide_h")->value));
        ae->set_guide_a(        s2i(geometry_file.get_child_by_name("guide_alpha")->get_value_or_default("255")));
    }
    
    if(!load_for_editor) cur_area_data.generate_blockmap();
}


/* ----------------------------------------------------------------------------
 * Loads the area's sector textures.
 */
void load_area_textures() {
    for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
        sector* s_ptr = cur_area_data.sectors[s];
        
        for(unsigned char t = 0; t < ((s_ptr->fade) ? 2 : 1); ++t) {
            if(s_ptr->texture_info.file_name.empty()) {
                s_ptr->texture_info.bitmap = NULL;
            } else {
                s_ptr->texture_info.bitmap = bitmaps.get("Textures/" + s_ptr->texture_info.file_name, NULL);
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Loads a bitmap from the game's content.
 * If the node is present, it'll be used to report errors.
 */
ALLEGRO_BITMAP* load_bmp(const string &file_name, data_node* node, bool report_error) {
    if(file_name.empty()) return NULL;
    ALLEGRO_BITMAP* b = al_load_bitmap((GRAPHICS_FOLDER + "/" + file_name).c_str());
    
    if(!b && report_error) {
        error_log("Could not open image " + file_name + "!", node);
        b = bmp_error;
    }
    
    return b;
}


/* ----------------------------------------------------------------------------
 * Loads a game control.
 */
void load_control(const unsigned char action, const unsigned char player, const string &name, data_node &file, const string &def) {
    string s = file.get_child_by_name("p" + i2s((player + 1)) + "_" + name)->get_value_or_default((player == 0) ? def : "");
    vector<string> possible_controls = split(s, ",");
    size_t n_possible_controls = possible_controls.size();
    
    for(size_t c = 0; c < n_possible_controls; ++c) {
        controls[player].push_back(control_info(action, possible_controls[c]));
        
    }
}


/* ----------------------------------------------------------------------------
 * Loads a data file from the game's content.
 */
data_node load_data_file(const string &file_name) {
    data_node n = data_node(file_name);
    if(!n.file_was_opened) {
        error_log("Could not open data file " + file_name + "!");
    }
    
    return n;
}


/* ----------------------------------------------------------------------------
 * Loads the game's configuration file.
 */
void load_game_config() {
    data_node file(CONFIG_FILE);
    
    game_name = file.get_child_by_name("game_name")->value;
    game_version = file.get_child_by_name("game_version")->value;
    
    set_if_exists(file.get_child_by_name("carrying_color_move")->value, carrying_color_move);
    set_if_exists(file.get_child_by_name("carrying_color_stop")->value, carrying_color_stop);
    set_if_exists(file.get_child_by_name("carrying_speed_base_mult")->value, carrying_speed_base_mult);
    set_if_exists(file.get_child_by_name("carrying_speed_max_mult")->value, carrying_speed_max_mult);
    set_if_exists(file.get_child_by_name("carrying_speed_weight_mult")->value, carrying_speed_weight_mult);
    
    set_if_exists(file.get_child_by_name("day_minutes_start")->value, day_minutes_start);
    set_if_exists(file.get_child_by_name("day_minutes_end")->value, day_minutes_end);
    set_if_exists(file.get_child_by_name("day_minutes_per_irl_sec")->value, day_minutes_per_irl_sec);
    
    set_if_exists(file.get_child_by_name("idle_task_range")->value, idle_task_range);
    set_if_exists(file.get_child_by_name("group_move_task_range")->value, group_move_task_range);
    set_if_exists(file.get_child_by_name("max_pikmin_in_field")->value, max_pikmin_in_field);
    set_if_exists(file.get_child_by_name("maturity_power_mult")->value, maturity_power_mult);
    set_if_exists(file.get_child_by_name("maturity_speed_mult")->value, maturity_speed_mult);
    set_if_exists(file.get_child_by_name("nectar_amount")->value, nectar_amount);
    
    set_if_exists(file.get_child_by_name("cursor_max_dist")->value, cursor_max_dist);
    set_if_exists(file.get_child_by_name("cursor_spin_speed")->value, cursor_spin_speed);
    set_if_exists(file.get_child_by_name("next_pluck_range")->value, next_pluck_range);
    set_if_exists(file.get_child_by_name("onion_open_range")->value, onion_open_range);
    set_if_exists(file.get_child_by_name("pikmin_grab_range")->value, pikmin_grab_range);
    set_if_exists(file.get_child_by_name("pluck_range")->value, pluck_range);
    set_if_exists(file.get_child_by_name("whistle_growth_speed")->value, whistle_growth_speed);
    
    set_if_exists(file.get_child_by_name("info_spot_trigger_range")->value, info_spot_trigger_range);
    set_if_exists(file.get_child_by_name("message_char_interval")->value, message_char_interval);
    set_if_exists(file.get_child_by_name("zoom_max_level")->value, zoom_max_level);
    set_if_exists(file.get_child_by_name("zoom_min_level")->value, zoom_min_level);
    
    al_set_window_title(display, game_name.c_str());
}


/* ----------------------------------------------------------------------------
 * Loads all of the game's content.
 */
void load_game_content() {
    load_status_types();
    load_spray_types();
    load_hazards();
    
    //Mob types.
    load_mob_types(true);
    
    //Weather.
    weather_conditions.clear();
    data_node weather_file = load_data_file(WEATHER_FILE);
    size_t n_weather_conditions = weather_file.get_nr_of_children_by_name("weather");
    
    for(size_t wc = 0; wc < n_weather_conditions; ++wc) {
        data_node* cur_weather = weather_file.get_child_by_name("weather", wc);
        
        string name = cur_weather->get_child_by_name("name")->value;
        if(name.empty()) name = "default";
        
        //Lighting.
        vector<pair<size_t, ALLEGRO_COLOR> > lighting;
        size_t n_lighting_points = cur_weather->get_child_by_name("lighting")->get_nr_of_children();
        
        bool have_midnight = false;
        
        for(size_t lp = 0; lp < n_lighting_points; ++lp) {
            data_node* lighting_node = cur_weather->get_child_by_name("lighting")->get_child(lp);
            
            size_t point_time = s2i(lighting_node->name);
            ALLEGRO_COLOR point_color = s2c(lighting_node->value);
            
            lighting.push_back(make_pair(point_time, point_color));
            
            if(point_time == 24 * 60) have_midnight = true;
        }
        
        sort(lighting.begin(), lighting.end(), [] (pair<size_t, ALLEGRO_COLOR> p1, pair<size_t, ALLEGRO_COLOR> p2) -> bool {
            return p1.first < p2.first;
        });
        
        if(lighting.empty()) {
            error_log("Weather condition " + name + " has no lighting!");
        } else {
            if(!have_midnight) {
                //If there is no data for the last hour, use the data from the first point
                //(this is because the day loops after 24:00; needed for interpolation).
                lighting.push_back(make_pair(24 * 60, lighting[0].second));
            }
        }
        
        //Sun's strength.
        vector<pair<size_t, unsigned char> > sun_strength;
        size_t n_sun_strength_points = cur_weather->get_child_by_name("sun_strength")->get_nr_of_children();
        
        have_midnight = false;
        
        for(size_t sp = 0; sp < n_sun_strength_points; ++sp) {
            data_node* sun_strength_node = cur_weather->get_child_by_name("sun_strength")->get_child(sp);
            
            size_t point_time = s2i(sun_strength_node->name);
            unsigned char point_strength = s2i(sun_strength_node->value);
            
            sun_strength.push_back(make_pair(point_time, point_strength));
            
            if(point_time == 24 * 60) have_midnight = true;
        }
        
        sort(sun_strength.begin(), sun_strength.end(), [] (pair<size_t, unsigned char> p1, pair<size_t, unsigned char> p2) -> bool {
            return p1.first < p2.first;
        });
        
        if(!sun_strength.empty()) {
            if(!have_midnight) {
                //If there is no data for the last hour, use the data from the first point
                //(this is because the day loops after 24:00; needed for interpolation).
                sun_strength.push_back(make_pair(24 * 60, sun_strength[0].second));
            }
        }
        
        //Percipitation.
        unsigned char percipitation_type = s2i(cur_weather->get_child_by_name("percipitation_type")->get_value_or_default(i2s(PERCIPITATION_TYPE_NONE)));
        interval percipitation_frequency = interval(cur_weather->get_child_by_name("percipitation_frequency")->value);
        interval percipitation_speed = interval(cur_weather->get_child_by_name("percipitation_speed")->value);
        interval percipitation_angle = interval(cur_weather->get_child_by_name("percipitation_angle")->get_value_or_default(f2s((M_PI + M_PI_2))));
        
        //Save.
        weather_conditions[name] = weather(name, lighting, sun_strength, percipitation_type, percipitation_frequency, percipitation_speed, percipitation_angle);
    }
}


/* ----------------------------------------------------------------------------
 *  Loads HUD coordinates from a file.
 */
void load_hud_coordinates() {
    data_node file = data_node(MISC_FOLDER + "/HUD.txt");
    if(!file.file_was_opened) return;
    
    load_hud_coordinates(HUD_ITEM_TIME,                file.get_child_by_name("time")->value);
    load_hud_coordinates(HUD_ITEM_DAY_BUBBLE,          file.get_child_by_name("day_bubble")->value);
    load_hud_coordinates(HUD_ITEM_DAY_NUMBER,          file.get_child_by_name("day_number")->value);
    load_hud_coordinates(HUD_ITEM_LEADER_1_ICON,       file.get_child_by_name("leader_1_icon")->value);
    load_hud_coordinates(HUD_ITEM_LEADER_2_ICON,       file.get_child_by_name("leader_2_icon")->value);
    load_hud_coordinates(HUD_ITEM_LEADER_3_ICON,       file.get_child_by_name("leader_3_icon")->value);
    load_hud_coordinates(HUD_ITEM_LEADER_1_HEALTH,     file.get_child_by_name("leader_1_health")->value);
    load_hud_coordinates(HUD_ITEM_LEADER_2_HEALTH,     file.get_child_by_name("leader_2_health")->value);
    load_hud_coordinates(HUD_ITEM_LEADER_3_HEALTH,     file.get_child_by_name("leader_3_health")->value);
    load_hud_coordinates(HUD_ITEM_PIKMIN_STANDBY_ICON, file.get_child_by_name("pikmin_standby_icon")->value);
    load_hud_coordinates(HUD_ITEM_PIKMIN_STANDBY_NR,   file.get_child_by_name("pikmin_standby_nr")->value);
    load_hud_coordinates(HUD_ITEM_PIKMIN_STANDBY_X,    file.get_child_by_name("pikmin_standby_x")->value);
    load_hud_coordinates(HUD_ITEM_PIKMIN_GROUP_NR,     file.get_child_by_name("pikmin_group_nr")->value);
    load_hud_coordinates(HUD_ITEM_PIKMIN_FIELD_NR,     file.get_child_by_name("pikmin_field_nr")->value);
    load_hud_coordinates(HUD_ITEM_PIKMIN_TOTAL_NR,     file.get_child_by_name("pikmin_total_nr")->value);
    load_hud_coordinates(HUD_ITEM_PIKMIN_SLASH_1,      file.get_child_by_name("pikmin_slash_1")->value);
    load_hud_coordinates(HUD_ITEM_PIKMIN_SLASH_2,      file.get_child_by_name("pikmin_slash_2")->value);
    load_hud_coordinates(HUD_ITEM_PIKMIN_SLASH_3,      file.get_child_by_name("pikmin_slash_3")->value);
    load_hud_coordinates(HUD_ITEM_SPRAY_1_ICON,        file.get_child_by_name("spray_1_icon")->value);
    load_hud_coordinates(HUD_ITEM_SPRAY_1_AMOUNT,      file.get_child_by_name("spray_1_amount")->value);
    load_hud_coordinates(HUD_ITEM_SPRAY_1_KEY,         file.get_child_by_name("spray_1_key")->value);
    load_hud_coordinates(HUD_ITEM_SPRAY_2_ICON,        file.get_child_by_name("spray_2_icon")->value);
    load_hud_coordinates(HUD_ITEM_SPRAY_2_AMOUNT,      file.get_child_by_name("spray_2_amount")->value);
    load_hud_coordinates(HUD_ITEM_SPRAY_2_KEY,         file.get_child_by_name("spray_2_key")->value);
    load_hud_coordinates(HUD_ITEM_SPRAY_PREV_ICON,     file.get_child_by_name("spray_prev_icon")->value);
    load_hud_coordinates(HUD_ITEM_SPRAY_PREV_KEY,      file.get_child_by_name("spray_prev_key")->value);
    load_hud_coordinates(HUD_ITEM_SPRAY_NEXT_ICON,     file.get_child_by_name("spray_next_icon")->value);
    load_hud_coordinates(HUD_ITEM_SPRAY_NEXT_KEY,      file.get_child_by_name("spray_next_key")->value);
    
    for(int i = 0; i < N_HUD_ITEMS; ++i) {
        for(unsigned char c = 0; c < 4; ++c) {
            if(hud_coords[i][c] == 0) {
                hud_coords[i][c] = -1;
            } else {
                if(c % 2 == 0) {
                    hud_coords[i][c] *= scr_w;
                } else {
                    hud_coords[i][c] *= scr_h;
                }
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Loads the hazards from the game data.
 */
void load_hazards() {
    data_node file = data_node(MISC_FOLDER + "/Hazards.txt");
    if(!file.file_was_opened) return;
    
    size_t n_hazards = file.get_nr_of_children();
    for(size_t h = 0; h < n_hazards; ++h) {
        data_node* h_node = file.get_child(h);
        hazard h_struct;
        
        h_struct.name = h_node->name;
        
        data_node* effects_node = h_node->get_child_by_name("effects");
        vector<string> effects_strs = semicolon_list_to_vector(effects_node->value);
        for(size_t e = 0; e < effects_strs.size(); ++e) {
            string effect_name = effects_strs[e];
            if(status_types.find(effect_name) == status_types.end()) {
                error_log("Status effect \"" + effect_name + "\" not found!", effects_node);
            } else {
                h_struct.effects.push_back(&(status_types[effect_name]));
            }
        }
        
        set_if_exists(h_node->get_child_by_name("color")->value, h_struct.main_color);
        
        hazards[h_node->name] = h_struct;
    }
}


/* ----------------------------------------------------------------------------
 * Loads HUD coordinates of a specific HUD item.
 */
void load_hud_coordinates(const int item, string data) {
    vector<string> words = split(data);
    if(data.size() < 4) return;
    
    for(unsigned char c = 0; c < 4; ++c) {
        hud_coords[item][c] = s2f(words[c]) / 100.0f;
    }
}



/* ----------------------------------------------------------------------------
 * Loads the player's options.
 */
void load_options() {
    data_node file = data_node("Options.txt");
    if(!file.file_was_opened) return;
    
    //Load joysticks.
    joystick_numbers.clear();
    int n_joysticks = al_get_num_joysticks();
    for(int j = 0; j < n_joysticks; ++j) {
        joystick_numbers[al_get_joystick(j)] = j;
    }
    
    //Load controls.
    //Format of a control: "p<player number>_<action>=<possible control 1>,<possible control 2>,<...>"
    //Format of a possible control: "<input method>_<parameters, underscore separated>"
    //Input methods: "k" (keyboard key), "mb" (mouse button), "mwu" (mouse wheel up), "mwd" (down),
    //"mwl" (left), "mwr" (right), "jb" (joystick button), "jap" (joystick axis, positive), "jan" (joystick axis, negative).
    //The parameters are the key/button number, joystick number, joystick stick and axis, etc.
    //Check the constructor of control_info for more information.
    for(size_t p = 0; p < 4; p++) {
        controls[p].clear();
    }
    
    for(unsigned char p = 0; p < 4; ++p) {
        load_control(BUTTON_THROW,                p, "punch", file);
        load_control(BUTTON_WHISTLE,              p, "whistle", file);
        load_control(BUTTON_MOVE_RIGHT,           p, "move_right", file);
        load_control(BUTTON_MOVE_UP,              p, "move_up", file);
        load_control(BUTTON_MOVE_LEFT,            p, "move_left", file);
        load_control(BUTTON_MOVE_DOWN,            p, "move_down", file);
        load_control(BUTTON_MOVE_CURSOR_RIGHT,    p, "move_cursor_right", file);
        load_control(BUTTON_MOVE_CURSOR_UP,       p, "move_cursor_up", file);
        load_control(BUTTON_MOVE_CURSOR_LEFT,     p, "move_cursor_left", file);
        load_control(BUTTON_MOVE_CURSOR_DOWN,     p, "move_cursor_down", file);
        load_control(BUTTON_GROUP_MOVE_GO_TO_CURSOR, p, "group_move_go_to_cursor", file);
        load_control(BUTTON_GROUP_MOVE_RIGHT,     p, "group_move_right", file);
        load_control(BUTTON_GROUP_MOVE_UP,        p, "group_move_up", file);
        load_control(BUTTON_GROUP_MOVE_LEFT,      p, "group_move_left", file);
        load_control(BUTTON_GROUP_MOVE_DOWN,      p, "group_move_down", file);
        load_control(BUTTON_SWITCH_LEADER_RIGHT,  p, "switch_leader_right", file);
        load_control(BUTTON_SWITCH_LEADER_LEFT,   p, "switch_leader_left", file);
        load_control(BUTTON_DISMISS,              p, "dismiss", file);
        load_control(BUTTON_USE_SPRAY_1,          p, "use_spray_1", file);
        load_control(BUTTON_USE_SPRAY_2,          p, "use_spray_2", file);
        load_control(BUTTON_USE_SPRAY,            p, "use_spray", file);
        load_control(BUTTON_SWITCH_SPRAY_RIGHT,   p, "switch_spray_right", file);
        load_control(BUTTON_SWITCH_SPRAY_LEFT,    p, "switch_spray_left", file);
        load_control(BUTTON_SWITCH_ZOOM,          p, "switch_zoom", file);
        load_control(BUTTON_ZOOM_IN,              p, "zoom_in", file);
        load_control(BUTTON_ZOOM_OUT,             p, "zoom_out", file);
        load_control(BUTTON_SWITCH_TYPE_RIGHT,    p, "switch_type_right", file);
        load_control(BUTTON_SWITCH_TYPE_LEFT,     p, "switch_type_left", file);
        load_control(BUTTON_SWITCH_MATURITY_UP,   p, "switch_maturity_up", file);
        load_control(BUTTON_SWITCH_MATURITY_DOWN, p, "switch_maturity_down", file);
        load_control(BUTTON_LIE_DOWN,             p, "lie_down", file);
        load_control(BUTTON_PAUSE,                p, "pause", file);
    }
    
    //Weed out controls that didn't parse correctly.
    for(size_t p = 0; p < 4; p++) {
        size_t n_controls = controls[p].size();
        for(size_t c = 0; c < n_controls; ) {
            if(controls[p][c].action == BUTTON_NONE) {
                controls[p].erase(controls[p].begin() + c);
            } else {
                c++;
            }
        }
    }
    
    for(unsigned char p = 0; p < 4; ++p) {
        mouse_moves_cursor[p] = s2b(file.get_child_by_name("p" + i2s((p + 1)) + "_mouse_moves_cursor")->get_value_or_default((p == 0) ? "true" : "false"));
    }
    
    //Other options.
    area_images_scale = s2f(file.get_child_by_name("area_quality")->get_value_or_default("1"));
    daylight_effect = s2b(file.get_child_by_name("daylight_effect")->get_value_or_default("true"));
    draw_cursor_trail = s2b(file.get_child_by_name("draw_cursor_trail")->get_value_or_default("true"));
    game_fps = s2i(file.get_child_by_name("fps")->get_value_or_default("30"));
    scr_h = s2i(file.get_child_by_name("height")->get_value_or_default(i2s(DEF_SCR_H)));
    particle_quality = s2i(file.get_child_by_name("particle_quality")->get_value_or_default("2"));
    pretty_whistle = s2b(file.get_child_by_name("pretty_whistle")->get_value_or_default("true"));
    scr_w = s2i(file.get_child_by_name("width")->get_value_or_default(i2s(DEF_SCR_W)));
    smooth_scaling = s2b(file.get_child_by_name("smooth_scaling")->get_value_or_default("true"));
    window_pos_hack = s2b(file.get_child_by_name("window_pos_hack")->get_value_or_default("false"));
}


/* ----------------------------------------------------------------------------
 * Loads an audio sample from the game's content.
 */
sample_struct load_sample(const string &file_name, ALLEGRO_MIXER* const mixer) {
    ALLEGRO_SAMPLE* sample = al_load_sample((AUDIO_FOLDER + "/" + file_name).c_str());
    if(!sample) {
        error_log("Could not open audio sample " + file_name + "!");
    }
    
    return sample_struct(sample, mixer);
}


/* ----------------------------------------------------------------------------
 * Loads spray types from the game data.
 */
void load_spray_types() {
    data_node file = data_node(MISC_FOLDER + "/Sprays.txt");
    if(!file.file_was_opened) return;
    
    size_t n_sprays = file.get_nr_of_children();
    for(size_t s = 0; s < n_sprays; ++s) {
        data_node* s_node = file.get_child(s);
        spray_type st;
        
        st.name = s_node->name;
        
        data_node* effects_node = s_node->get_child_by_name("effects");
        vector<string> effects_strs = semicolon_list_to_vector(effects_node->value);
        for(size_t e = 0; e < effects_strs.size(); ++e) {
            string effect_name = effects_strs[e];
            if(status_types.find(effect_name) == status_types.end()) {
                error_log("Status effect \"" + effect_name + "\" not found!", effects_node);
            } else {
                st.effects.push_back(&(status_types[effect_name]));
            }
        }
        
        set_if_exists(s_node->get_child_by_name("group")->value, st.group);
        set_if_exists(s_node->get_child_by_name("angle")->value, st.angle);
        set_if_exists(s_node->get_child_by_name("distance_range")->value, st.distance_range);
        set_if_exists(s_node->get_child_by_name("angle_range")->value, st.angle_range);
        set_if_exists(s_node->get_child_by_name("color")->value, st.main_color);
        set_if_exists(s_node->get_child_by_name("berries_needed")->value, st.berries_needed);
        
        data_node* icon_node = s_node->get_child_by_name("icon");
        st.bmp_spray = bitmaps.get(icon_node->value, icon_node);
        
        spray_types.push_back(st);
    }
}


/* ----------------------------------------------------------------------------
 * Loads status effect types from the game data.
 */
void load_status_types() {
    data_node file = data_node(MISC_FOLDER + "/Statuses.txt");
    if(!file.file_was_opened) return;
    
    size_t n_statuses = file.get_nr_of_children();
    for(size_t s = 0; s < n_statuses; ++s) {
        data_node* s_node = file.get_child(s);
        status_type st;
        
        st.name = s_node->name;
        
        set_if_exists(s_node->get_child_by_name("color")->value, st.color);
        set_if_exists(s_node->get_child_by_name("tint")->value, st.tint);
        set_if_exists(s_node->get_child_by_name("removable_with_whistle")->value, st.removable_with_whistle);
        set_if_exists(s_node->get_child_by_name("auto_remove_time")->value, st.auto_remove_time);
        set_if_exists(s_node->get_child_by_name("health_change_ratio")->value, st.health_change_ratio);
        set_if_exists(s_node->get_child_by_name("causes_panic")->value, st.causes_panic);
        set_if_exists(s_node->get_child_by_name("causes_flailing")->value, st.causes_flailing);
        set_if_exists(s_node->get_child_by_name("speed_multiplier")->value, st.speed_multiplier);
        set_if_exists(s_node->get_child_by_name("attack_multiplier")->value, st.attack_multiplier);
        set_if_exists(s_node->get_child_by_name("defense_multiplier")->value, st.defense_multiplier);
        set_if_exists(s_node->get_child_by_name("anim_speed_multiplier")->value, st.anim_speed_multiplier);
        
        st.affects = 0;
        if(s2b(s_node->get_child_by_name("affects_pikmin")->value))  st.affects |= STATUS_AFFECTS_PIKMIN;
        if(s2b(s_node->get_child_by_name("affects_leaders")->value)) st.affects |= STATUS_AFFECTS_LEADERS;
        if(s2b(s_node->get_child_by_name("affects_enemies")->value)) st.affects |= STATUS_AFFECTS_ENEMIES;
        
        status_types[st.name] = st;
    }
}


/* ----------------------------------------------------------------------------
 * Returns the movement necessary to move a point.
 * x/y:          Coordinates of the initial point.
 * tx/ty:        Coordinates of the target point.
 * speed:        Speed at which the point can move.
 * reach_radius: If the point is within this range of the target, consider it as already being there.
 * mx/my:        Variables to return the amount of movement to.
 * angle:        Variable to return the angle the point faces to.
 * reached:      Variable to return whether the point reached the target or not to.
 */
void move_point(const float x, const float y, const float tx, const float ty, const float speed, const float reach_radius, float* mx, float* my, float* angle, bool* reached) {
    float dx = tx - x, dy = ty - y;
    float dist = sqrt(dx * dx + dy * dy);
    
    if(dist > reach_radius) {
        float move_amount = min((double) (dist / delta_t / 2.0f), (double) speed);
        
        dx *= move_amount / dist;
        dy *= move_amount / dist;
        
        if(mx) *mx = dx;
        if(my) *my = dy;
        if(angle) *angle = atan2(dy, dx);
        if(reached) *reached = false;
        
    } else {
    
        if(mx) *mx = 0;
        if(my) *my = 0;
        if(reached) *reached = true;
    }
}


/* ----------------------------------------------------------------------------
 * Normalizes an angle so that it's between 0 and M_PI * 2.
 */
float normalize_angle(float a) {
    a = fmod((double) a, M_PI * 2);
    if(a < 0) a += M_PI * 2;
    return a;
}


/* ----------------------------------------------------------------------------
 * Prints a bit of info onto the screen, for some seconds.
 */
void print_info(string text) {
    info_print_text = text;
    info_print_timer.start();
}


/* ----------------------------------------------------------------------------
 * Returns a random float between the provided range, inclusive.
 */
float randomf(float min, float max) {
    if(min > max) swap(min, max);
    if(min == max) return min;
    return (float) rand() / ((float) RAND_MAX / (max - min)) + min;
}


/* ----------------------------------------------------------------------------
 * Returns a random integer between the provided range, inclusive.
 */
int randomi(int min, int max) {
    if(min > max) swap(min, max);
    if(min == max) return min;
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
 * Rotates a point by an angle. The x and y are meant to represent the difference between the point and the center of the rotation.
 */
void rotate_point(const float x, const float y, const float angle, float* final_x, float* final_y) {
    float c = cos(angle);
    float s = sin(angle);
    if(final_x) *final_x = c * x - s * y;
    if(final_y) *final_y = s * x + c * y;
}


/* ----------------------------------------------------------------------------
 * Saves the player's options.
 */
void save_options() {
    //TODO make this prettier. Like a list of constants somewhere where it associates an action with the name on the text file.
    ALLEGRO_FILE* file = al_fopen("Options.txt", "w");
    
    if(!file) return;
    
    //First, group the controls by action and player.
    map<string, string> grouped_controls;
    
    //Tell the map what they are.
    for(unsigned char p = 0; p < 4; ++p) {
        string prefix = "p" + i2s((p + 1)) + "_";
        grouped_controls[prefix + "punch"].clear();
        grouped_controls[prefix + "whistle"].clear();
        grouped_controls[prefix + "move_right"].clear();
        grouped_controls[prefix + "move_up"].clear();
        grouped_controls[prefix + "move_left"].clear();
        grouped_controls[prefix + "move_down"].clear();
        grouped_controls[prefix + "move_cursor_right"].clear();
        grouped_controls[prefix + "move_cursor_up"].clear();
        grouped_controls[prefix + "move_cursor_left"].clear();
        grouped_controls[prefix + "move_cursor_down"].clear();
        grouped_controls[prefix + "group_move_right"].clear();
        grouped_controls[prefix + "group_move_up"].clear();
        grouped_controls[prefix + "group_move_left"].clear();
        grouped_controls[prefix + "group_move_down"].clear();
        grouped_controls[prefix + "group_move_go_to_cursor"].clear();
        grouped_controls[prefix + "switch_leader_right"].clear();
        grouped_controls[prefix + "switch_leader_left"].clear();
        grouped_controls[prefix + "dismiss"].clear();
        grouped_controls[prefix + "use_spray_1"].clear();
        grouped_controls[prefix + "use_spray_2"].clear();
        grouped_controls[prefix + "use_spray"].clear();
        grouped_controls[prefix + "switch_spray_right"].clear();
        grouped_controls[prefix + "switch_spray_left"].clear();
        grouped_controls[prefix + "switch_zoom"].clear();
        grouped_controls[prefix + "zoom_in"].clear();
        grouped_controls[prefix + "zoom_out"].clear();
        grouped_controls[prefix + "switch_type_right"].clear();
        grouped_controls[prefix + "switch_type_left"].clear();
        grouped_controls[prefix + "switch_maturity_up"].clear();
        grouped_controls[prefix + "switch_maturity_down"].clear();
        grouped_controls[prefix + "lie_down"].clear();
        grouped_controls[prefix + "pause"].clear();
    }
    
    for(size_t p = 0; p < 4; p++) {
        size_t n_controls = controls[p].size();
        for(size_t c = 0; c < n_controls; ++c) {
            string name = "p" + i2s(p + 1) + "_";
            if(controls[p][c].action == BUTTON_THROW)                     name += "punch";
            else if(controls[p][c].action == BUTTON_WHISTLE)              name += "whistle";
            else if(controls[p][c].action == BUTTON_MOVE_RIGHT)           name += "move_right";
            else if(controls[p][c].action == BUTTON_MOVE_UP)              name += "move_up";
            else if(controls[p][c].action == BUTTON_MOVE_LEFT)            name += "move_left";
            else if(controls[p][c].action == BUTTON_MOVE_DOWN)            name += "move_down";
            else if(controls[p][c].action == BUTTON_MOVE_CURSOR_RIGHT)    name += "move_cursor_right";
            else if(controls[p][c].action == BUTTON_MOVE_CURSOR_UP)       name += "move_cursor_up";
            else if(controls[p][c].action == BUTTON_MOVE_CURSOR_LEFT)     name += "move_cursor_left";
            else if(controls[p][c].action == BUTTON_MOVE_CURSOR_DOWN)     name += "move_cursor_down";
            else if(controls[p][c].action == BUTTON_GROUP_MOVE_RIGHT)     name += "group_move_right";
            else if(controls[p][c].action == BUTTON_GROUP_MOVE_UP)        name += "group_move_up";
            else if(controls[p][c].action == BUTTON_GROUP_MOVE_LEFT)      name += "group_move_left";
            else if(controls[p][c].action == BUTTON_GROUP_MOVE_DOWN)      name += "group_move_down";
            else if(controls[p][c].action == BUTTON_GROUP_MOVE_GO_TO_CURSOR) name += "group_move_go_to_cursor";
            else if(controls[p][c].action == BUTTON_SWITCH_LEADER_RIGHT)  name += "switch_leader_right";
            else if(controls[p][c].action == BUTTON_SWITCH_LEADER_LEFT)   name += "switch_leader_left";
            else if(controls[p][c].action == BUTTON_DISMISS)              name += "dismiss";
            else if(controls[p][c].action == BUTTON_USE_SPRAY_1)          name += "use_spray_1";
            else if(controls[p][c].action == BUTTON_USE_SPRAY_2)          name += "use_spray_2";
            else if(controls[p][c].action == BUTTON_USE_SPRAY)            name += "use_spray";
            else if(controls[p][c].action == BUTTON_SWITCH_SPRAY_RIGHT)   name += "switch_spray_right";
            else if(controls[p][c].action == BUTTON_SWITCH_SPRAY_LEFT)    name += "switch_spray_left";
            else if(controls[p][c].action == BUTTON_SWITCH_ZOOM)          name += "switch_zoom";
            else if(controls[p][c].action == BUTTON_ZOOM_IN)              name += "zoom_in";
            else if(controls[p][c].action == BUTTON_ZOOM_OUT)             name += "zoom_out";
            else if(controls[p][c].action == BUTTON_SWITCH_TYPE_RIGHT)    name += "switch_type_right";
            else if(controls[p][c].action == BUTTON_SWITCH_TYPE_LEFT)     name += "switch_type_left";
            else if(controls[p][c].action == BUTTON_SWITCH_MATURITY_UP)   name += "switch_maturity_up";
            else if(controls[p][c].action == BUTTON_SWITCH_MATURITY_DOWN) name += "switch_maturity_down";
            else if(controls[p][c].action == BUTTON_LIE_DOWN)             name += "lie_down";
            else if(controls[p][c].action == BUTTON_PAUSE)                name += "pause";
            
            grouped_controls[name] += controls[p][c].stringify() + ",";
        }
    }
    
    //Save controls.
    for(auto c = grouped_controls.begin(); c != grouped_controls.end(); ++c) {
        if(c->second.size()) c->second.erase(c->second.size() - 1); //Remove the final character, which is always an extra comma.
        
        al_fwrite(file, c->first + "=" + c->second + "\n");
    }
    
    for(unsigned char p = 0; p < 4; ++p) {
        al_fwrite(file, "p" + i2s((p + 1)) + "_mouse_moves_cursor=" + b2s(mouse_moves_cursor[p]) + "\n");
    }
    
    //Other options.
    al_fwrite(file, "area_quality=" + f2s(area_images_scale) + "\n");
    al_fwrite(file, "daylight_effect=" + b2s(daylight_effect) + "\n");
    al_fwrite(file, "draw_cursor_trail=" + b2s(draw_cursor_trail) + "\n");
    al_fwrite(file, "fps=" + i2s(game_fps) + "\n");
    al_fwrite(file, "height=" + i2s(scr_h) + "\n");
    al_fwrite(file, "particle_quality=" + i2s(particle_quality) + "\n");
    al_fwrite(file, "pretty_whistle=" + b2s(pretty_whistle) + "\n");
    al_fwrite(file, "width=" + i2s(scr_w) + "\n");
    al_fwrite(file, "smooth_scaling=" + b2s(smooth_scaling) + "\n");
    al_fwrite(file, "window_pos_hack=" + b2s(window_pos_hack) + "\n");
    
    al_fclose(file);
}


/* ----------------------------------------------------------------------------
 * Sets a variable to a value, but only if its string is not-empty
 * value: The string with the value.
 * var:   The var to put it into. This is a string.
 */
void set_if_exists(const string &value, string &var) {
    if(value.empty()) return;
    var = value;
}


/* ----------------------------------------------------------------------------
 * Sets a variable to a value, but only if its string is not-empty
 * value: The string with the value.
 * var:   The var to put it into. This is an integer.
 */
void set_if_exists(const string &value, size_t &var) {
    if(value.empty()) return;
    var = s2i(value);
}


/* ----------------------------------------------------------------------------
 * Sets a variable to a value, but only if its string is not-empty
 * value: The string with the value.
 * var:   The var to put it into. This is a boolean.
 */
void set_if_exists(const string &value, bool &var) {
    if(value.empty()) return;
    var = s2b(value);
}


/* ----------------------------------------------------------------------------
 * Sets a variable to a value, but only if its string is not-empty
 * value: The string with the value.
 * var:   The var to put it into. This is a float.
 */
void set_if_exists(const string &value, float &var) {
    if(value.empty()) return;
    var = s2f(value);
}


/* ----------------------------------------------------------------------------
 * Returns a vector with all items inside a semicolon-separated list.
 */
vector<string> semicolon_list_to_vector(const string s) {
    vector<string> parts = split(s, ";");
    for(size_t p = 0; p < parts.size(); ++p) {
        parts[p] = trim_spaces(parts[p]);
    }
    return parts;
}


/* ----------------------------------------------------------------------------
 * Sets a variable to a value, but only if its string is not-empty
 * value: The string with the value.
 * var:   The var to put it into. This is an Allegro color.
 */
void set_if_exists(const string &value, ALLEGRO_COLOR &var) {
    if(value.empty()) return;
    var = s2c(value);
}


/* ----------------------------------------------------------------------------
 * Splits a string into several substrings, by the specified delimiter.
 * text:        The string to split.
 * del:         The delimiter. Default is space.
 * inc_empty:   If true, include empty substrings on the vector.
 ** i.e. if two delimiters come together in a row, keep an empty substring between.
 * inc_del:     If true, include the delimiters on the vector as a substring.
 */
vector<string> split(string text, const string &del, const bool inc_empty, const bool inc_del) {
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
                
            text.erase(text.begin(), text.begin() + pos + del_size);    //Delete everything before the delimiter, including the delimiter itself, and search again.
        }
    } while (pos != string::npos);
    
    //Text after the final delimiter. (If there is one. If not, it's just the whole string.)
    
    if (text != "" || inc_empty) //If it's a blank string, only add it if we want empty strings.
        v.push_back(text);
        
    return v;
}


/* ----------------------------------------------------------------------------
 * Returns whether a square intersects with a line segment.
 * Also returns true if the line is fully inside the square.
 * s**: Square coordinates.
 * l**: Line coordinates.
 */
bool square_intersects_line(const float sx1, const float sy1, const float sx2, const float sy2, const float lx1, const float ly1, const float lx2, const float ly2) {
    //Line crosses left side?
    if(lines_intersect(lx1, ly1, lx2, ly2, sx1, sy1, sx1, sy2, NULL, NULL)) return true;
    //Line crosses right side?
    if(lines_intersect(lx1, ly1, lx2, ly2, sx2, sy1, sx2, sy2, NULL, NULL)) return true;
    //Line crosses top side?
    if(lines_intersect(lx1, ly1, lx2, ly2, sx1, sy1, sx2, sy1, NULL, NULL)) return true;
    //Line crosses bottom side?
    if(lines_intersect(lx1, ly1, lx2, ly2, sx1, sy2, sx2, sy2, NULL, NULL)) return true;
    
    if(
        (lx1 >= sx1 && lx2 >= sx1) &&
        (lx1 <= sx2 && lx2 <= sx2) &&
        (ly1 >= sy1 && ly2 >= sy1) &&
        (ly1 <= sy2 && ly2 <= sy2)
    ) return true;
    
    return false;
    
}


/* ----------------------------------------------------------------------------
 * Starts the display of a text message. If the text is empty, it closes the message box.
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
    cur_message_stopping_chars.push_back(0); //First character. Makes it easier.
    cur_message_section = 0;
    
    vector<string> lines = split(text, "\n");
    size_t char_count = 0;
    for(size_t l = 0; l < lines.size(); ++l) {
        char_count += lines[l].size() + 1; //+1 because of the new line character.
        if((l + 1) % 3 == 0) cur_message_stopping_chars.push_back(char_count);
    }
    
    if(cur_message_stopping_chars.size() > 1) {
        cur_message_stopping_chars.back()--; //Remove one because the last line doesn't have a new line character. Even if it does, it's invisible.
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
 * Unloads hazards loaded in memory.
 */
void unload_hazards() {
    hazards.clear();
}


/* ----------------------------------------------------------------------------
 * Unloads status types loaded in memory.
 */
void unload_status_types() {
    status_types.clear();
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


//Converts a string to a boolean, judging by the English language words that represent true and false.
bool s2b(const string &s) {
    string s2 = s;
    s2 = str_to_lower(s2);
    s2 = trim_spaces(s2);
    if(s2 == "yes" || s2 == "true" || s2 == "y" || s2 == "t") return true;
    else return (s2i(s2) != 0);
}


//Converts a string to an Allegro color. Components are separated by spaces, and the final one (alpha) is optional.
ALLEGRO_COLOR s2c(const string &s) {
    string s2 = s;
    s2 = trim_spaces(s2);
    
    unsigned char alpha = 255;
    vector<string> components = split(s2);
    if(components.size() >= 2) alpha = s2i(components[1]);
    
    if(s2 == "nothing" || s2 == "none") return al_map_rgba(0,   0,   0,   0);
    if(s2 == "black")                   return al_map_rgba(0,   0,   0,   alpha);
    if(s2 == "gray" || s2 == "grey")    return al_map_rgba(128, 128, 128, alpha);
    if(s2 == "white")                   return map_alpha(alpha);
    if(s2 == "yellow")                  return al_map_rgba(255, 255, 0,   alpha);
    if(s2 == "orange")                  return al_map_rgba(255, 128, 0,   alpha);
    if(s2 == "brown")                   return al_map_rgba(128, 64,  0,   alpha);
    if(s2 == "red")                     return al_map_rgba(255, 0,   0,   alpha);
    if(s2 == "violet")                  return al_map_rgba(255, 0,   255, alpha);
    if(s2 == "purple")                  return al_map_rgba(128, 0,   255, alpha);
    if(s2 == "blue")                    return al_map_rgba(0,   0,   255, alpha);
    if(s2 == "cyan")                    return al_map_rgba(0,   255, 255, alpha);
    if(s2 == "green")                   return al_map_rgba(0,   255, 0,   alpha);
    
    ALLEGRO_COLOR c =
        al_map_rgba(
            ((components.size() > 0) ? s2i(components[0]) : 0),
            ((components.size() > 1) ? s2i(components[1]) : 0),
            ((components.size() > 2) ? s2i(components[2]) : 0),
            ((components.size() > 3) ? s2i(components[3]) : 255)
        );
    return c;
}


//Converts a string to a float, trimming the spaces and accepting commas or points.
double s2f(const string &s) { string s2 = trim_spaces(s); replace(s2.begin(), s2.end(), ',', '.'); return atof(s2.c_str()); }


//Converts a string to an integer.
int s2i(const string &s) { return s2f(s); }
