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
    explicit Polygon(const vector<Vertex*>& vertexes);
    void clean(bool recursive);
    void cut();
    void cutAllAsRoot();
    void destroy();
    Vertex* getRightmostVertex() const;
    bool insertChild(Polygon* p);
    bool isPointInside(const Point& p) const;
    
};


/**
 * @brief Info about the geometry problems the area currently has.
 */
struct GeometryProblems {

    //--- Members ---
    
    //Non-simple sectors found, and their reason for being broken.
    map<Sector*, TRIANGULATION_ERROR> nonSimples;
    
    //List of lone edges found.
    unordered_set<Edge*> loneEdges;
    
};


void findTraceEdge(
    Vertex* vptr, const Vertex* prevVPtr, const Sector* sPtr,
    float prevEAngle, bool bestIsClosestCw,
    Edge** nextEPtr, float* nextEAngle, Vertex** nextVPtr,
    unordered_set<Edge*>* excludedEdges
);
void getCCE(
    const vector<Vertex>& vertexesLeft, vector<size_t>& ears,
    vector<size_t>& convexVertexes, vector<size_t>& concaveVertexes
);
vector<std::pair<Distance, Vertex*> > getMergeVertexes(
    const Point& p, const vector<Vertex*>& allVertexes,
    float mergeRadius
);
TRIANGULATION_ERROR getPolys(
    Sector* sPtr, vector<Polygon>* outers, vector<vector<Polygon>>* inners
);
bool getPolysIsOuter(
    Vertex* vPtr, const Sector* sPtr, const unordered_set<Edge*>& edgesLeft,
    bool doingFirstPolygon
);
Vertex* getRightmostVertex(const unordered_set<Edge*>& edges);
Vertex* getRightmostVertex(Vertex* v1, Vertex* v2);
bool isPolygonClockwise(vector<Vertex*>& vertexes);
bool isVertexConvex(const vector<Vertex*>& vec, size_t idx);
bool isVertexEar(
    const vector<Vertex*>& vec, const vector<size_t>& concaves, size_t idx
);
TRIANGULATION_ERROR traceEdges(
    Vertex* startVPtr, const Sector* sPtr, bool goingCw,
    vector<Vertex*>* vertexes,
    unordered_set<Edge*>* unvisitedEdges,
    unordered_set<Edge*>* polygonEdgesSoFar
);
TRIANGULATION_ERROR triangulatePolygon(
    Polygon* poly, vector<Triangle>* triangles
);
TRIANGULATION_ERROR triangulateSector(
    Sector* s_ptr, set<Edge*>* loneEdges, bool clearLoneEdges
);
