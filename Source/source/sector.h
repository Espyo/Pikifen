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
#include "mobs/mob_utils.h"
#include "pathing.h"
#include "weather.h"


using std::map;
using std::set;
using std::size_t;
using std::unordered_set;
using std::vector;


struct area_data;
struct blockmap;
struct edge;
struct sector;
struct triangle;
struct vertex;


namespace GEOMETRY {
extern const float BLOCKMAP_BLOCK_SIZE;
extern const unsigned char DEF_SECTOR_BRIGHTNESS;
extern const float STEP_HEIGHT;
extern const float LIQUID_DRAIN_DURATION;
extern const float SHADOW_AUTO_LENGTH_MULT;
extern const ALLEGRO_COLOR SHADOW_DEF_COLOR;
extern const float SHADOW_MAX_AUTO_LENGTH;
extern const float SHADOW_MAX_LENGTH;
extern const float SHADOW_MIN_AUTO_LENGTH;
extern const float SHADOW_MIN_LENGTH;
extern const ALLEGRO_COLOR SMOOTHING_DEF_COLOR;
extern const float SMOOTHING_MAX_LENGTH;
}


//Possible errors after a triangulation operation.
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


//Types of sector.
enum SECTOR_TYPES {
    //Normal sector.
    SECTOR_TYPE_NORMAL,
    //Blocks all mob movement.
    SECTOR_TYPE_BLOCKING,
};


/* ----------------------------------------------------------------------------
 * Intersection between two edges. Used to mark
 * edges as red on the editor.
 */
struct edge_intersection {
    //First edge in the intersection.
    edge* e1;
    //Second edge in the intersection.
    edge* e2;
    
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
    //Vertexes that make up the edge.
    vertex* vertexes[2];
    //Index of the vertexes that make up the edge.
    size_t vertex_nrs[2];
    //Sectors on each side of the edge.
    sector* sectors[2];
    //Index of the sectors on each side of the edge.
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
};




/* ----------------------------------------------------------------------------
 * Information about a sector's texture.
 */
struct sector_texture_info {
    //Texture scale.
    point scale;
    //Texture translation.
    point translation;
    //Texture rotation.
    float rot;
    //Texture bitmap.
    ALLEGRO_BITMAP* bitmap;
    //Texture tint.
    ALLEGRO_COLOR tint;
    //File name of the texture bitmap.
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
    //Its type.
    SECTOR_TYPES type;
    //Is it a bottomless pit?
    bool is_bottomless_pit;
    //Z coordinate of the floor.
    float z;
    //Extra information, if any.
    string tag;
    //Brightness.
    unsigned char brightness;
    //Information about its texture.
    sector_texture_info texture_info;
    //Is this sector meant to fade textures from neighboring sectors?
    bool fade;
    //String representing its hazards. Used for the editor.
    string hazards_str;
    //List of hazards.
    vector<hazard*> hazards;
    //Is only floor hazardous, or the air as well?
    bool hazard_floor;
    //Time left to drain the liquid in the sector.
    float liquid_drain_left;
    //Is it currently draining its liquid?
    bool draining_liquid;
    //Scrolling speed, if any.
    point scroll;
    //Index number of the edges that make up this sector.
    vector<size_t> edge_nrs;
    //Edges that make up this sector.
    vector<edge*> edges;
    //Triangles it is composed of.
    vector<triangle> triangles;
    //Bounding box.
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
    //Points that make up this triangle.
    vertex* points[3];
    
    triangle(vertex* v1, vertex* v2, vertex* v3);
};




/* ----------------------------------------------------------------------------
 * A vertex is a 2D point, used to determine
 * the end-points of an edge.
 */
struct vertex {
    //X coordinate.
    float x;
    //Y coordinate.
    float y;
    //Index number of the edges around it.
    vector<size_t> edge_nrs;
    //Edges around it.
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
    //Ordered list of vertexes that represent the polygon.
    vector<vertex*> vertexes;
    
    void clean();
    void cut(vector<polygon>* inners);
    vertex* get_rightmost_vertex() const;
    
    polygon();
    explicit polygon(const vector<vertex*> &vertexes);
};




/* ----------------------------------------------------------------------------
 * This structure holds the information for a mob's generation.
 * It is a mob's representation on the editor and in the area file,
 * but it doesn't have the data of a LIVING mob. This only holds its
 * position and type data, plus some other tiny things.
 */
struct mob_gen {
    //Mob category.
    mob_category* category;
    //Mob type.
    mob_type* type;
    //Position.
    point pos;
    //Angle.
    float angle;
    //Script vars.
    string vars;
    //Linked objects. Cache for performance.
    vector<mob_gen*> links;
    //Index of linked objects.
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
 * A structure holding info on the geometry problems the area currently has.
 */
struct geometry_problems {
    //Non-simple sectors found, and their reason for being broken.
    map<sector*, TRIANGULATION_ERRORS> non_simples;
    //List of lone edges found.
    unordered_set<edge*> lone_edges;
};




void get_cce(
    vector<vertex> &vertexes_left, vector<size_t> &ears,
    vector<size_t> &convex_vertexes, vector<size_t> &concave_vertexes
);
vector<std::pair<dist, vertex*> > get_merge_vertexes(
    const point &p, vector<vertex*> &all_vertexes, const float merge_radius
);
TRIANGULATION_ERRORS get_polys(
    sector* s, polygon* outer, vector<polygon>* inners,
    set<edge*>* lone_edges, const bool check_vertex_reuse
);
vertex* get_rightmost_vertex(const map<edge*, bool> &edges);
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

#endif //ifndef SECTOR_INCLUDED
