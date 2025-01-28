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


struct edge;


/**
 * @brief A 2D point, used to determine the end-points of an edge.
 *
 */
struct vertex {

    //--- Members ---
    
    //X coordinate.
    float x = 0.0f;
    
    //Y coordinate.
    float y = 0.0f;
    
    //Index number of the edges connected to it.
    vector<size_t> edge_idxs;
    
    //Edges around it.
    vector<edge*> edges;
    
    
    //--- Function declarations ---
    
    explicit vertex(float x = 0.0f, float y = 0.0f);
    void add_edge(edge* e_ptr, size_t e_idx);
    edge* get_edge_by_neighbor(const vertex* neighbor) const;
    bool has_edge(const edge* e_ptr) const;
    bool is_2nd_degree_neighbor(
        const vertex* other_v, vertex** first_neighbor
    ) const;
    bool is_2nd_degree_neighbor(
        const edge* other_e, vertex** first_neighbor
    ) const;
    bool is_neighbor(const vertex* v_ptr) const;
    void remove_edge(const edge* e_ptr);
    
};
