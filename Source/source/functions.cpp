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

#include "editors/animation_editor.h"
#include "editors/area_editor.h"
#include "const.h"
#include "data_file.h"
#include "drawing.h"
#include "functions.h"
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
 * Clears the textures of the area's sectors from memory.
 */
void clear_area_textures() {
    for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
        sector* s_ptr = cur_area_data.sectors[s];
        if(
            s_ptr->texture_info.bitmap &&
            s_ptr->texture_info.bitmap != bmp_error
        ) {
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
void coordinates_to_angle(
    const float x_coord, const float y_coord, float* angle, float* magnitude
) {
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
    
    ALLEGRO_FS_ENTRY* folder = NULL;
    folder = al_create_fs_entry(folder_name.c_str());
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
            area_images[x].push_back(
                al_create_bitmap(area_image_size, area_image_size)
            );
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
        unsigned int sector_start_col, sector_end_col;
        unsigned int sector_start_row, sector_end_row;
        get_sector_bounding_box(s_ptr, &s_min_x, &s_min_y, &s_max_x, &s_max_y);
        
        s_min_x *= area_images_scale;
        s_max_x *= area_images_scale;
        s_min_y *= area_images_scale;
        s_max_y *= area_images_scale;
        
        sector_start_col =
            (s_min_x - area_images_x1) / area_image_size;
        sector_end_col =
            ceil((s_max_x - area_images_x1) / area_image_size) - 1;
        sector_start_row =
            (s_min_y - area_images_y1) / area_image_size;
        sector_end_row =
            ceil((s_max_y - area_images_y1) / area_image_size) - 1;
            
        al_set_separate_blender(
            ALLEGRO_ADD, ALLEGRO_ALPHA,
            ALLEGRO_INVERSE_ALPHA, ALLEGRO_ADD,
            ALLEGRO_ONE, ALLEGRO_INVERSE_ALPHA
        );
        
        for(size_t x = sector_start_col; x <= sector_end_col; ++x) {
            for(size_t y = sector_start_row; y <= sector_end_row; ++y) {
                ALLEGRO_BITMAP* prev_target_bmp = al_get_target_bitmap();
                al_set_target_bitmap(area_images[x][y]); {
                
                    draw_sector(
                        cur_area_data.sectors[s],
                        (x * area_image_size + area_images_x1) /
                        area_images_scale,
                        (y * area_image_size + area_images_y1) /
                        area_images_scale,
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
    
    dist closest_mob_to_cursor_dist = 0;
    mob* closest_mob_to_cursor = NULL;
    
    for(size_t m = 0; m < mobs.size(); ++m) {
        mob* m_ptr = mobs[m];
        
        if(!m_ptr->fsm.cur_state) continue;
        
        dist d = dist(mx, my, m_ptr->x, m_ptr->y);
        if(!closest_mob_to_cursor || d < closest_mob_to_cursor_dist) {
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
 * Returns an ALLEGRO_TRANSFORM that transforms world coordinates
 * into screen coordinates.
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
ALLEGRO_COLOR interpolate_color(
    const float n, const float n1, const float n2,
    const ALLEGRO_COLOR &c1, const ALLEGRO_COLOR &c2
) {
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
float interpolate_number(
    const float p, const float p1, const float p2,
    const float v1, const float v2
) {
    return v1 + ((p - p1) / (float) (p2 - p1)) * (v2 - v1);
}


/* ----------------------------------------------------------------------------
 * Loads an area into memory.
 * name:            Name of the area's folder.
 * load_for_editor: If true, skips loading some things that the area editor
   * won't need.
 * from_backup:     If true, load from a backup, if any.
 */
void load_area(
    const string &name, const bool load_for_editor, const bool from_backup
) {

    cur_area_data.clear();
    
    data_node data_file =
        load_data_file(AREAS_FOLDER_PATH + "/" + name + "/Data.txt");
        
    cur_area_data.name =
        data_file.get_child_by_name("name")->get_value_or_default(name);
    cur_area_data.subtitle =
        data_file.get_child_by_name("subtitle")->value;
        
    if(loading_text_bmp) al_destroy_bitmap(loading_text_bmp);
    if(loading_subtext_bmp) al_destroy_bitmap(loading_subtext_bmp);
    loading_text_bmp = NULL;
    loading_subtext_bmp = NULL;
    
    draw_loading_screen(cur_area_data.name, cur_area_data.subtitle, 1.0);
    al_flip_display();
    
    cur_area_data.weather_name = data_file.get_child_by_name("weather")->value;
    if(!load_for_editor) {
        if(
            weather_conditions.find(cur_area_data.weather_name) ==
            weather_conditions.end()
        ) {
            log_error(
                "Area " + name +
                " refers to a non-existing weather condition, \"" +
                cur_area_data.weather_name + "\"!",
                &data_file
            );
            cur_area_data.weather_condition =
                weather();
                
        } else {
            cur_area_data.weather_condition =
                weather_conditions[cur_area_data.weather_name];
        }
    }
    
    cur_area_data.bg_bmp_file_name =
        data_file.get_child_by_name("bg_bmp")->value;
    if(!load_for_editor && !cur_area_data.bg_bmp_file_name.empty()) {
        cur_area_data.bg_bmp =
            bitmaps.get(cur_area_data.bg_bmp_file_name, &data_file);
    }
    cur_area_data.bg_color =
        s2c(data_file.get_child_by_name("bg_color")->value);
    cur_area_data.bg_dist =
        s2f(data_file.get_child_by_name("bg_dist")->get_value_or_default("2"));
    cur_area_data.bg_bmp_zoom =
        s2f(data_file.get_child_by_name("bg_zoom")->get_value_or_default("1"));
        
        
    data_node geometry_file =
        load_data_file(
            AREAS_FOLDER_PATH + "/" + name +
            (from_backup ? "/Geometry_backup.txt" : "/Geometry.txt")
        );
        
    //Vertexes.
    size_t n_vertexes =
        geometry_file.get_child_by_name(
            "vertexes"
        )->get_nr_of_children_by_name("v");
    for(size_t v = 0; v < n_vertexes; ++v) {
        data_node* vertex_data =
            geometry_file.get_child_by_name(
                "vertexes"
            )->get_child_by_name("v", v);
        vector<string> words = split(vertex_data->value);
        if(words.size() == 2) {
            cur_area_data.vertexes.push_back(
                new vertex(s2f(words[0]), s2f(words[1]))
            );
        }
    }
    
    //Edges.
    size_t n_edges =
        geometry_file.get_child_by_name(
            "edges"
        )->get_nr_of_children_by_name("e");
    for(size_t e = 0; e < n_edges; ++e) {
        data_node* edge_data =
            geometry_file.get_child_by_name(
                "edges"
            )->get_child_by_name("e", e);
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
    size_t n_sectors =
        geometry_file.get_child_by_name(
            "sectors"
        )->get_nr_of_children_by_name("s");
    for(size_t s = 0; s < n_sectors; ++s) {
        data_node* sector_data =
            geometry_file.get_child_by_name(
                "sectors"
            )->get_child_by_name("s", s);
        sector* new_sector = new sector();
        
        new_sector->type =
            sector_types.get_nr(sector_data->get_child_by_name("type")->value);
        if(new_sector->type == 255) new_sector->type = SECTOR_TYPE_NORMAL;
        new_sector->brightness =
            s2f(
                sector_data->get_child_by_name(
                    "brightness"
                )->get_value_or_default(i2s(DEF_SECTOR_BRIGHTNESS))
            );
        new_sector->tag = sector_data->get_child_by_name("tag")->value;
        new_sector->z = s2f(sector_data->get_child_by_name("z")->value);
        new_sector->fade = s2b(sector_data->get_child_by_name("fade")->value);
        new_sector->always_cast_shadow =
            s2b(
                sector_data->get_child_by_name("always_cast_shadow")->value
            );
            
        new_sector->texture_info.file_name =
            sector_data->get_child_by_name("texture")->value;
        new_sector->texture_info.rot =
            s2f(sector_data->get_child_by_name("texture_rotate")->value);
            
        vector<string> scales =
            split(sector_data->get_child_by_name("texture_scale")->value);
        if(scales.size() >= 2) {
            new_sector->texture_info.scale_x = s2f(scales[0]);
            new_sector->texture_info.scale_y = s2f(scales[0]);
        }
        vector<string> translations =
            split(sector_data->get_child_by_name("texture_trans")->value);
        if(translations.size() >= 2) {
            new_sector->texture_info.trans_x = s2f(translations[0]);
            new_sector->texture_info.trans_y = s2f(translations[1]);
        }
        new_sector->texture_info.tint =
            s2c(
                sector_data->get_child_by_name("texture_tint")->
                get_value_or_default("255 255 255")
            );
            
        data_node* hazards_node = sector_data->get_child_by_name("hazards");
        vector<string> hazards_strs =
            semicolon_list_to_vector(hazards_node->value);
        for(size_t h = 0; h < hazards_strs.size(); ++h) {
            string hazard_name = hazards_strs[h];
            if(hazards.find(hazard_name) == hazards.end()) {
                log_error(
                    "Unknown hazard \"" + hazard_name +
                    "\"!", hazards_node
                );
            } else {
                new_sector->hazards.push_back(&(hazards[hazard_name]));
                if(hazards[hazard_name].associated_liquid) {
                    new_sector->associated_liquid =
                        hazards[hazard_name].associated_liquid;
                }
            }
        }
        new_sector->hazards_str = hazards_node->value;
        new_sector->hazard_floor =
            s2b(
                sector_data->get_child_by_name(
                    "hazards_floor"
                )->get_value_or_default("true")
            );
            
        cur_area_data.sectors.push_back(new_sector);
    }
    
    //Mobs.
    size_t n_mobs =
        geometry_file.get_child_by_name("mobs")->get_nr_of_children();
    for(size_t m = 0; m < n_mobs; ++m) {
    
        data_node* mob_node =
            geometry_file.get_child_by_name("mobs")->get_child(m);
            
        mob_gen* mob_ptr = new mob_gen();
        
        vector<string> coords = split(mob_node->get_child_by_name("p")->value);
        mob_ptr->x = (coords.size() >= 1 ? s2f(coords[0]) : 0);
        mob_ptr->y = (coords.size() >= 2 ? s2f(coords[1]) : 0);
        mob_ptr->angle =
            s2f(
                mob_node->get_child_by_name("angle")->get_value_or_default("0")
            );
        mob_ptr->vars = mob_node->get_child_by_name("vars")->value;
        
        mob_ptr->category = mob_categories.get_nr_from_sname(mob_node->name);
        string mt = mob_node->get_child_by_name("type")->value;
        mob_categories.set_mob_type_ptr(mob_ptr, mt);
        
        bool problem = false;
        
        if(!mob_ptr->type && !load_for_editor) {
            //Error.
            log_error(
                "Unknown \"" + mob_categories.get_sname(mob_ptr->category) +
                "\" mob type \"" +
                mt + "\"!",
                mob_node
            );
            problem = true;
        }
        
        if(
            (
                mob_ptr->category == MOB_CATEGORY_NONE ||
                mob_ptr->category == 255
            ) && !load_for_editor
        ) {
        
            log_error(
                "Unknown mob category \"" + mob_node->name + "\"!", mob_node
            );
            mob_ptr->category = MOB_CATEGORY_NONE;
            problem = true;
            
        }
        
        if(!problem) cur_area_data.mob_generators.push_back(mob_ptr);
    }
    
    //Path stops.
    size_t n_stops =
        geometry_file.get_child_by_name("path_stops")->get_nr_of_children();
    for(size_t s = 0; s < n_stops; ++s) {
    
        data_node* path_stop_node =
            geometry_file.get_child_by_name("path_stops")->get_child(s);
            
        path_stop* s_ptr = new path_stop();
        
        vector<string> words =
            split(path_stop_node->get_child_by_name("pos")->value);
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
    size_t n_shadows =
        geometry_file.get_child_by_name("tree_shadows")->get_nr_of_children();
    for(size_t s = 0; s < n_shadows; ++s) {
    
        data_node* shadow_node =
            geometry_file.get_child_by_name("tree_shadows")->get_child(s);
            
        tree_shadow* s_ptr = new tree_shadow();
        
        vector<string> words =
            split(shadow_node->get_child_by_name("pos")->value);
        s_ptr->x = (words.size() >= 1 ? s2f(words[0]) : 0);
        s_ptr->y = (words.size() >= 2 ? s2f(words[1]) : 0);
        
        words = split(shadow_node->get_child_by_name("size")->value);
        s_ptr->w = (words.size() >= 1 ? s2f(words[0]) : 0);
        s_ptr->h = (words.size() >= 2 ? s2f(words[1]) : 0);
        
        s_ptr->angle =
            s2f(
                shadow_node->get_child_by_name(
                    "angle"
                )->get_value_or_default("0")
            );
        s_ptr->alpha =
            s2i(
                shadow_node->get_child_by_name(
                    "alpha"
                )->get_value_or_default("255")
            );
        s_ptr->file_name = shadow_node->get_child_by_name("file")->value;
        s_ptr->bitmap = bitmaps.get("Textures/" + s_ptr->file_name, NULL);
        
        words = split(shadow_node->get_child_by_name("sway")->value);
        s_ptr->sway_x = (words.size() >= 1 ? s2f(words[0]) : 0);
        s_ptr->sway_y = (words.size() >= 2 ? s2f(words[1]) : 0);
        
        if(s_ptr->bitmap == bmp_error && !load_for_editor) {
            log_error(
                "Unknown tree shadow texture \"" + s_ptr->file_name + "\"!",
                shadow_node
            );
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
    if(!load_for_editor) {
        //Fade sectors that also fade brightness should be
        //at midway between the two neighbors.
        for(size_t s = 0; s < cur_area_data.sectors.size(); ++s) {
            sector* s_ptr = cur_area_data.sectors[s];
            if(s_ptr->fade) {
                sector* n1 = NULL;
                sector* n2 = NULL;
                s_ptr->get_texture_merge_sectors(&n1, &n2);
                if(n1 && n2) {
                    s_ptr->brightness = (n1->brightness + n2->brightness) / 2;
                }
            }
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
        ae->set_guide_file_name(
            geometry_file.get_child_by_name("guide_file_name")->value
        );
        ae->set_guide_x(
            s2f(geometry_file.get_child_by_name("guide_x")->value)
        );
        ae->set_guide_y(
            s2f(geometry_file.get_child_by_name("guide_y")->value)
        );
        ae->set_guide_w(
            s2f(geometry_file.get_child_by_name("guide_w")->value)
        );
        ae->set_guide_h(
            s2f(geometry_file.get_child_by_name("guide_h")->value)
        );
        ae->set_guide_a(
            s2i(
                geometry_file.get_child_by_name(
                    "guide_alpha"
                )->get_value_or_default("255")
            )
        );
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
                s_ptr->texture_info.bitmap =
                    bitmaps.get(
                        "Textures/" + s_ptr->texture_info.file_name, NULL
                    );
            }
        }
    }
}


/* ----------------------------------------------------------------------------
 * Loads a bitmap from the game's content.
 * If the node is present, it'll be used to report errors.
 */
ALLEGRO_BITMAP* load_bmp(
    const string &file_name, data_node* node, bool report_error
) {
    if(file_name.empty()) return NULL;
    ALLEGRO_BITMAP* b =
        al_load_bitmap((GRAPHICS_FOLDER_PATH + "/" + file_name).c_str());
        
    if(!b && report_error) {
        log_error("Could not open image " + file_name + "!", node);
        b = bmp_error;
    }
    
    return b;
}


/* ----------------------------------------------------------------------------
 * Loads a game control.
 */
void load_control(
    const unsigned char action, const unsigned char player,
    const string &name, data_node &file, const string &def
) {
    string s =
        file.get_child_by_name(
            "p" + i2s((player + 1)) + "_" + name
        )->get_value_or_default((player == 0) ? def : "");
    vector<string> possible_controls = split(s, ";");
    size_t n_possible_controls = possible_controls.size();
    
    for(size_t c = 0; c < n_possible_controls; ++c) {
        controls[player].push_back(control_info(action, possible_controls[c]));
    }
}


/* ----------------------------------------------------------------------------
 * Loads the user-made particle generators.
 */
void load_custom_particle_generators() {
    custom_particle_generators.clear();
    
    data_node file(PARTICLE_GENERATORS_FILE);
    
    size_t n_pg = file.get_nr_of_children();
    for(size_t pg = 0; pg < n_pg; ++pg) {
    
        data_node* pg_node = file.get_child(pg);
        data_node* p_node = pg_node->get_child_by_name("base");
        
        reader_setter grs(pg_node);
        reader_setter prs(p_node);
        
        float emission_interval;
        size_t number;
        string bitmap_name;
        particle base_p;
        base_p.priority = PARTICLE_PRIORITY_MEDIUM;
        
        grs.set("emission_interval", emission_interval);
        grs.set("number", number);
        
        prs.set("bitmap", bitmap_name);
        base_p.bitmap =
            bitmaps.get(bitmap_name, p_node->get_child_by_name("bitmap"));
        if(base_p.bitmap) {
            base_p.type = PARTICLE_TYPE_BITMAP;
        } else {
            base_p.type = PARTICLE_TYPE_CIRCLE;
        }
        prs.set("duration",        base_p.duration);
        prs.set("friction",        base_p.friction);
        prs.set("gravity",         base_p.gravity);
        prs.set("size_grow_speed", base_p.size_grow_speed);
        prs.set("size",            base_p.size);
        prs.set("speed_x",         base_p.speed_x);
        prs.set("speed_y",         base_p.speed_y);
        prs.set("color",           base_p.color);
        prs.set("before_mobs",     base_p.before_mobs);
        base_p.time = base_p.duration;
        
        particle_generator pg_struct(emission_interval, base_p, number);
        
        grs.set("number_deviation",   pg_struct.number_deviation);
        grs.set("duration_deviation", pg_struct.duration_deviation);
        grs.set("friction_deviation", pg_struct.friction_deviation);
        grs.set("gravity_deviation",  pg_struct.gravity_deviation);
        grs.set("size_deviation",     pg_struct.size_deviation);
        grs.set("x_deviation",        pg_struct.x_deviation);
        grs.set("y_deviation",        pg_struct.y_deviation);
        grs.set("speed_x_deviation",  pg_struct.speed_x_deviation);
        grs.set("speed_y_deviation",  pg_struct.speed_y_deviation);
        grs.set("angle",              pg_struct.angle);
        grs.set("angle_deviation",    pg_struct.angle_deviation);
        grs.set("speed",              pg_struct.speed);
        grs.set("speed_deviation",    pg_struct.speed_deviation);
        
        pg_struct.id = MOB_PARTICLE_GENERATOR_STATUS + pg;
        
        custom_particle_generators[pg_node->name] = pg_struct;
    }
}


/* ----------------------------------------------------------------------------
 * Loads a data file from the game's content.
 */
data_node load_data_file(const string &file_name) {
    data_node n = data_node(file_name);
    if(!n.file_was_opened) {
        log_error("Could not open data file " + file_name + "!");
    }
    
    return n;
}


/* ----------------------------------------------------------------------------
 * Loads the game's configuration file.
 */
void load_game_config() {
    data_node file(CONFIG_FOLDER_PATH);
    
    reader_setter rs(&file);
    
    rs.set("game_name", game_name);
    rs.set("game_version", game_version);
    
    rs.set("carrying_color_move", carrying_color_move);
    rs.set("carrying_color_stop", carrying_color_stop);
    rs.set("carrying_speed_base_mult", carrying_speed_base_mult);
    rs.set("carrying_speed_max_mult", carrying_speed_max_mult);
    rs.set("carrying_speed_weight_mult", carrying_speed_weight_mult);
    
    rs.set("day_minutes_start", day_minutes_start);
    rs.set("day_minutes_end", day_minutes_end);
    rs.set("day_minutes_per_irl_sec", day_minutes_per_irl_sec);
    
    rs.set("idle_task_range", idle_task_range);
    rs.set("group_move_task_range", group_move_task_range);
    rs.set("max_pikmin_in_field", max_pikmin_in_field);
    rs.set("maturity_power_mult", maturity_power_mult);
    rs.set("maturity_speed_mult", maturity_speed_mult);
    rs.set("nectar_amount", nectar_amount);
    
    rs.set("cursor_max_dist", cursor_max_dist);
    rs.set("cursor_spin_speed", cursor_spin_speed);
    rs.set("next_pluck_range", next_pluck_range);
    rs.set("onion_open_range", onion_open_range);
    rs.set("pikmin_grab_range", pikmin_grab_range);
    rs.set("pluck_range", pluck_range);
    rs.set("whistle_growth_speed", whistle_growth_speed);
    
    rs.set("info_spot_trigger_range", info_spot_trigger_range);
    rs.set("message_char_interval", message_char_interval);
    rs.set("zoom_max_level", zoom_max_level);
    rs.set("zoom_min_level", zoom_min_level);
    
    al_set_window_title(display, game_name.c_str());
}


/* ----------------------------------------------------------------------------
 * Loads all of the game's content.
 */
void load_game_content() {
    load_custom_particle_generators();
    load_liquids();
    load_status_types();
    load_spray_types();
    load_hazards();
    
    //Mob types.
    load_mob_types(true);
    
    //Weather.
    weather_conditions.clear();
    data_node weather_file = load_data_file(WEATHER_FILE);
    size_t n_weather_conditions =
        weather_file.get_nr_of_children_by_name("weather");
        
    for(size_t wc = 0; wc < n_weather_conditions; ++wc) {
        data_node* cur_weather = weather_file.get_child_by_name("weather", wc);
        
        string name = cur_weather->get_child_by_name("name")->value;
        if(name.empty()) name = "default";
        
        //Lighting.
        vector<pair<size_t, ALLEGRO_COLOR> > lighting;
        size_t n_lighting_points =
            cur_weather->get_child_by_name("lighting")->get_nr_of_children();
            
        bool have_midnight = false;
        
        for(size_t lp = 0; lp < n_lighting_points; ++lp) {
            data_node* lighting_node =
                cur_weather->get_child_by_name("lighting")->get_child(lp);
                
            size_t point_time = s2i(lighting_node->name);
            ALLEGRO_COLOR point_color = s2c(lighting_node->value);
            
            lighting.push_back(make_pair(point_time, point_color));
            
            if(point_time == 24 * 60) have_midnight = true;
        }
        
        sort(
            lighting.begin(), lighting.end(),
            [] (
                pair<size_t, ALLEGRO_COLOR> p1, pair<size_t, ALLEGRO_COLOR> p2
        ) -> bool {
            return p1.first < p2.first;
        }
        );
        
        if(lighting.empty()) {
            log_error("Weather condition " + name + " has no lighting!");
        } else {
            if(!have_midnight) {
                //If there is no data for the last hour,
                //use the data from the first point
                //(this is because the day loops after 24:00;
                //needed for interpolation).
                lighting.push_back(make_pair(24 * 60, lighting[0].second));
            }
        }
        
        //Sun's strength.
        vector<pair<size_t, unsigned char> > sun_strength;
        size_t n_sun_strength_points =
            cur_weather->get_child_by_name(
                "sun_strength"
            )->get_nr_of_children();
            
        have_midnight = false;
        
        for(size_t sp = 0; sp < n_sun_strength_points; ++sp) {
            data_node* sun_strength_node =
                cur_weather->get_child_by_name("sun_strength")->get_child(sp);
                
            size_t point_time = s2i(sun_strength_node->name);
            unsigned char point_strength = s2i(sun_strength_node->value);
            
            sun_strength.push_back(make_pair(point_time, point_strength));
            
            if(point_time == 24 * 60) have_midnight = true;
        }
        
        sort(
            sun_strength.begin(), sun_strength.end(),
            [] (
                pair<size_t, unsigned char> p1,
                pair<size_t, unsigned char> p2
        ) -> bool {
            return p1.first < p2.first;
        }
        );
        
        if(!sun_strength.empty()) {
            if(!have_midnight) {
                //If there is no data for the last hour,
                //use the data from the first point
                //(this is because the day loops after 24:00;
                //needed for interpolation).
                sun_strength.push_back(
                    make_pair(24 * 60, sun_strength[0].second)
                );
            }
        }
        
        //Precipitation.
        unsigned char precipitation_type =
            s2i(
                cur_weather->get_child_by_name(
                    "precipitation_type"
                )->get_value_or_default(i2s(PRECIPITATION_TYPE_NONE))
            );
        interval precipitation_frequency =
            interval(
                cur_weather->get_child_by_name(
                    "precipitation_frequency"
                )->value
            );
        interval precipitation_speed =
            interval(
                cur_weather->get_child_by_name(
                    "precipitation_speed"
                )->value
            );
        interval precipitation_angle =
            interval(
                cur_weather->get_child_by_name(
                    "precipitation_angle"
                )->get_value_or_default(f2s((M_PI + M_PI_2)))
            );
            
        //Save.
        weather_conditions[name] =
            weather(
                name, lighting, sun_strength,
                precipitation_type, precipitation_frequency,
                precipitation_speed, precipitation_angle
            );
    }
}


/* ----------------------------------------------------------------------------
 *  Loads HUD coordinates from a file.
 */
void load_hud_coordinates() {
    data_node file = data_node(MISC_FOLDER_PATH + "/HUD.txt");
    if(!file.file_was_opened) return;
    
#define loader(id, name) \
    load_hud_coordinates(id, file.get_child_by_name(name)->value)
    
    loader(HUD_ITEM_TIME,                "time");
    loader(HUD_ITEM_DAY_BUBBLE,          "day_bubble");
    loader(HUD_ITEM_DAY_NUMBER,          "day_number");
    loader(HUD_ITEM_LEADER_1_ICON,       "leader_1_icon");
    loader(HUD_ITEM_LEADER_2_ICON,       "leader_2_icon");
    loader(HUD_ITEM_LEADER_3_ICON,       "leader_3_icon");
    loader(HUD_ITEM_LEADER_1_HEALTH,     "leader_1_health");
    loader(HUD_ITEM_LEADER_2_HEALTH,     "leader_2_health");
    loader(HUD_ITEM_LEADER_3_HEALTH,     "leader_3_health");
    loader(HUD_ITEM_PIKMIN_STANDBY_ICON, "pikmin_standby_icon");
    loader(HUD_ITEM_PIKMIN_STANDBY_NR,   "pikmin_standby_nr");
    loader(HUD_ITEM_PIKMIN_STANDBY_X,    "pikmin_standby_x");
    loader(HUD_ITEM_PIKMIN_GROUP_NR,     "pikmin_group_nr");
    loader(HUD_ITEM_PIKMIN_FIELD_NR,     "pikmin_field_nr");
    loader(HUD_ITEM_PIKMIN_TOTAL_NR,     "pikmin_total_nr");
    loader(HUD_ITEM_PIKMIN_SLASH_1,      "pikmin_slash_1");
    loader(HUD_ITEM_PIKMIN_SLASH_2,      "pikmin_slash_2");
    loader(HUD_ITEM_PIKMIN_SLASH_3,      "pikmin_slash_3");
    loader(HUD_ITEM_SPRAY_1_ICON,        "spray_1_icon");
    loader(HUD_ITEM_SPRAY_1_AMOUNT,      "spray_1_amount");
    loader(HUD_ITEM_SPRAY_1_KEY,         "spray_1_key");
    loader(HUD_ITEM_SPRAY_2_ICON,        "spray_2_icon");
    loader(HUD_ITEM_SPRAY_2_AMOUNT,      "spray_2_amount");
    loader(HUD_ITEM_SPRAY_2_KEY,         "spray_2_key");
    loader(HUD_ITEM_SPRAY_PREV_ICON,     "spray_prev_icon");
    loader(HUD_ITEM_SPRAY_PREV_KEY,      "spray_prev_key");
    loader(HUD_ITEM_SPRAY_NEXT_ICON,     "spray_next_icon");
    loader(HUD_ITEM_SPRAY_NEXT_KEY,      "spray_next_key");
    
#undef loader
    
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
    data_node file = data_node(MISC_FOLDER_PATH + "/Hazards.txt");
    if(!file.file_was_opened) return;
    
    size_t n_hazards = file.get_nr_of_children();
    for(size_t h = 0; h < n_hazards; ++h) {
        data_node* h_node = file.get_child(h);
        hazard h_struct;
        
        h_struct.name = h_node->name;
        
        data_node* effects_node = h_node->get_child_by_name("effects");
        vector<string> effects_strs =
            semicolon_list_to_vector(effects_node->value);
        for(size_t e = 0; e < effects_strs.size(); ++e) {
            string effect_name = effects_strs[e];
            if(status_types.find(effect_name) == status_types.end()) {
                log_error(
                    "Unknown status effect \"" + effect_name + "\"!",
                    effects_node
                );
            } else {
                h_struct.effects.push_back(&(status_types[effect_name]));
            }
        }
        data_node* l_node = h_node->get_child_by_name("liquid");
        if(!l_node->value.empty()) {
            if(liquids.find(l_node->value) == liquids.end()) {
                log_error(
                    "Liquid \"" + l_node->value + "\" not found!",
                    l_node
                );
            } else {
                h_struct.associated_liquid = &(liquids[l_node->value]);
            }
        }
        
        reader_setter(h_node).set("color", h_struct.main_color);
        
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
 * Loads the liquids from the game data.
 */
void load_liquids() {
    data_node file = data_node(MISC_FOLDER_PATH + "/Liquids.txt");
    if(!file.file_was_opened) return;
    
    map<string, data_node*> nodes;
    
    size_t n_liquids = file.get_nr_of_children();
    for(size_t l = 0; l < n_liquids; ++l) {
        data_node* l_node = file.get_child(l);
        liquid l_struct;
        
        l_struct.name = l_node->name;
        reader_setter rs(l_node);
        rs.set("color", l_struct.main_color);
        rs.set("surface_1_speed", l_struct.surface_speed[0]);
        rs.set("surface_2_speed", l_struct.surface_speed[0]);
        rs.set("surface_alpha", l_struct.surface_alpha);
        
        liquids[l_node->name] = l_struct;
        nodes[l_node->name] = l_node;
    }
    
    for(auto l = liquids.begin(); l != liquids.end(); ++l) {
        data_node anim_file(
            ANIMATIONS_FOLDER_PATH + "/" +
            nodes[l->first]->get_child_by_name("animation")->value
        );
        l->second.anim_pool = load_animation_database_from_file(&anim_file);
        if(!l->second.anim_pool.animations.empty()) {
            l->second.anim_instance = animation_instance(&l->second.anim_pool);
            l->second.anim_instance.cur_anim = l->second.anim_pool.animations[0];
            l->second.anim_instance.start();
        }
    }
}


/* ----------------------------------------------------------------------------
 * Loads the player's options.
 */
void load_options() {
    for(size_t h = 0; h < ANIMATION_EDITOR_HISTORY_SIZE; ++h) {
        animation_editor_history.push_back("");
    }
    
    data_node file = data_node("Options.txt");
    if(!file.file_was_opened) return;
    
    //Init joysticks.
    joystick_numbers.clear();
    int n_joysticks = al_get_num_joysticks();
    for(int j = 0; j < n_joysticks; ++j) {
        joystick_numbers[al_get_joystick(j)] = j;
    }
    
    /* Load controls.
     * Format of a control:
     * "p<player>_<action>=<possible control 1>,<possible control 2>,<...>"
     * Format of a possible control:
     * "<input method>_<parameters, underscore separated>"
     * Input methods:
     * "k" (keyboard key), "mb" (mouse button),
     * "mwu" (mouse wheel up), "mwd" (down),
     * "mwl" (left), "mwr" (right), "jb" (joystick button),
     * "jap" (joystick axis, positive), "jan" (joystick axis, negative).
     * The parameters are the key/button number, joystick number,
     * joystick stick and axis, etc.
     * Check the constructor of control_info for more information.
     */
    for(unsigned char p = 0; p < 4; ++p) {
        controls[p].clear();
        for(size_t b = 0; b < N_BUTTONS; ++b) {
            string option_name = buttons.list[b].option_name;
            if(option_name.empty()) continue;
            load_control(buttons.list[b].id, p, option_name, file);
        }
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
        mouse_moves_cursor[p] =
            s2b(
                file.get_child_by_name(
                    "p" + i2s((p + 1)) + "_mouse_moves_cursor"
                )->get_value_or_default((p == 0) ? "true" : "false")
            );
    }
    
    //Other options.
    reader_setter rs(&file);
    string resolution_str;
    rs.set("area_quality", area_images_scale);
    rs.set("draw_cursor_trail", draw_cursor_trail);
    rs.set("editor_backup_interval", editor_backup_interval);
    rs.set("fps", game_fps);
    rs.set("max_particles", max_particles);
    rs.set("middle_zoom_level", zoom_mid_level);
    rs.set("pretty_whistle", pretty_whistle);
    rs.set("resolution", resolution_str);
    rs.set("smooth_scaling", smooth_scaling);
    rs.set("window_position_hack", window_position_hack);
    game_fps = max(1, game_fps);
    
    vector<string> resolution_parts = split(resolution_str);
    if(resolution_parts.size() >= 2) {
        scr_w = max(1, s2i(resolution_parts[0]));
        scr_h = max(1, s2i(resolution_parts[1]));
    }
    
    for(size_t h = 0; h < ANIMATION_EDITOR_HISTORY_SIZE; ++h) {
        rs.set(
            "animation_editor_history_" + i2s(h + 1),
            animation_editor_history[h]
        );
    }
    
}


/* ----------------------------------------------------------------------------
 * Loads an audio sample from the game's content.
 */
sample_struct load_sample(
    const string &file_name, ALLEGRO_MIXER* const mixer
) {
    ALLEGRO_SAMPLE* sample =
        al_load_sample((AUDIO_FOLDER_PATH + "/" + file_name).c_str());
    if(!sample) {
        log_error("Could not open audio sample " + file_name + "!");
    }
    
    return sample_struct(sample, mixer);
}


/* ----------------------------------------------------------------------------
 * Loads spray types from the game data.
 */
void load_spray_types() {
    data_node file = data_node(MISC_FOLDER_PATH + "/Sprays.txt");
    if(!file.file_was_opened) return;
    
    size_t n_sprays = file.get_nr_of_children();
    for(size_t s = 0; s < n_sprays; ++s) {
        data_node* s_node = file.get_child(s);
        spray_type st;
        
        st.name = s_node->name;
        
        data_node* effects_node = s_node->get_child_by_name("effects");
        vector<string> effects_strs =
            semicolon_list_to_vector(effects_node->value);
        for(size_t e = 0; e < effects_strs.size(); ++e) {
            string effect_name = effects_strs[e];
            if(status_types.find(effect_name) == status_types.end()) {
                log_error(
                    "Unknown status effect \"" + effect_name + "\"!",
                    effects_node
                );
            } else {
                st.effects.push_back(&(status_types[effect_name]));
            }
        }
        
        reader_setter rs(s_node);
        rs.set("group", st.group);
        rs.set("angle", st.angle);
        rs.set("distance_range", st.distance_range);
        rs.set("angle_range", st.angle_range);
        rs.set("color", st.main_color);
        rs.set("berries_needed", st.berries_needed);
        
        data_node* icon_node = s_node->get_child_by_name("icon");
        st.bmp_spray = bitmaps.get(icon_node->value, icon_node);
        
        spray_types.push_back(st);
    }
}


/* ----------------------------------------------------------------------------
 * Loads status effect types from the game data.
 */
void load_status_types() {
    data_node file = data_node(MISC_FOLDER_PATH + "/Statuses.txt");
    if(!file.file_was_opened) return;
    
    size_t n_statuses = file.get_nr_of_children();
    for(size_t s = 0; s < n_statuses; ++s) {
        data_node* s_node = file.get_child(s);
        status_type st;
        
        st.name = s_node->name;
        
        reader_setter rs(s_node);
        rs.set("color",                   st.color);
        rs.set("tint",                    st.tint);
        rs.set("removable_with_whistle",  st.removable_with_whistle);
        rs.set("auto_remove_time",        st.auto_remove_time);
        rs.set("health_change_ratio",     st.health_change_ratio);
        rs.set("causes_disable",          st.causes_disable);
        rs.set("causes_flailing",         st.causes_flailing);
        rs.set("causes_panic",            st.causes_panic);
        rs.set("disabled_state_inedible", st.disabled_state_inedible);
        rs.set("speed_multiplier",        st.speed_multiplier);
        rs.set("attack_multiplier",       st.attack_multiplier);
        rs.set("defense_multiplier",      st.defense_multiplier);
        rs.set("anim_speed_multiplier",   st.anim_speed_multiplier);
        rs.set("animation",               st.animation_name);
        rs.set("animation_mob_scale",     st.animation_mob_scale);
        
        st.affects = 0;
        if(s2b(s_node->get_child_by_name("affects_pikmin")->value)) {
            st.affects |= STATUS_AFFECTS_PIKMIN;
        }
        if(s2b(s_node->get_child_by_name("affects_leaders")->value)) {
            st.affects |= STATUS_AFFECTS_LEADERS;
        }
        if(s2b(s_node->get_child_by_name("affects_enemies")->value)) {
            st.affects |= STATUS_AFFECTS_ENEMIES;
        }
        
        data_node* pg_node = s_node->get_child_by_name("particle_generator");
        string pg_name = pg_node->value;
        if(!pg_name.empty()) {
            if(
                custom_particle_generators.find(pg_name) ==
                custom_particle_generators.end()
            ) {
                log_error(
                    "Unknown particle generator \"" +
                    pg_name + "\"!", pg_node
                );
            } else {
                st.generates_particles = true;
                st.particle_gen = &custom_particle_generators[pg_name];
            }
        }
        
        status_types[st.name] = st;
    }
    
    for(auto s = status_types.begin(); s != status_types.end(); ++s) {
        if(s->second.animation_name.empty()) continue;
        data_node anim_file(ANIMATIONS_FOLDER_PATH + "/" + s->second.animation_name);
        s->second.anim_pool = load_animation_database_from_file(&anim_file);
        if(!s->second.anim_pool.animations.empty()) {
            s->second.anim_instance = animation_instance(&s->second.anim_pool);
            s->second.anim_instance.cur_anim = s->second.anim_pool.animations[0];
            s->second.anim_instance.start();
        }
    }
}


/* ----------------------------------------------------------------------------
 * Prints something onto the error log.
 * s: String that represents the error.
 * d: If not null, this will be used to obtain the file name
   * and line that caused the error.
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
    
    for(unsigned char p = 0; p < 4; ++p) {
        string prefix = "p" + i2s((p + 1)) + "_";
        for(size_t b = 0; b < N_BUTTONS; ++b) {
            string option_name = buttons.list[b].option_name;
            if(option_name.empty()) continue;
            grouped_controls[prefix + option_name].clear();
        }
    }
    
    //Write down their control strings.
    for(size_t p = 0; p < 4; p++) {
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
    
    for(unsigned char p = 0; p < 4; ++p) {
        file.add(
            new data_node(
                "p" + i2s((p + 1)) + "_mouse_moves_cursor",
                b2s(mouse_moves_cursor[p])
            )
        );
    }
    
    //Other options.
    file.add(new data_node("area_quality", f2s(area_images_scale)));
    file.add(new data_node("draw_cursor_trail", b2s(draw_cursor_trail)));
    file.add(
        new data_node("editor_backup_interval", f2s(editor_backup_interval))
    );
    file.add(new data_node("fps", i2s(game_fps)));
    file.add(new data_node("max_particles", i2s(max_particles)));
    file.add(new data_node("middle_zoom_level", f2s(zoom_mid_level)));
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
 * Splits a string into several substrings, by the specified delimiter.
 * text:        The string to split.
 * del:         The delimiter. Default is space.
 * inc_empty:   If true, include empty substrings on the vector.
   * i.e. if two delimiters come together in a row,
   * keep an empty substring between.
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
