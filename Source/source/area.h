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

#include "sector.h"


//Types of areas that can be played.
enum AREA_TYPES {
    //A simple area with no goal.
    AREA_TYPE_SIMPLE,
    //An area that likely has a goal, constraints, and/or scoring.
    AREA_TYPE_MISSION,
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
    ALLEGRO_BITMAP* thumbnail;
    //Thumbnail, if any.
    string tags;
    //Stage difficulty, if applicable. Goes from 1 to 5.
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
    //Known geometry problems.
    geometry_problems problems;
    
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
    void generate_edges_blockmap(vector<edge*> &edges);
    size_t get_nr_path_links();
    void load_thumbnail();
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
