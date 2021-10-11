/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the sector, edge, etc. classes and related functions.
 */

#ifndef SECTOR_INCLUDED
#define SECTOR_INCLUDED

#include <map>
#include <set>
#include <unordered_set>
#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "const.h"
#include "hazard.h"
#include "mob_categories/mob_category.h"
#include "mob_types/mob_type.h"
#include "weather.h"


using std::map;
using std::set;
using std::size_t;
using std::unordered_set;
using std::vector;


const float LIQUID_DRAIN_DURATION = 2.0f;

struct area_data;
struct blockmap;
struct edge;
struct path_stop;
struct path_link;
struct sector;
struct triangle;
struct vertex;

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

enum PATH_LINK_TYPES {
    //Normal.
    PATH_LINK_TYPE_NORMAL,
    //Only useable by mob scripts that reference it.
    PATH_LINK_TYPE_SCRIPT_ONLY,
    //Only for mobs carrying nothing, or a 1-weight mob.
    PATH_LINK_TYPE_LIGHT_LOAD_ONLY,
    //Only for objects that can fly.
    PATH_LINK_TYPE_AIRBORNE_ONLY,
};

enum PATH_TAKER_FLAGS {
    //The mob was told to use this path by a script.
    PATH_TAKER_FLAG_SCRIPT_USE = 1,
    //The mob has light load.
    PATH_TAKER_FLAG_LIGHT_LOAD = 2,
    //The mob can fly.
    PATH_TAKER_FLAG_AIRBORNE = 4,
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
 * A line segment that delimits a sector -- an edge of a polygon.
 * In DOOM, these are what's known as linedefs.
 */
struct edge {
    vertex* vertexes[2];
    size_t vertex_nrs[2];
    sector* sectors[2];
    size_t sector_nrs[2];
    
    //Length of the wall shadow. 0 = none. LARGE_FLOAT = auto.
    float wall_shadow_length;
    //Color of the wall shadow, opacity included.
    ALLEGRO_COLOR wall_shadow_color;
    
    //Length of the ledge smoothing effect. 0 = none.
    float ledge_smoothing_length;
    //Color of the ledge smoothing effect, opacity included.
    ALLEGRO_COLOR ledge_smoothing_color;
    
    edge(size_t v1_nr = INVALID, size_t v2_nr = INVALID);
    void clone(edge* new_edge) const;
    sector* get_other_sector(const sector* v_ptr) const;
    vertex* get_other_vertex(const vertex* v_ptr) const;
    size_t get_side_with_sector(sector* s_ptr) const;
    vertex* has_neighbor(edge* other) const;
    bool is_valid() const;
    size_t remove_from_sectors();
    size_t remove_from_vertexes();
    void swap_vertexes();
    void transfer_sector(
        sector* from, sector* to, const size_t to_nr, const size_t edge_nr
    );
    
    static const float SHADOW_AUTO_LENGTH_MULT;
    static const ALLEGRO_COLOR SHADOW_DEF_COLOR;
    static const float SHADOW_MAX_AUTO_LENGTH;
    static const float SHADOW_MAX_LENGTH;
    static const float SHADOW_MIN_AUTO_LENGTH;
    static const float SHADOW_MIN_LENGTH;
    static const ALLEGRO_COLOR SMOOTHING_DEF_COLOR;
    static const float SMOOTHING_MAX_LENGTH;
};




/* ----------------------------------------------------------------------------
 * Stops are points that make up a path. In mathematics, this is a node
 * in the graph. In a real-world example, this is a bus stop.
 * Pikmin start carrying by going for the closest stop.
 * Then they move stop by stop, following the connections, until they
 * reach the final stop and go wherever they need.
 */
struct path_stop {
    //Coordinates.
    point pos;
    //Links that go to other stops.
    vector<path_link*> links;
    //Sector it's on. Only applicable during gameplay. Cache for performance.
    sector* sector_ptr;
    
    path_stop(
        const point &pos = point(),
        const vector<path_link*> &links = vector<path_link*>()
    );
    ~path_stop();
    void add_link(path_stop* other_stop, const bool normal);
    path_link* get_link(path_stop* other_stop) const;
    void remove_link(path_link* link_ptr);
    void remove_link(path_stop* other_stop);
    void calculate_dists();
    void calculate_dists_plus_neighbors();
};




/* ----------------------------------------------------------------------------
 * Info about a path link. A path stop can link to N other path stops,
 * and this structure holds information about a connection.
 */
struct path_link {
    //Pointer to the path stop at the start.
    path_stop* start_ptr;
    //Pointer to the path stop at the end.
    path_stop* end_ptr;
    //Index number of the path stop at the end.
    size_t end_nr;
    
    //Type. Used for special restrictions and behaviors.
    PATH_LINK_TYPES type;
    //Its label, if any.
    string label;
    
    //Distance between the two stops.
    float distance;
    //Is the stop currently blocked by an obstacle? Cache for performance.
    bool blocked_by_obstacle;
    
    path_link(path_stop* start_ptr, path_stop* end_ptr, size_t end_nr);
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
 * A sector, like the ones in DOOM.
 * It's composed of edges (linedefs), so it's essentially
 * a polygon. It has a certain height, and its appearance
 * is determined by its floors.
 */
struct sector {
    unsigned char type;
    bool is_bottomless_pit;
    float z; //Height.
    string tag;
    unsigned char brightness;
    
    sector_texture_info texture_info;
    bool fade;
    
    string hazards_str; //For the editor.
    vector<hazard*> hazards; //For gameplay.
    bool hazard_floor;
    float liquid_drain_left;
    bool draining_liquid;
    point scroll;
    
    vector<size_t> edge_nrs;
    vector<edge*> edges;
    vector<triangle> triangles;
    
    point bbox[2];
    
    sector();
    void add_edge(edge* e_ptr, const size_t e_nr);
    void calculate_bounding_box();
    void clone(sector* new_sector);
    vertex* get_rightmost_vertex() const;
    void get_texture_merge_sectors(sector** s1, sector** s2) const;
    bool is_clockwise() const;
    bool is_point_in_sector(const point &p) const;
    void remove_edge(edge* e_ptr);
    void get_neighbor_sectors_conditionally(
        const std::function<bool(sector* s_ptr)> &condition,
        vector<sector*> &sector_list
    );
    ~sector();
};




/* ----------------------------------------------------------------------------
 * A triangle. Sectors (essentially polygons) are made out of triangles.
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
    edge* get_edge_by_neighbor(vertex* neighbor) const;
    bool has_edge(edge* e_ptr) const;
    bool is_2nd_degree_neighbor(vertex* other_v, vertex** first_neighbor) const;
    bool is_2nd_degree_neighbor(edge* other_e, vertex** first_neighbor) const;
    bool is_neighbor(vertex* v_ptr) const;
    void remove_edge(edge* e_ptr);
};




/* ----------------------------------------------------------------------------
 * Represents a series of vertexes that make up a polygon.
 */
struct polygon {
    vector<vertex*> vertexes;
    
    void clean();
    void cut(vector<polygon>* inners);
    vertex* get_rightmost_vertex() const;
    
    polygon();
    polygon(const vector<vertex*> &vertexes);
};




/* ----------------------------------------------------------------------------
 * This structure holds the information for a mob's generation.
 * It is a mob's representation on the editor and in the area file,
 * but it doesn't have the data of a LIVING mob. This only holds its
 * position and type data, plus some other tiny things.
 */
struct mob_gen {
    mob_category* category;
    mob_type* type;
    
    point pos;
    float angle;
    string vars;
    vector<mob_gen*> links; //Cache for performance.
    vector<size_t> link_nrs;
    
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
 * A structure holding info on the geometry problems the area currently has.
 */
struct geometry_problems {
    //Non-simple sectors found, and their reason for being broken.
    map<sector*, TRIANGULATION_ERRORS> non_simples;
    //List of lone edges found.
    unordered_set<edge*> lone_edges;
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
    
    string maker;
    string version;
    string notes;
    string spray_amounts;
    
    weather weather_condition;
    string weather_name;
    
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




/* ----------------------------------------------------------------------------
 * Manages the paths in the area. Particularly, this keeps an eye out on
 * what stops and links have any sort of obstacle in them that could deter
 * mobs. When these problems disappear, the manager is in charge of alerting
 * all mobs that were following paths, in order to get them recalculate
 * their paths if needed.
 * The reason we want them to recalculate regardless of whether the
 * obstacle affected them or not, is because this obstacle could've freed
 * a different route.
 */
struct path_manager {
    map<path_link*, unordered_set<mob*> > obstructions;
    unordered_set<path_stop*> hazardous_stops;
    
    void handle_area_load();
    void handle_obstacle_add(mob* m);
    void handle_obstacle_remove(mob* m);
    void handle_sector_hazard_change(sector* sector_ptr);
    void clear();
};




bool can_traverse_path_link(
    path_link* link_ptr, const bool ignore_obstacles,
    const vector<hazard*> &invulnerabilities, const unsigned char taker_flags,
    const string& label
);
void depth_first_search(
    vector<path_stop*> &nodes,
    unordered_set<path_stop*> &visited, path_stop* start
);
vector<path_stop*> dijkstra(
    path_stop* start_node, path_stop* end_node,
    const bool ignore_obstacles,
    const vector<hazard*> &invulnerabilities, const unsigned char taker_flags,
    const string &label,
    float* total_dist
);
void get_cce(
    vector<vertex> &vertexes_left, vector<size_t> &ears,
    vector<size_t> &convex_vertexes, vector<size_t> &concave_vertexes
);
vector<std::pair<dist, vertex*> > get_merge_vertexes(
    const point &p, vector<vertex*> &all_vertexes, const float merge_radius
);
vector<path_stop*> get_path(
    const point &start, const point &end,
    const vector<hazard*> invulnerabilities,
    const unsigned char taker_flags, const string &label,
    bool* go_straight, float* get_dist,
    path_stop** start_stop, path_stop** end_stop
);
mob* get_path_link_obstacle(path_stop* s1, path_stop* s2);
TRIANGULATION_ERRORS get_polys(
    sector* s, polygon* outer, vector<polygon>* inners,
    set<edge*>* lone_edges, const bool check_vertex_reuse
);
vertex* get_rightmost_vertex(map<edge*, bool> &edges);
vertex* get_rightmost_vertex(vertex* v1, vertex* v2);
sector* get_sector(
    const point &p, size_t* sector_nr, const bool use_blockmap
);
bool is_polygon_clockwise(vector<vertex*> &vertexes);
bool is_vertex_convex(const vector<vertex*> &vec, const size_t nr);
bool is_vertex_ear(
    const vector<vertex*> &vec, const vector<size_t> &concaves, const size_t nr
);
TRIANGULATION_ERRORS triangulate(
    sector* s_ptr, set<edge*>* lone_edges, const bool check_vertex_reuse,
    const bool clear_lone_edges
);


enum SECTOR_TYPES {
    SECTOR_TYPE_NORMAL,
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
