/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the area class and related functions.
 */

#ifndef AREA_INCLUDED
#define AREA_INCLUDED

#include <memory>

#include "../mission.h"
#include "../pathing.h"
#include "../weather.h"
#include "sector.h"


//Types of areas that can be played.
enum AREA_TYPES {
    //A simple area with no goal.
    AREA_TYPE_SIMPLE,
    //An area that likely has a goal, constraints, and/or scoring.
    AREA_TYPE_MISSION,
};


namespace AREA {
extern const float DEF_DAY_TIME_SPEED;
extern const size_t DEF_DAY_TIME_START;
extern const unsigned char DEF_DIFFICULTY;
extern const int DEF_MISSION_MEDAL_BRONZE_REQ;
extern const int DEF_MISSION_MEDAL_GOLD_REQ;
extern const int DEF_MISSION_MEDAL_PLATINUM_REQ;
extern const int DEF_MISSION_MEDAL_SILVER_REQ;
extern const size_t DEF_MISSION_TIME_LIMIT;
};


/* ----------------------------------------------------------------------------
 * The blockmap divides the entire area
 * in a grid, so that collision detections only
 * happen between stuff in the same grid cell.
 * This is to avoid having, for instance, a Pikmin
 * on the lake part of TIS check for collisions with
 * a wall on the landing site part of TIS.
 * It's also used when checking sectors in a certain spot.
 */
struct blockmap {
    //Top-left corner of the blockmap.
    point top_left_corner;
    //Specifies a list of edges in each block.
    vector<vector<vector<edge*> > > edges;
    //Specifies a list of sectors in each block.
    vector<vector<unordered_set<sector*> > >  sectors;
    //Number of columns.
    size_t n_cols;
    //Number of rows.
    size_t n_rows;
    
    blockmap();
    size_t get_col(const float x) const;
    size_t get_row(const float y) const;
    bool get_edges_in_region(
        const point &tl, const point &br, set<edge*> &edges
    ) const;
    point get_top_left_corner(const size_t col, const size_t row) const;
    void clear();
};


/* ----------------------------------------------------------------------------
 * This structure holds the information for a mob's generation.
 * It is a mob's representation on the editor and in the area file,
 * but it doesn't have the data of a LIVING mob. This only holds its
 * position and type data, plus some other tiny things.
 */
struct mob_gen {
    //Mob type.
    mob_type* type;
    //Position.
    point pos;
    //Angle.
    float angle;
    //Script vars.
    string vars;
    //Indexes of linked objects.
    vector<size_t> link_nrs;
    //Index to the mob storing this one inside, if any.
    size_t stored_inside;
    //Linked objects. Cache for performance.
    vector<mob_gen*> links;
    
    mob_gen(
        const point &pos = point(),
        mob_type* type = NULL, const float angle = 0, const string &vars = ""
    );
    
    void clone(mob_gen* destination, const bool include_position = true) const;
};


/* ----------------------------------------------------------------------------
 * A structure holding info on the shadows
 * cast onto the area by a tree (or
 * whatever the game maker desires).
 */
struct tree_shadow {
    //File name of the tree shadow texture.
    string file_name;
    //Tree shadow texture.
    ALLEGRO_BITMAP* bitmap;
    //Center coordinates.
    point center;
    //Width and height.
    point size;
    //Angle.
    float angle;
    //Opacity.
    unsigned char alpha;
    //Swaying is multiplied by this.
    point sway;
    
    tree_shadow(
        const point &center = point(), const point &size = point(100, 100),
        const float angle = 0, const unsigned char alpha = 255,
        const string &file_name = "", const point &sway = point(1, 1)
    );
    ~tree_shadow();
};


/* ----------------------------------------------------------------------------
 * A structure that holds all of the
 * info about the current area, so that
 * the sectors know how to communicate with
 * the edges, the edges with the
 * vertexes, etc.
 */
struct area_data {
    //Type of area.
    AREA_TYPES type;
    //Name of the folder with this area's data.
    string folder_name;
    //Blockmap.
    blockmap bmap;
    //List of vertexes.
    vector<vertex*> vertexes;
    //List of edges.
    vector<edge*> edges;
    //List of sectors.
    vector<sector*> sectors;
    //List of mob generators.
    vector<mob_gen*> mob_generators;
    //List of path stops.
    vector<path_stop*> path_stops;
    //List of tree shadows.
    vector<tree_shadow*> tree_shadows;
    //Bitmap of the background.
    ALLEGRO_BITMAP* bg_bmp;
    //File name of the background bitmap.
    string bg_bmp_file_name;
    //Zoom the background by this much.
    float bg_bmp_zoom;
    //How far away the background is.
    float bg_dist;
    //Tint the background with this color.
    ALLEGRO_COLOR bg_color;
    //Name of the area. This is not the internal name.
    string name;
    //Area subtitle, if any.
    string subtitle;
    //Area description, if any.
    string description;
    //Area tags, separated by semicolon, if any.
    std::shared_ptr<ALLEGRO_BITMAP> thumbnail;
    //Thumbnail, if any.
    string tags;
    //Area difficulty, if applicable. Goes from 1 to 5.
    unsigned char difficulty;
    //Who made this area.
    string maker;
    //Optional version number.
    string version;
    //Any notes from the person who made it, for other makers to see.
    string notes;
    //Version of the engine this area was built in.
    string engine_version;
    //String representing the starting amounts of each spray.
    string spray_amounts;
    //Weather condition to use.
    weather weather_condition;
    //Name of the weather condition to use.
    string weather_name;
    //Area day time at the start of gameplay. This is in minutes.
    size_t day_time_start;
    //Area day time speed, in game-minutes per real-minutes.
    float day_time_speed;
    //Known geometry problems.
    geometry_problems problems;
    //Mission data.
    mission_data mission;
    
    area_data();
    void check_stability();
    void clone(area_data &other);
    void connect_edge_to_sector(edge* e_ptr, sector* s_ptr, size_t side);
    void connect_edge_to_vertex(edge* e_ptr, vertex* v_ptr, size_t endpoint);
    void connect_sector_edges(sector* s_ptr);
    void connect_vertex_edges(vertex* v_ptr);
    size_t find_edge_nr(const edge* e_ptr) const;
    size_t find_mob_gen_nr(const mob_gen* m_ptr) const;
    size_t find_sector_nr(const sector* s_ptr) const;
    size_t find_vertex_nr(const vertex* v_ptr) const;
    void fix_edge_nrs(edge* e_ptr);
    void fix_edge_pointers(edge* e_ptr);
    void fix_path_stop_nrs(path_stop* s_ptr);
    void fix_path_stop_pointers(path_stop* s_ptr);
    void fix_sector_nrs(sector* s_ptr);
    void fix_sector_pointers(sector* s_ptr);
    void fix_vertex_nrs(vertex* v_ptr);
    void fix_vertex_pointers(vertex* v_ptr);
    void generate_blockmap();
    void generate_edges_blockmap(const vector<edge*> &edges);
    size_t get_nr_path_links();
    void load_thumbnail(const string &thumbnail_path);
    edge* new_edge();
    sector* new_sector();
    vertex* new_vertex();
    void remove_vertex(const size_t v_nr);
    void remove_vertex(const vertex* v_ptr);
    void remove_edge(const size_t e_nr);
    void remove_edge(const edge* e_ptr);
    void remove_sector(const size_t s_nr);
    void remove_sector(const sector* s_ptr);
    void clear();
};


void get_area_info_from_path(
    const string &requested_area_path,
    string* final_area_folder_name,
    AREA_TYPES* final_area_type
);
string get_base_area_folder_path(
    const AREA_TYPES type, const bool from_game_data
);

#endif //ifndef AREA_INCLUDED
