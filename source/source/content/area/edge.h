/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the sector edge class and related functions.
 */

#pragma once

#include <allegro5/allegro_color.h>

#include "vertex.h"

#include "../../core/const.h"
#include "../../util/general_utils.h"
#include "geometry.h"


struct Sector;


/**
 * @brief A line segment that delimits a sector -- an edge of a polygon.
 * In DOOM, these are what's known as linedefs.
 */
struct Edge {

    //--- Members ---
    
    //Vertexes that make up the edge.
    Vertex* vertexes[2] = { nullptr, nullptr };
    
    //Index of the vertexes that make up the edge.
    size_t vertexIdxs[2] = { INVALID, INVALID };
    
    //Sectors on each side of the edge.
    Sector* sectors[2] = { nullptr, nullptr };
    
    //Index of the sectors on each side of the edge.
    size_t sectorIdxs[2] = { INVALID, INVALID };
    
    //Length of the wall shadow. 0 = none. LARGE_FLOAT = auto.
    float wallShadowLength = LARGE_FLOAT;
    
    //Color of the wall shadow, opacity included.
    ALLEGRO_COLOR wallShadowColor = GEOMETRY::SHADOW_DEF_COLOR;
    
    //Length of the ledge smoothing effect. 0 = none.
    float ledgeSmoothingLength = 0.0f;
    
    //Color of the ledge smoothing effect, opacity included.
    ALLEGRO_COLOR ledgeSmoothingColor = GEOMETRY::SMOOTHING_DEF_COLOR;
    
    
    //--- Function declarations ---
    
    explicit Edge(size_t v1Idx = INVALID, size_t v2Idx = INVALID);
    void clone(Edge* destination) const;
    Sector* getOtherSector(const Sector* vPtr) const;
    Vertex* getOtherVertex(const Vertex* vPtr) const;
    size_t getSideWithSector(const Sector* sPtr) const;
    Vertex* hasNeighbor(const Edge* other) const;
    bool isValid() const;
    size_t removeFromSectors();
    size_t removeFromVertexes();
    void swapVertexes();
    void transferSector(
        Sector* from, Sector* to, size_t toIdx, size_t edgeIdx
    );
    
};


/**
 * @brief Intersection between two edges.
 * Used to mark edges as red on the editor.
 */
struct EdgeIntersection {

    //--- Members ---
    
    //First edge in the intersection.
    Edge* e1 = nullptr;
    
    //Second edge in the intersection.
    Edge* e2 = nullptr;
    
    
    //--- Function declarations ---
    
    EdgeIntersection(Edge* e1, Edge* e2);
    bool contains(const Edge* e);
    
};
