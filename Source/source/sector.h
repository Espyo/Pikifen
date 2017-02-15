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
#include "mobs/mob_type.h"
#include "weather.h"

using namespace std;


struct area_data;
struct blockmap;
struct edge;
struct path_stop;
struct path_link;
struct sector;
struct sector_correction;
struct triangle;
struct vertex;
typedef vector<vertex*> polygon;


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
    float x1, y1;
    //Specifies a list of edges in each block.
    vector<vector<vector<edge*> > > edges;
    //Specifies a list of sectors in each block.
    vector<vector<unordered_set<sector*> > >  sectors;
    size_t n_cols, n_rows;
    
    blockmap();
    size_t get_col(const float x);
    size_t get_row(const float y);
    float get_x1(const size_t col);
    float get_y1(const size_t row);
    void clear();
};




/* ----------------------------------------------------------------------------
 * An line segment that delimits a sector -- an edge of a polygon.
 * In Doom, these are what's known as linedefs.
 */
struct edge {
    vertex* vertexes[2];
    size_t vertex_nrs[2];
    sector* sectors[2];
    size_t sector_nrs[2];
    
    edge(size_t v1 = INVALID, size_t v2 = INVALID);
    void fix_pointers(area_data &a);
    size_t remove_from_sectors();
    size_t remove_from_vertexes();
};




/* ----------------------------------------------------------------------------
 * Stops are points that make up a path. In mathematics, this is a node
 * in the graph. In a real-world example, this is a bus stop.
 * Pikmin start carrying by going for the closest stop.
 * Then they move stop by stop, following the connections, until they
 * reach the final stop and go wherever they need.
 */
struct path_stop {
    float x, y;
    vector<path_link> links;
    
    path_stop(
        float x = 0, float y = 0, vector<path_link> links = vector<path_link>()
    );
    bool has_link(path_stop* other_stop);
    void fix_pointers(area_data &a);
    void fix_nrs(area_data &a);
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
    float scale_x; //Texture scale, X...
    float scale_y; //and Y.
    float trans_x; //X translation...
    float trans_y; //and Y.
    float rot;     //Rotation.
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
    
    sector();
    void connect_edges(area_data &a, size_t s_nr);
    void fix_pointers(area_data &a);
    void clone(sector* new_sector);
    void get_texture_merge_sectors(sector** s1, sector** s2);
    ~sector();
};




/* ----------------------------------------------------------------------------
 * List of visual corrections to be done on a sector, AFTER the area
 * buffer images have been created. This is used for sectors that
 * can change appearance after the area is done loading.
 * These changes are drawn on top of the normal area.
 * So for instance, if you want a gate to begone, have the normal
 * area show no gate, and then if the gate is alive, specify a
 * "correction" to its sector, saying that it should be drawing
 * a gate.
 */
struct sector_correction {
    sector* sec;
    sector_texture_info new_texture;
    
    sector_correction(sector* sec);
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
    
    vertex(float x, float y);
    void connect_edges(area_data &a, size_t v_nr);
    void fix_pointers(area_data &a);
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
    unsigned char category;
    mob_type* type;
    
    float x, y;
    float angle;
    string vars;
    
    mob_gen(
        float x = 0, float y = 0,
        unsigned char category = MOB_CATEGORY_NONE, mob_type* type = NULL,
        float angle = 0, string vars = ""
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
    
    float x, y;  //X and Y of the center.
    float w, h;  //Width and height.
    float angle; //Rotation angle.
    unsigned char alpha; //Opacity.
    float sway_x; //Swaying is multiplied by this, horizontally.
    float sway_y; //And vertically.
    
    tree_shadow(
        float x = 0, float y = 0, float w = 100, float h = 100,
        float an = 0, unsigned char al = 255, string f = "",
        float sx = 1, float sy = 0
    );
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
    vector<sector_correction> sector_corrections;
    
    ALLEGRO_BITMAP* bg_bmp;
    string bg_bmp_file_name;
    float bg_bmp_zoom;
    float bg_dist;
    ALLEGRO_COLOR bg_color;
    string name;
    string subtitle;
    
    weather weather_condition;
    string weather_name;
    
    area_data();
    size_t find_edge_nr(const edge* e_ptr);
    size_t find_sector_nr(const sector* s_ptr);
    size_t find_vertex_nr(const vertex* v_ptr);
    void generate_blockmap();
    void generate_edges_blockmap(vector<edge*> &edges);
    void remove_vertex(const size_t v_nr);
    void remove_vertex(const vertex* v_ptr);
    void remove_edge(const size_t e_nr);
    void remove_edge(const edge* e_ptr);
    void remove_sector(const size_t s_nr);
    void remove_sector(const sector* s_ptr);
    void clear();
};




void check_edge_intersections(vertex* v);
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
vector<path_stop*> get_path(
    const float start_x, const float start_y,
    const float end_x, const float end_y,
    mob** obstacle_found, bool* go_straight, float* get_dist
);
mob* get_path_link_obstacle(path_stop* s1, path_stop* s2);
float get_point_sign(
    float x, float y, float lx1, float ly1, float lx2, float ly2
);
void get_polys(sector* s, polygon* outer, vector<polygon>* inners);
vertex* get_rightmost_vertex(map<edge*, bool> &edges);
vertex* get_rightmost_vertex(polygon* p);
vertex* get_rightmost_vertex(sector* s);
vertex* get_rightmost_vertex(vertex* v1, vertex* v2);
sector* get_sector(
    const float x, const float y, size_t* sector_nr, const bool use_blockmap
);
void get_sector_bounding_box(
    sector* s_ptr, float* min_x, float* min_y, float* max_x, float* max_y
);
void get_shadow_bounding_box(
    tree_shadow* s_ptr, float* min_x, float* min_y, float* max_x, float* max_y
);
bool is_path_link_ok(path_stop* s1, path_stop* s2);
bool is_vertex_convex(const vector<vertex> &vec, const size_t nr);
bool is_vertex_ear(
    const vector<vertex> &vec, const vector<size_t> &concaves, const size_t nr
);
bool is_point_in_sector(const float x, const float y, sector* s_ptr);
bool is_point_in_triangle(
    float px, float py,
    float tx1, float ty1, float tx2, float ty2, float tx3, float ty3,
    bool loq
);
bool lines_intersect(
    float l1x1, float l1y1, float l1x2, float l1y2,
    float l2x1, float l2y1, float l2x2, float l2y2,
    float* ur, float* ul
);
void triangulate(sector* s_ptr);



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
