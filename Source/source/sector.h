/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the sector, edge, etc. classes and related functions.
 */

#ifndef SECTOR_INCLUDED
#define SECTOR_INCLUDED

#include <set>
#include <unordered_set>
#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "const.h"
#include "hazard.h"
#include "mobs/mob_category.h"
#include "mobs/mob_type.h"
#include "weather.h"

using namespace std;


struct area_data;
struct blockmap;
struct edge;
struct path_stop;
struct path_link;
struct sector;
struct triangle;
struct vertex;
typedef vector<vertex*> polygon;

enum TRIANGULATION_ERRORS {
    //No error occured.
    TRIANGULATION_NO_ERROR,
    //Invalid arguments provided.
    TRIANGULATION_ERROR_INVALID_ARGS,
    //Non-simple sector: Lone edges break the sector.
    TRIANGULATION_ERROR_LONE_EDGES,
    //Non-simple sector: Vertexes are used multiple times.
    TRIANGULATION_ERROR_VERTEXES_REUSED,
    //Non-simple sector: Ran out of ears while triangulating.
    TRIANGULATION_ERROR_NO_EARS,
};


/* ----------------------------------------------------------------------------
 * Intersection between two edges. Used to mark
 * edges as red on the editor.
 */
struct edge_intersection {
    edge* e1, *e2;
    edge_intersection(edge* e1, edge* e2);
    bool contains(edge* e);
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
    size_t n_cols, n_rows;
    
    blockmap();
    size_t get_col(const float x);
    size_t get_row(const float y);
    point get_top_left_corner(const size_t col, const size_t row);
    void clear();
};




/* ----------------------------------------------------------------------------
 * A line segment that delimits a sector -- an edge of a polygon.
 * In Doom, these are what's known as linedefs.
 */
struct edge {
    vertex* vertexes[2];
    size_t vertex_nrs[2];
    sector* sectors[2];
    size_t sector_nrs[2];
    
    edge(size_t v1_nr = INVALID, size_t v2_nr = INVALID);
    vertex* get_other_vertex(vertex* v_ptr);
    size_t get_side_with_sector(sector* s_ptr);
    vertex* has_neighbor(edge* other);
    size_t remove_from_sectors();
    size_t remove_from_vertexes();
    void swap_vertexes();
};




/* ----------------------------------------------------------------------------
 * Stops are points that make up a path. In mathematics, this is a node
 * in the graph. In a real-world example, this is a bus stop.
 * Pikmin start carrying by going for the closest stop.
 * Then they move stop by stop, following the connections, until they
 * reach the final stop and go wherever they need.
 */
struct path_stop {
    point pos;
    vector<path_link> links;
    
    path_stop(
        const point &pos = point(),
        const vector<path_link> &links = vector<path_link>()
    );
    void add_link(path_stop* other_stop, const bool normal);
    bool has_link(path_stop* other_stop);
    void remove_link(path_stop* other_stop);
    void calculate_dists();
};




/* ----------------------------------------------------------------------------
 * Info about a path link. A path stop can link to N other path stops,
 * and this structure holds information about the connection.
 */
struct path_link {
    path_stop* end_ptr;
    size_t end_nr;
    float distance;
    
    path_link(path_stop* end_ptr, size_t end_nr);
    void calculate_dist(path_stop* start_ptr);
};




/* ----------------------------------------------------------------------------
 * Information about a sector's texture.
 */
struct sector_texture_info {
    point scale; //Texture scale.
    point translation; //Translation.
    float rot; //Rotation.
    ALLEGRO_BITMAP* bitmap;
    ALLEGRO_COLOR tint;
    string file_name;
    
    sector_texture_info();
};




/* ----------------------------------------------------------------------------
 * A sector, like the ones in Doom.
 * It's composed of edges (linedefs), so it's essentially
 * a polygon. It has a certain height, and its appearance
 * is determined by its floors.
 */
struct sector {
    unsigned char type;
    float z; //Height.
    string tag;
    unsigned char brightness;
    
    sector_texture_info texture_info;
    bool fade;
    bool always_cast_shadow;
    
    string hazards_str; //For the editor.
    vector<hazard*> hazards; //For gameplay.
    bool hazard_floor;
    liquid* associated_liquid;
    
    vector<size_t> edge_nrs;
    vector<edge*> edges;
    vector<triangle> triangles;
    
    point bbox[2];
    
    sector();
    void add_edge(edge* e_ptr, const size_t e_nr);
    void clone(sector* new_sector);
    void get_texture_merge_sectors(sector** s1, sector** s2);
    void remove_edge(edge* e_ptr);
    ~sector();
};




/* ----------------------------------------------------------------------------
 * A triangle. Sectors (polygons) are made out of triangles.
 * These are used to detect whether a point is inside a sector,
 * and to draw, seeing as OpenGL cannot draw concave polygons.
 */
struct triangle {
    vertex* points[3];
    triangle(vertex* v1, vertex* v2, vertex* v3);
};




/* ----------------------------------------------------------------------------
 * A vertex is a 2D point, used to determine
 * the end-points of an edge.
 */
struct vertex {
    float x, y;
    vector<size_t> edge_nrs;
    vector<edge*> edges;
    
    vertex(float x = 0.0f, float y = 0.0f);
    void add_edge(edge* e_ptr, const size_t e_nr);
    edge* get_edge_by_neighbor(vertex* neighbor);
    bool has_edge(edge* e_ptr);
    void remove_edge(edge* e_ptr);
};




/* ----------------------------------------------------------------------------
 * This structure holds the information for
 * a mob's generation. It's a mob on the editor
 * and area file, but it doesn't have the
 * data of a LIVING mob. This only holds its
 * position and type data, plus some other
 * tiny things.
 */
struct mob_gen {
    mob_category* category;
    mob_type* type;
    
    point pos;
    float angle;
    string vars;
    
    mob_gen(
        mob_category* category = NULL, const point &pos = point(),
        mob_type* type = NULL, const float angle = 0, const string &vars = ""
    );
};




/* ----------------------------------------------------------------------------
 * A structure holding info on the shadows
 * cast onto the area by a tree (or
 * whatever the game maker desires).
 */
struct tree_shadow {
    string file_name;
    ALLEGRO_BITMAP* bitmap;
    
    point center; //X and Y of the center.
    point size;   //Width and height.
    float angle;  //Rotation angle.
    unsigned char alpha; //Opacity.
    point sway;   //Swaying is multiplied by this.
    
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

    blockmap bmap;
    vector<vertex*> vertexes;
    vector<edge*> edges;
    vector<sector*> sectors;
    vector<mob_gen*> mob_generators;
    vector<path_stop*> path_stops;
    vector<tree_shadow*> tree_shadows;
    
    ALLEGRO_BITMAP* bg_bmp;
    string bg_bmp_file_name;
    float bg_bmp_zoom;
    float bg_dist;
    ALLEGRO_COLOR bg_color;
    string name;
    string subtitle;
    
    string reference_file_name;
    point reference_center;
    point reference_size;
    unsigned char reference_alpha;
    
    weather weather_condition;
    string weather_name;
    
    area_data();
    void check_stability();
    void clone(area_data &other);
    void connect_edge_to_sector(edge* e_ptr, sector* s_ptr, size_t side);
    void connect_edge_to_vertex(edge* e_ptr, vertex* v_ptr, size_t endpoint);
    void connect_sector_edges(sector* s_ptr);
    void connect_vertex_edges(vertex* v_ptr);
    size_t find_edge_nr(const edge* e_ptr);
    size_t find_sector_nr(const sector* s_ptr);
    size_t find_vertex_nr(const vertex* v_ptr);
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




void clean_poly(polygon* p);
void cut_poly(polygon* outer, vector<polygon>* inners);
void depth_first_search(
    vector<path_stop*> &nodes,
    unordered_set<path_stop*> &visited, path_stop* start
);
vector<path_stop*> dijkstra(
    path_stop* start_node, path_stop* end_node,
    mob** obstacle_found, float* total_dist
);
float get_angle_cw_dif(float a1, float a2);
float get_angle_smallest_dif(float a1, float a2);
void get_cce(
    vector<vertex> &vertexes_left, vector<size_t> &ears,
    vector<size_t> &convex_vertexes, vector<size_t> &concave_vertexes
);
vertex* get_merge_vertex(
    const point &p,
    vector<vertex*> &all_vertexes, const float merge_radius,
    size_t* nr = NULL, vertex* ignore = NULL
);
vector<path_stop*> get_path(
    const point &start, const point &end,
    mob** obstacle_found, bool* go_straight, float* get_dist
);
mob* get_path_link_obstacle(path_stop* s1, path_stop* s2);
float get_point_sign(
    const point &p, const point &lp1, const point &lp2
);
TRIANGULATION_ERRORS get_polys(
    sector* s, polygon* outer, vector<polygon>* inners,
    set<edge*>* lone_edges, const bool check_vertex_reuse
);
vertex* get_rightmost_vertex(map<edge*, bool> &edges);
vertex* get_rightmost_vertex(polygon* p);
vertex* get_rightmost_vertex(sector* s);
vertex* get_rightmost_vertex(vertex* v1, vertex* v2);
sector* get_sector(
    const point &p, size_t* sector_nr, const bool use_blockmap
);
void get_sector_bounding_box(
    sector* s_ptr, point* min_coords, point* max_coords
);
void get_shadow_bounding_box(
    tree_shadow* s_ptr, point* min_coords, point* max_coords
);
bool is_edge_valid(edge* l);
bool is_path_link_ok(path_stop* s1, path_stop* s2);
bool is_polygon_clockwise(vector<vertex*> &vertexes);
bool is_sector_clockwise(sector* s_ptr);
bool is_vertex_convex(const vector<vertex> &vec, const size_t nr);
bool is_vertex_ear(
    const vector<vertex> &vec, const vector<size_t> &concaves, const size_t nr
);
bool is_point_in_sector(const point &p, sector* s_ptr);
bool is_point_in_triangle(
    const point &p, const point &tp1, const point &tp2, const point &tp3,
    bool loq
);
bool lines_intersect(
    const point &l1p1, const point &l1p2, const point &l2p1, const point &l2p2,
    float* ur, float* ul
);
bool lines_intersect(
    const point &l1p1, const point &l1p2, const point &l2p1, const point &l2p2,
    point* intersection
);
TRIANGULATION_ERRORS triangulate(
    sector* s_ptr, set<edge*>* lone_edges, const bool check_vertex_reuse,
    const bool clear_lone_edges
);


enum SECTOR_TYPES {
    SECTOR_TYPE_NORMAL,
    SECTOR_TYPE_BOTTOMLESS_PIT,
    SECTOR_TYPE_BLOCKING,
    SECTOR_TYPE_BRIDGE,
    SECTOR_TYPE_BRIDGE_RAIL,
};

enum TERRAIN_SOUNDS {
    TERRAIN_SOUND_NONE,
    TERRAIN_SOUND_DIRT,
    TERRAIN_SOUND_GRASS,
    TERRAIN_SOUND_STONE,
    TERRAIN_SOUND_WOOD,
    TERRAIN_SOUND_METAL,
    TERRAIN_SOUND_WATER,
};

const float BLOCKMAP_BLOCK_SIZE = 128;
const unsigned char DEF_SECTOR_BRIGHTNESS = 255;
//Mobs can walk up sectors that are, at the most,
//this high from the current one, as if climbing up steps.
const float SECTOR_STEP = 50;

#endif //ifndef SECTOR_INCLUDED
