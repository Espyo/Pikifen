/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the general area editor-related functions.
 */

#ifndef AREA_EDITOR_INCLUDED
#define AREA_EDITOR_INCLUDED

#include <string>
#include <unordered_set>
#include <vector>

#include <allegro5/allegro.h>

#include "editor.h"
#include "../game_state.h"
#include "../geometry_utils.h"
#include "../LAFI/gui.h"
#include "../LAFI/widget.h"
#include "../sector.h"

using namespace std;

struct texture_suggestion {
    ALLEGRO_BITMAP* bmp;
    string name;
    texture_suggestion(const string &n);
    ~texture_suggestion();
};

class area_editor : public editor {
private:
    enum EDITOR_MODES {
        EDITOR_MODE_MAIN,
        EDITOR_MODE_TOOLS,
        EDITOR_MODE_OPTIONS,
        EDITOR_MODE_SECTORS,
        EDITOR_MODE_ADV_TEXTURE_SETTINGS,
        EDITOR_MODE_TEXTURE,
        EDITOR_MODE_OBJECTS,
        EDITOR_MODE_FOLDER_PATHS,
        EDITOR_MODE_SHADOWS,
        EDITOR_MODE_GUIDE,
        EDITOR_MODE_REVIEW,
    };
    
    enum EDITOR_SEC_MODES {
        ESM_NONE,
        ESM_NEW_SECTOR,
        ESM_NEW_CIRCLE_SECTOR,
        ESM_NEW_OBJECT,
        ESM_DUPLICATE_OBJECT,
        ESM_NEW_STOP,
        ESM_NEW_LINK1,   //Click #1.
        ESM_NEW_LINK2,   //Click #2.
        ESM_NEW_1WLINK1, //One-way link, click #1.
        ESM_NEW_1WLINK2, //One-way link, click #2.
        ESM_DEL_STOP,
        ESM_DEL_LINK,
        ESM_NEW_SHADOW,
        ESM_GUIDE_MOUSE,   //Guide transformation being controlled by mouse.
        ESM_TEXTURE_VIEW,
    };
    
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
        EET_LONE_FOLDER_PATH_STOP,       //A path stop is all by itself.
        EET_FOLDER_PATH_STOP_OOB,        //A path stop is out of bounds.
        EET_FOLDER_PATH_STOPS_TOGETHER,  //Two path stops are in the same place.
        EET_FOLDER_PATHS_UNCONNECTED,    //The path graph is unconnected.
        EET_INVALID_SHADOW,       //Invalid tree shadow image.
    };
    
    static const float  CROSS_SECTION_POINT_RADIUS;
    static const float  DEBUG_TEXT_SCALE;
    static const float  DEF_GRID_INTERVAL;
    static const size_t MAX_CIRCLE_SECTOR_POINTS;
    static const float  MAX_GRID_INTERVAL;
    static const size_t MAX_TEXTURE_SUGGESTIONS;
    static const size_t MIN_CIRCLE_SECTOR_POINTS;
    static const float  MIN_GRID_INTERVAL;
    static const float  PATH_LINK_THICKNESS;
    static const float  PATH_PREVIEW_CHECKPOINT_RADIUS;
    static const float  PATH_PREVIEW_TIMEOUT_DUR;
    static const float  POINT_LETTER_TEXT_SCALE;
    static const float  STOP_RADIUS;
    static const float  VERTEX_MERGE_RADIUS;
    static const float  ZOOM_MAX_LEVEL_EDITOR;
    static const float  ZOOM_MIN_LEVEL_EDITOR;
    
    static const string EDITOR_ICONS_FOLDER_NAME;
    static const string DELETE_ICON;
    static const string DELETE_LINK_ICON;
    static const string DELETE_STOP_ICON;
    static const string DUPLICATE_ICON;
    static const string EXIT_ICON;
    static const string GUIDE_ICON;
    static const string NEW_CIRCLE_SECTOR_ICON;
    static const string NEW_1WLINK_ICON;
    static const string NEW_ICON;
    static const string NEW_LINK_ICON;
    static const string NEW_STOP_ICON;
    static const string NEXT_ICON;
    static const string OPTIONS_ICON;
    static const string PREVIOUS_ICON;
    static const string SAVE_ICON;
    static const string SELECT_NONE_ICON;
    
    string                       area_name;
    timer                        backup_timer;
    point                        cross_section_points[2];
    point                        cross_section_window_start;
    point                        cross_section_window_end;
    point                        cross_section_z_window_start;
    point                        cross_section_z_window_end;
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
    bool                         guide_aspect_ratio;
    ALLEGRO_BITMAP*              guide_bitmap;
    string                       guide_file_name;
    point                        guide_pos;
    point                        guide_size;
    unsigned char                guide_a;
    unsigned char                mode_before_options;
    signed char                  moving_path_preview_checkpoint;
    signed char                  moving_cross_section_point;
    //Current vertex, object or shadow being moved.
    size_t                       moving_thing;
    //Relative X/Y coordinate of the point where the vertex,
    //object or shadow was grabbed.
    point                        moving_thing_pos;
    point                        new_circle_sector_anchor;
    point                        new_circle_sector_center;
    vector<bool>                 new_circle_sector_valid_edges;
    vector<point>                new_circle_sector_points;
    unsigned char                new_circle_sector_step;
    path_stop*                   new_link_first_stop;
    vector<vertex*>              new_sector_vertexes;
    bool                         new_sector_valid_line;
    sector*                      on_sector;
    point                        path_preview_checkpoints[2];
    vector<path_stop*>           path_preview;
    timer                        path_preview_timeout;
    bool                         shift_pressed;
    bool                         show_closest_stop;
    bool                         show_cross_section;
    bool                         show_cross_section_grid;
    bool                         show_guide;
    bool                         show_path_preview;
    bool                         show_shadows;
    vector<texture_suggestion>   texture_suggestions;
    lafi::widget*                wum; //Widget under mouse.
    
    void calculate_preview_path();
    void cancel_new_sector();
    void center_camera(const point min_coords, const point max_coords);
    void change_guide(string new_file_name);
    void clear_current_area();
    void create_sector();
    void draw_cross_section_sector(
        const float start_ratio, const float end_ratio, const float proportion,
        const float lowest_z, sector* sector_ptr
    );
    void draw_debug_text(
        const ALLEGRO_COLOR color, const point where, const string text
    );
    void find_errors();
    bool get_common_sector(
        vector<vertex*> &vertexes, vector<vertex*> &merges, sector** result
    );
    void set_new_circle_sector_points();
    void goto_error();
    bool is_new_sector_line_valid(const point pos);
    void load_area(const bool from_backup);
    void load_backup();
    void merge_vertex(
        vertex* v1, vertex* v2, unordered_set<sector*>* affected_sectors
    );
    void open_picker(const unsigned char content_type);
    void populate_texture_suggestions();
    bool remove_isolated_sector(sector* s_ptr);
    void resize_everything();
    void save_area(const bool to_backup);
    void save_backup();
    point snap_to_grid(const point p);
    void toggle_duplicate_mob_mode();
    bool update_backup_status();
    void update_options_frame();
    void update_review_frame();
    void update_texture_suggestions(const string &n);
    
    void adv_textures_to_gui();
    void guide_to_gui();
    void mob_to_gui();
    void sector_to_gui();
    void shadow_to_gui();
    void gui_to_adv_textures();
    void gui_to_guide();
    void gui_to_mob();
    void gui_to_sector(bool called_by_brightness_bar = false);
    void gui_to_shadow();
    
    virtual void hide_all_frames();
    virtual void change_to_right_frame();
    virtual void create_new_from_picker(const string &name);
    virtual void pick(const string &name, const unsigned char type);
    
public:

    vector<edge_intersection> intersecting_edges;
    unordered_set<sector*>    non_simples;
    unordered_set<edge*>      lone_edges;
    
    string auto_load_area;
    
    area_editor();
    
    virtual void do_logic();
    virtual void do_drawing();
    virtual void handle_controls(const ALLEGRO_EVENT &ev);
    virtual void load();
    virtual void unload();
    virtual void update_transformations();
    
    void set_guide_file_name(string n);
    void set_guide_x(float x);
    void set_guide_y(float y);
    void set_guide_w(float w);
    void set_guide_h(float h);
    void set_guide_a(unsigned char a);
    
};

#endif //ifndef AREA_EDITOR_INCLUDED
