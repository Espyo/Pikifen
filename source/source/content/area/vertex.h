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
    vector<size_t> edge_idxs;
    
    //Edges around it.
    vector<Edge*> edges;
    
    
    //--- Function declarations ---
    
    explicit Vertex(float x = 0.0f, float y = 0.0f);
    void add_edge(Edge* e_ptr, size_t e_idx);
    Edge* get_edge_by_neighbor(const Vertex* neighbor) const;
    bool has_edge(const Edge* e_ptr) const;
    bool is_2nd_degree_neighbor(
        const Vertex* other_v, Vertex** first_neighbor
    ) const;
    bool is_2nd_degree_neighbor(
        const Edge* other_e, Vertex** first_neighbor
    ) const;
    bool is_neighbor(const Vertex* v_ptr) const;
    void remove_edge(const Edge* e_ptr);
    
};
