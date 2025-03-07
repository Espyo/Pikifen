/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for area geometry-related functions.
 */

#pragma once

#include <map>
#include <set>
#include <unordered_set>
#include <vector>

#include <allegro5/allegro.h>

#include "../../util/geometry_utils.h"


using std::map;
using std::set;
using std::unordered_set;
using std::vector;


struct Edge;
struct Sector;
struct Vertex;


namespace GEOMETRY {
extern const float AREA_CELL_SIZE;
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
enum TRIANGULATION_ERROR {

    //No error occured.
    TRIANGULATION_ERROR_NONE,
    
    //Invalid arguments provided.
    TRIANGULATION_ERROR_INVALID_ARGS,
    
    //Non-simple sector: Sector is not closed.
    TRIANGULATION_ERROR_NOT_CLOSED,
    
    //Non-simple sector: Lone edges break the sector.
    TRIANGULATION_ERROR_LONE_EDGES,
    
    //Non-simple sector: Ran out of ears while triangulating.
    TRIANGULATION_ERROR_NO_EARS,
    
};


/**
 * @brief A triangle.
 *
 * Sectors (essentially polygons) are made out of triangles.
 * These are used to detect whether a point is inside a sector,
 * and to draw, seeing as OpenGL cannot draw concave polygons.
 */
struct Triangle {

    //--- Members ---
    
    //Points that make up this triangle.
    Vertex* points[3] = { nullptr, nullptr, nullptr };
    
    
    //--- Function declarations ---
    
    Triangle(Vertex* v1, Vertex* v2, Vertex* v3);
    
};


/**
 * @brief A polygon.
 *
 * Represents a series of vertexes that make up a plain old geometric polygon.
 * A polygon cannot have holes or islands.
 * Since a polygon can have children polygons, this is effectively a node
 * in a polygon tree. If it has no vertexes, then instead it represents the
 * root of said tree.
 */
struct Polygon {

    //--- Members ---
    
    //Ordered list of vertexes that represent the polygon.
    vector<Vertex*> vertexes;
    
    //Children, if any.
    vector<Polygon*> children;
    
    
    //--- Function declarations ---
    
    Polygon();
    explicit Polygon(const vector<Vertex*> &vertexes);
    void clean(bool recursive);
    void cut();
    void cut_all_as_root();
    void destroy();
    Vertex* get_rightmost_vertex() const;
    bool insert_child(Polygon* p);
    bool is_point_inside(const Point &p) const;
    
};


/**
 * @brief Info about the geometry problems the area currently has.
 */
struct GeometryProblems {

    //--- Members ---
    
    //Non-simple sectors found, and their reason for being broken.
    map<Sector*, TRIANGULATION_ERROR> non_simples;
    
    //List of lone edges found.
    unordered_set<Edge*> lone_edges;
    
};


void find_trace_edge(
    Vertex* v_ptr, const Vertex* prev_v_ptr, const Sector* s_ptr,
    float prev_e_angle, bool best_is_closest_cw,
    Edge** next_e_ptr, float* next_e_angle, Vertex** next_v_ptr,
    unordered_set<Edge*>* excluded_edges
);
void get_cce(
    const vector<Vertex> &vertexes_left, vector<size_t> &ears,
    vector<size_t> &convex_vertexes, vector<size_t> &concave_vertexes
);
vector<std::pair<Distance, Vertex*> > get_merge_vertexes(
    const Point &p, const vector<Vertex*> &all_vertexes,
    float merge_radius
);
TRIANGULATION_ERROR get_polys(
    Sector* s_ptr, vector<Polygon>* outers, vector<vector<Polygon>>* inners
);
bool get_polys_is_outer(
    Vertex* v_ptr, const Sector* s_ptr, const unordered_set<Edge*> &edges_left,
    bool doing_first_polygon
);
Vertex* get_rightmost_vertex(const unordered_set<Edge*> &edges);
Vertex* get_rightmost_vertex(Vertex* v1, Vertex* v2);
bool is_polygon_clockwise(vector<Vertex*> &vertexes);
bool is_vertex_convex(const vector<Vertex*> &vec, size_t idx);
bool is_vertex_ear(
    const vector<Vertex*> &vec, const vector<size_t> &concaves, size_t idx
);
TRIANGULATION_ERROR trace_edges(
    Vertex* start_v_ptr, const Sector* s_ptr, bool going_cw,
    vector<Vertex*>* vertexes,
    unordered_set<Edge*>* unvisited_edges,
    unordered_set<Edge*>* polygon_edges_so_far
);
TRIANGULATION_ERROR triangulate_polygon(
    Polygon* poly, vector<Triangle>* triangles
);
TRIANGULATION_ERROR triangulate_sector(
    Sector* s_ptr, set<Edge*>* lone_edges, bool clear_lone_edges
);
