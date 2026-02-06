/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the edge vertex class and related functions.
 */

#pragma once

#include <vector>


using std::size_t;
using std::vector;


struct Edge;


/**
 * @brief A 2D point, used to determine the end-points of an edge.
 *
 */
struct Vertex {

    //--- Public members ---
    
    //X coordinate.
    float x = 0.0f;
    
    //Y coordinate.
    float y = 0.0f;
    
    //Index number of the edges connected to it.
    vector<size_t> edgeIdxs;
    
    //Edges around it.
    vector<Edge*> edges;
    
    
    //--- Public function declarations ---
    
    explicit Vertex(float x = 0.0f, float y = 0.0f);
    void addEdge(Edge* ePtr, size_t eIdx);
    Edge* getEdgeByNeighbor(const Vertex* neighbor) const;
    bool hasEdge(const Edge* ePtr) const;
    bool is2ndDegreeNeighbor(
        const Vertex* otherV, Vertex** firstNeighbor
    ) const;
    bool is2ndDegreeNeighbor(
        const Edge* otherE, Vertex** firstNeighbor
    ) const;
    bool isNeighbor(const Vertex* vPtr) const;
    void removeEdge(const Edge* ePtr);
    
};
