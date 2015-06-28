/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the sector, linedef, etc. classes and related functions.
 */

#ifndef SECTOR_INCLUDED
#define SECTOR_INCLUDED

#include <set>
#include <unordered_set>
#include <vector>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_primitives.h>

#include "hazard.h"
#include "mob_type.h"
#include "weather.h"

using namespace std;


struct area_map;
struct blockmap;
struct linedef;
struct sector;
struct sector_texture;
struct triangle;
struct vertex;
typedef vector<vertex*> polygon;


/* ----------------------------------------------------------------------------
 * Intersection between two lines. Used to mark
 * linedefs as red on the editor.
 */
struct linedef_intersection {
    linedef* l1, *l2;
    linedef_intersection(linedef* l1, linedef* l2);
    bool contains(linedef* l);
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
    float x1, y1; //Top-left corner of the blockmap.
    vector<vector<vector<linedef*> > > linedefs; //Specifies a list of linedefs in each block.
    vector<vector<unordered_set<sector*> > >  sectors;  //Specifies a list of sectors in each block. A block must have at least one sector.
    size_t n_cols, n_rows;
    
    blockmap();
    size_t get_col(const float x);
    size_t get_row(const float y);
    float get_x1(const size_t col);
    float get_y1(const size_t row);
    void clear();
};



/* ----------------------------------------------------------------------------
 * A line that delimits a sector.
 */
struct linedef {
    vertex* vertices[2];
    size_t vertex_nrs[2];
    sector* sectors[2];
    size_t sector_nrs[2];
    
    linedef(size_t v1 = string::npos, size_t v2 = string::npos);
    void fix_pointers(area_map &a);
    size_t remove_from_sectors();
    size_t remove_from_vertices();
};



/* ----------------------------------------------------------------------------
 * A sector, like the ones in Doom.
 * It's composed of lines, so it's essentially
 * a polygon. It has a certain height, and its looks
 * is determined by its floors.
 */
struct sector {
    unsigned char type;
    float z; //Height.
    unsigned int tag; //TODO are these used?
    unsigned char brightness;
    
    float scale_x; //Texture scale, X...
    float scale_y; //and Y.
    float trans_x; //X translation...
    float trans_y; //and Y.
    float rot;     //Rotation.
    ALLEGRO_BITMAP* bitmap;
    string file_name;
    bool fade;
    bool always_cast_shadow;
    
    vector<hazard*> elements;
    vector<size_t> linedef_nrs;
    vector<linedef*> linedefs;
    vector<triangle> triangles;
    
    sector();
    void connect_linedefs(area_map &a, size_t s_nr);
    void fix_pointers(area_map &a);
    void clone(sector* new_sector);
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
 * the end-points of a linedef.
 */
struct vertex {
    float x, y;
    vector<size_t> linedef_nrs;
    vector<linedef*> linedefs;
    
    vertex(float x, float y);
    void connect_linedefs(area_map &a, size_t v_nr);
    void fix_pointers(area_map &a);
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
    
    mob_gen(float x = 0, float y = 0, unsigned char category = MOB_CATEGORY_NONE, mob_type* type = NULL, float angle = 0, string vars = "");
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
    
    tree_shadow(float x = 0, float y = 0, float w = 100, float h = 100, float an = 0, unsigned char al = 255, string f = "", float sx = 1, float sy = 0);
};



/* ----------------------------------------------------------------------------
 * A structure that holds all of the
 * info about the current area, so that
 * the sectors know how to communicate with
 * the linedefs, the linedefs with the
 * vertices, etc.
 */
struct area_map {
    blockmap bmap;
    vector<vertex*> vertices;
    vector<linedef*> linedefs;
    vector<sector*> sectors;
    vector<mob_gen*> mob_generators;
    vector<tree_shadow*> tree_shadows;
    
    ALLEGRO_BITMAP* bg_bmp;
    string bg_bmp_file_name;
    float bg_bmp_zoom;
    float bg_dist;
    ALLEGRO_COLOR bg_color;
    
    weather weather_condition;
    string weather_name;
    
    area_map();
    void generate_blockmap();
    void clear();
};



void check_linedef_intersections(vertex* v);
void clean_poly(polygon* p);
void cut_poly(polygon* outer, vector<polygon>* inners);
float get_angle_cw_dif(float a1, float a2);
float get_angle_smallest_dif(float a1, float a2);
void get_cce(vector<vertex> &vertices_left, vector<size_t> &ears, vector<size_t> &convex_vertices, vector<size_t> &concave_vertices);
float get_point_sign(float x, float y, float lx1, float ly1, float lx2, float ly2);
void get_polys(sector* s, polygon* outer, vector<polygon>* inners);
vertex* get_rightmost_vertex(map<linedef*, bool> &lines);
vertex* get_rightmost_vertex(polygon* p);
vertex* get_rightmost_vertex(vertex* v1, vertex* v2);
sector* get_sector(const float x, const float y, size_t* sector_nr, const bool use_blockmap);
void get_sector_bounding_box(sector* s_ptr, float* min_x, float* min_y, float* max_x, float* max_y);
void get_shadow_bounding_box(tree_shadow* s_ptr, float* min_x, float* min_y, float* max_x, float* max_y);
bool is_vertex_convex(const vector<vertex> &vec, const size_t nr);
bool is_vertex_ear(const vector<vertex> &vec, const vector<size_t> &concaves, const size_t nr);
bool is_point_in_sector(const float x, const float y, sector* s_ptr);
bool is_point_in_triangle(float px, float py, float tx1, float ty1, float tx2, float ty2, float tx3, float ty3, bool loq);
bool lines_intersect(float l1x1, float l1y1, float l1x2, float l1y2, float l2x1, float l2y1, float l2x2, float l2y2, float* ur, float* ul);
void triangulate(sector* s_ptr);



enum SECTOR_TYPES {
    SECTOR_TYPE_NORMAL,
    SECTOR_TYPE_BOTTOMLESS_PIT,
    SECTOR_TYPE_LANDING_SITE,
    SECTOR_TYPE_WALL,
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
const float SECTOR_STEP = 50; //Mobs can walk up sectors that are, at the most, this high from the current one, as if climbing up steps.

#endif //ifndef SECTOR_INCLUDED
