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

#include "../const.h"
#include "../utils/general_utils.h"
#include "geometry.h"


struct sector;


/**
 * @brief A line segment that delimits a sector -- an edge of a polygon.
 * In DOOM, these are what's known as linedefs.
 */
struct edge {

    //--- Members ---
    
    //Vertexes that make up the edge.
    vertex* vertexes[2] = { nullptr, nullptr };
    
    //Index of the vertexes that make up the edge.
    size_t vertex_idxs[2] = { INVALID, INVALID };
    
    //Sectors on each side of the edge.
    sector* sectors[2] = { nullptr, nullptr };
    
    //Index of the sectors on each side of the edge.
    size_t sector_idxs[2] = { INVALID, INVALID };
    
    //Length of the wall shadow. 0 = none. LARGE_FLOAT = auto.
    float wall_shadow_length = LARGE_FLOAT;
    
    //Color of the wall shadow, opacity included.
    ALLEGRO_COLOR wall_shadow_color = GEOMETRY::SHADOW_DEF_COLOR;
    
    //Length of the ledge smoothing effect. 0 = none.
    float ledge_smoothing_length = 0.0f;
    
    //Color of the ledge smoothing effect, opacity included.
    ALLEGRO_COLOR ledge_smoothing_color = GEOMETRY::SMOOTHING_DEF_COLOR;
    
    
    //--- Function declarations ---
    
    explicit edge(size_t v1_idx = INVALID, size_t v2_idx = INVALID);
    void clone(edge* destination) const;
    sector* get_other_sector(const sector* v_ptr) const;
    vertex* get_other_vertex(const vertex* v_ptr) const;
    size_t get_side_with_sector(const sector* s_ptr) const;
    vertex* has_neighbor(const edge* other) const;
    bool is_valid() const;
    size_t remove_from_sectors();
    size_t remove_from_vertexes();
    void swap_vertexes();
    void transfer_sector(
        sector* from, sector* to, size_t to_idx, size_t edge_idx
    );
    
};


/**
 * @brief Intersection between two edges.
 * Used to mark edges as red on the editor.
 */
struct edge_intersection {

    //--- Members ---
    
    //First edge in the intersection.
    edge* e1 = nullptr;
    
    //Second edge in the intersection.
    edge* e2 = nullptr;
    
    
    //--- Function declarations ---
    
    edge_intersection(edge* e1, edge* e2);
    bool contains(const edge* e);
    
};
