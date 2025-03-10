/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the area class and related functions.
 */

#pragma once

#include <memory>

#include "../../core/pathing.h"
#include "../../util/drawing_utils.h"
#include "../../util/general_utils.h"
#include "../content.h"
#include "../other/weather.h"
#include "mission.h"
#include "sector.h"


//Types of areas that can be played.
enum AREA_TYPE {

    //A simple area with no goal.
    AREA_TYPE_SIMPLE,
    
    //An area that likely has a goal, constraints, and/or scoring.
    AREA_TYPE_MISSION,
    
    //Total number of area types.
    N_AREA_TYPES,
    
};


namespace AREA {
extern const float DEF_DAY_TIME_SPEED;
extern const size_t DEF_DAY_TIME_START;
extern const unsigned char DEF_DIFFICULTY;
};


/**
 * @brief Info about dividing the area in a grid.
 *
 * The blockmap divides the entire area
 * in a grid, so that collision detections only
 * happen between stuff in the same grid cell.
 * This is to avoid having, for instance, a Pikmin
 * on the lake part of TIS check for collisions with
 * a wall on the landing site part of TIS.
 * It's also used when checking sectors in a certain spot.
 */
struct Blockmap {

    //--- Members ---
    
    //Top-left corner of the blockmap.
    Point top_left_corner;
    
    //Specifies a list of edges in each block.
    vector<vector<vector<Edge*> > > edges;
    
    //Specifies a list of sectors in each block.
    vector<vector<unordered_set<Sector*> > >  sectors;
    
    //Number of columns.
    size_t n_cols = 0;
    
    //Number of rows.
    size_t n_rows = 0;
    
    
    //--- Function declarations ---
    
    size_t get_col(float x) const;
    size_t get_row(float y) const;
    bool get_edges_in_region(
        const Point &tl, const Point &br, set<Edge*> &edges
    ) const;
    Point get_top_left_corner(size_t col, size_t row) const;
    void clear();
    
};


/**
 * @brief Info for a mob's generation.
 *
 * It is a mob's representation on the editor and in the area file,
 * but it doesn't have the data of a LIVING mob. This only holds its
 * position and type data, plus some other tiny things.
 */
struct MobGen {

    //--- Members ---
    
    //Mob type.
    MobType* type = nullptr;
    
    //Position.
    Point pos;
    
    //Angle.
    float angle = 0.0f;
    
    //Script vars.
    string vars;
    
    //Indexes of linked objects.
    vector<size_t> link_idxs;
    
    //Index to the mob storing this one inside, if any.
    size_t stored_inside = INVALID;
    
    //Linked objects. Cache for performance.
    vector<MobGen*> links;
    
    
    //--- Function declarations ---
    
    explicit MobGen(
        const Point &pos = Point(),
        MobType* type = nullptr, float angle = 0, const string &vars = ""
    );
    void clone(MobGen* destination, bool include_position = true) const;
    
};


/**
 * @brief Info about the shadows cast onto the area by a tree
 * (or whatever the game maker desires).
 */
struct TreeShadow {

    //--- Members ---
    
    //Internal name of the tree shadow texture.
    string bmp_name;
    
    //Tree shadow texture.
    ALLEGRO_BITMAP* bitmap = nullptr;
    
    //Center coordinates.
    Point center;
    
    //Width and height.
    Point size;
    
    //Angle.
    float angle = 0.0f;
    
    //Opacity.
    unsigned char alpha = 255;
    
    //Swaying is multiplied by this.
    Point sway;
    
    
    //--- Function declarations ---
    
    explicit TreeShadow(
        const Point &center = Point(), const Point &size = Point(100.0f),
        float angle = 0, unsigned char alpha = 255,
        const string &bmp_name = "", const Point &sway = Point(1.0f)
    );
    ~TreeShadow();
    
};


/**
 * @brief Info about an area.
 *
 * This structure is so that the sectors know how to communicate with
 * the edges, the edges with the vertexes, etc.
 */
struct Area : public Content {

    //--- Members ---
    
    //Type of area.
    AREA_TYPE type = AREA_TYPE_SIMPLE;
    
    //Blockmap.
    Blockmap bmap;
    
    //List of vertexes.
    vector<Vertex*> vertexes;
    
    //List of edges.
    vector<Edge*> edges;
    
    //List of sectors.
    vector<Sector*> sectors;
    
    //List of mob generators.
    vector<MobGen*> mob_generators;
    
    //List of path stops.
    vector<PathStop*> path_stops;
    
    //List of tree shadows.
    vector<TreeShadow*> tree_shadows;
    
    //Bitmap of the background.
    ALLEGRO_BITMAP* bg_bmp = nullptr;
    
    //Internal name of the background bitmap.
    string bg_bmp_name;
    
    //Zoom the background by this much.
    float bg_bmp_zoom = 1.0f;
    
    //How far away the background is.
    float bg_dist = 2.0f;
    
    //Tint the background with this color.
    ALLEGRO_COLOR bg_color = COLOR_BLACK;
    
    //Area subtitle, if any.
    string subtitle;
    
    //Thumbnail, if any.
    std::shared_ptr<ALLEGRO_BITMAP> thumbnail = nullptr;
    
    //Area difficulty, if applicable. Goes from 1 to 5.
    unsigned char difficulty = AREA::DEF_DIFFICULTY;
    
    //String representing the starting amounts of each spray.
    string spray_amounts;
    
    //Song to play.
    string song_name;
    
    //Weather condition to use.
    Weather weather_condition;
    
    //Name of the weather condition to use.
    string weather_name;
    
    //Area day time at the start of gameplay. This is in minutes.
    size_t day_time_start = AREA::DEF_DAY_TIME_START;
    
    //Area day time speed, in game-minutes per real-minutes.
    float day_time_speed = AREA::DEF_DAY_TIME_SPEED;
    
    //Known geometry problems.
    GeometryProblems problems;
    
    //Mission data.
    MissionData mission;
    
    //Path to the user data folder for this area.
    string user_data_path;
    
    
    //--- Function declarations ---
    
    void check_stability();
    void cleanup(bool* out_deleted_sectors = nullptr);
    void clone(Area &other);
    void connect_edge_to_sector(Edge* e_ptr, Sector* s_ptr, size_t side);
    void connect_edge_to_vertex(Edge* e_ptr, Vertex* v_ptr, size_t endpoint);
    void connect_sector_edges(Sector* s_ptr);
    void connect_vertex_edges(Vertex* v_ptr);
    size_t find_edge_idx(const Edge* e_ptr) const;
    size_t find_mob_gen_idx(const MobGen* m_ptr) const;
    size_t find_sector_idx(const Sector* s_ptr) const;
    size_t find_vertex_idx(const Vertex* v_ptr) const;
    void fix_edge_idxs(Edge* e_ptr);
    void fix_edge_pointers(Edge* e_ptr);
    void fix_path_stop_idxs(PathStop* s_ptr);
    void fix_path_stop_pointers(PathStop* s_ptr);
    void fix_sector_idxs(Sector* s_ptr);
    void fix_sector_pointers(Sector* s_ptr);
    void fix_vertex_idxs(Vertex* v_ptr);
    void fix_vertex_pointers(Vertex* v_ptr);
    void generate_blockmap();
    void generate_edges_blockmap(const vector<Edge*> &edges);
    size_t get_nr_path_links();
    void load_main_data_from_data_node(
        DataNode* node, CONTENT_LOAD_LEVEL level
    );
    void load_mission_data_from_data_node(DataNode* node);
    void load_geometry_from_data_node(
        DataNode* node, CONTENT_LOAD_LEVEL level
    );
    void load_thumbnail(const string &thumbnail_path);
    Edge* new_edge();
    Sector* new_sector();
    Vertex* new_vertex();
    void remove_vertex(size_t v_idx);
    void remove_vertex(const Vertex* v_ptr);
    void remove_edge(size_t e_idx);
    void remove_edge(const Edge* e_ptr);
    void remove_sector(size_t s_idx);
    void remove_sector(const Sector* s_ptr);
    void save_geometry_to_data_node(DataNode* node);
    void save_main_data_to_data_node(DataNode* node);
    void save_mission_data_to_data_node(DataNode* node);
    void save_thumbnail(bool to_backup);
    void clear();
    
};
