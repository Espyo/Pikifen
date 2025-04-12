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

    //--- Members ---
    
    //X coordinate.
    float x = 0.0f;
    
    //Y coordinate.
    float y = 0.0f;
    
    //Index number of the edges connected to it.
    vector<size_t> edgeIdxs;
    
    //Edges around it.
    vector<Edge*> edges;
    
    
    //--- Function declarations ---
    
    explicit Vertex(float x = 0.0f, float y = 0.0f);
    void addEdge(Edge* e_ptr, size_t e_idx);
    Edge* getEdgeByNeighbor(const Vertex* neighbor) const;
    bool hasEdge(const Edge* e_ptr) const;
    bool is2ndDegreeNeighbor(
        const Vertex* other_v, Vertex** first_neighbor
    ) const;
    bool is2ndDegreeNeighbor(
        const Edge* other_e, Vertex** first_neighbor
    ) const;
    bool isNeighbor(const Vertex* v_ptr) const;
    void removeEdge(const Edge* e_ptr);
    
};
