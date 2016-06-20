/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the area editor-related functions.
 */

#ifndef AREA_EDITOR_INCLUDED
#define AREA_EDITOR_INCLUDED

#include <string>
#include <unordered_set>
#include <vector>

#include <allegro5/allegro.h>

#include "game_state.h"
#include "LAFI/gui.h"
#include "LAFI/widget.h"
#include "misc_structs.h"
#include "sector.h"

using namespace std;

struct texture_suggestion {
    ALLEGRO_BITMAP* bmp;
    string name;
    texture_suggestion(const string &n);
    ~texture_suggestion();
};

class area_editor : public game_state {
private:
    enum AREA_EDITOR_PICKER_TYPES {
        AREA_EDITOR_PICKER_AREA,
        AREA_EDITOR_PICKER_SECTOR_TYPE,
        AREA_EDITOR_PICKER_MOB_CATEGORY,
        AREA_EDITOR_PICKER_MOB_TYPE,
    };
    
    enum EDITOR_ERROR_TYPES {
        EET_NONE_YET,
        EET_NONE,
        EET_INTERSECTING_EDGES,   //Two edges intersect.
        EET_LONE_EDGE,            //An edge is all by itself.
        EET_OVERLAPPING_VERTEXES, //Two vertexes in the same spot.
        EET_BAD_SECTOR,           //A sector is corrupted.
        EET_MISSING_LEADER,       //No leader mob found.
        EET_MISSING_TEXTURE,      //A sector is without texture.
        EET_UNKNOWN_TEXTURE,      //A texture is not found in the game files.
        EET_TYPELESS_MOB,         //Mob with no type.
        EET_MOB_OOB,              //Mob out of bounds.
        EET_MOB_IN_WALL,          //Mob stuck in a wall.
        EET_LONE_PATH_STOP,       //A path stop is all by itself.
        EET_PATH_STOP_OOB,        //A path stop is out of bounds.
        EET_PATH_STOPS_TOGETHER,  //Two path stops are in the same place.
        EET_PATHS_UNCONNECTED,    //The path graph is unconnected.
        EET_INVALID_SHADOW,       //Invalid tree shadow image.
    };
    
    static const float  DEF_GRID_INTERVAL;
    static const float  MAX_GRID_INTERVAL;
    static const size_t MAX_TEXTURE_SUGGESTIONS;
    static const float  MIN_GRID_INTERVAL;
    static const float  PATH_LINK_THICKNESS;
    static const float  PATH_PREVIEW_CHECKPOINT_RADIUS;
    static const float  PATH_PREVIEW_TIMEOUT_DUR;
    static const float  STOP_RADIUS;
    static const float  VERTEX_MERGE_RADIUS;
    
    string                       area_name;
    timer                        backup_timer;
    mob_gen*                     cur_mob;
    sector*                      cur_sector;
    tree_shadow*                 cur_shadow;
    path_stop*                   cur_stop;
    bool                         debug_edge_nrs;
    bool                         debug_sector_nrs;
    bool                         debug_triangulation;
    bool                         debug_vertex_nrs;
    float                        double_click_time;
    mob_gen*                     error_mob_ptr;
    path_stop*                   error_path_stop_ptr;
    sector*                      error_sector_ptr;
    tree_shadow*                 error_shadow_ptr;
    string                       error_string;
    unsigned char                error_type;
    vertex*                      error_vertex_ptr;
    float                        grid_interval;
    lafi::gui*                   gui;
    bool                         guide_aspect_ratio;
    ALLEGRO_BITMAP*              guide_bitmap;
    string                       guide_file_name;
    float                        guide_x;
    float                        guide_y;
    float                        guide_w;
    float                        guide_h;
    unsigned char                guide_a;
    bool                         holding_m1;
    bool                         holding_m2;
    bool                         made_changes;
    unsigned char                mode;
    unsigned char                mode_before_options;
    signed char                  moving_path_preview_checkpoint;
    //Current vertex, object or shadow being moved.
    size_t                       moving_thing;
    //Relative X/Y coordinate of the point where the vertex,
    //object or shadow was grabbed.
    float                        moving_thing_x;
    float                        moving_thing_y;
    path_stop*                   new_link_first_stop;
    vector<vertex*>              new_sector_vertexes;
    bool                         new_sector_valid_line;
    sector*                      on_sector;
    float                        path_preview_checkpoints_x[2];
    float                        path_preview_checkpoints_y[2];
    vector<path_stop*>           path_preview;
    timer                        path_preview_timeout;
    unsigned char                sec_mode; //Secondary/sub mode.
    bool                         shift_pressed;
    bool                         show_closest_stop;
    bool                         show_path_preview;
    bool                         show_guide;
    bool                         show_shadows;
    vector<texture_suggestion>   texture_suggestions;
    lafi::widget*                wum; //Widget under mouse.
    
    void adv_textures_to_gui();
    void guide_to_gui();
    void calculate_preview_path();
    void cancel_new_sector();
    void center_camera(float min_x, float min_y, float max_x, float max_y);
    void close_changes_warning();
    void change_guide(string new_file_name);
    void change_to_right_frame(bool hide_all = false);
    void create_sector();
    void leave();
    void find_errors();
    bool get_common_sector(
        vector<vertex*> &vertexes, vector<vertex*> &merges, sector** result
    );
    vertex* get_merge_vertex(const float x, const float y, size_t* nr = NULL);
    void goto_error();
    void gui_to_guide();
    void gui_to_mob();
    void gui_to_sector(bool called_by_brightness_bar = false);
    void gui_to_shadow();
    void gui_to_adv_textures();
    bool is_edge_valid(edge* l);
    bool is_new_sector_line_valid(const float x, const float y);
    bool is_polygon_clockwise(vector<vertex*> &vertexes);
    void load_area(const bool from_backup);
    void load_backup();
    void merge_vertex(
        vertex* v1, vertex* v2,
        const size_t v2_nr, unordered_set<sector*>* affected_sectors
    );
    void mob_to_gui();
    void open_picker(unsigned char type);
    void pick(string name, unsigned char type);
    void populate_texture_suggestions();
    void resize_everything();
    void save_area(const bool to_backup);
    void save_backup();
    void sector_to_gui();
    void shadow_to_gui();
    void show_changes_warning();
    float snap_to_grid(const float c);
    void toggle_duplicate_mob_mode();
    bool update_backup_status();
    void update_options_frame();
    void update_review_frame();
    void update_texture_suggestions(const string &n);
    
public:

    vector<edge_intersection> intersecting_edges;
    unordered_set<sector*>    non_simples;
    unordered_set<edge*>      lone_edges;
    
    string auto_load_area;
    
    area_editor();
    
    virtual void do_logic();
    virtual void do_drawing();
    virtual void handle_controls(ALLEGRO_EVENT ev);
    virtual void load();
    virtual void unload();
    
    void set_guide_file_name(string n);
    void set_guide_x(float x);
    void set_guide_y(float y);
    void set_guide_w(float w);
    void set_guide_h(float h);
    void set_guide_a(unsigned char a);
    
};

#endif //ifndef AREA_EDITOR_INCLUDED
