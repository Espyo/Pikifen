/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the sector class and related functions.
 */

#pragma once

#include <functional>
#include <string>

#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>

#include "../hazard.h"
#include "../utils/geometry_utils.h"
#include "../utils/drawing_utils.h"
#include "geometry.h"
#include "edge.h"

using std::string;


//Types of sector.
enum SECTOR_TYPE {

    //Normal sector.
    SECTOR_TYPE_NORMAL,
    
    //Blocks all mob movement.
    SECTOR_TYPE_BLOCKING,
    
};


/**
 * @brief Info about a sector's texture.
 */
struct sector_texture_t {

    //--- Members ---
    
    //Texture scale.
    point scale = point(1.0f, 1.0f);
    
    //Texture translation.
    point translation;
    
    //Texture rotation.
    float rot = 0.0f;
    
    //Texture bitmap.
    ALLEGRO_BITMAP* bitmap = nullptr;
    
    //Texture tint.
    ALLEGRO_COLOR tint = COLOR_WHITE;
    
    //File name of the texture bitmap.
    string file_name;
    
};


/**
 * @brief A sector, like the ones in DOOM.
 *
 * It's composed of edges (linedefs), so it's essentially
 * a polygon (or multiple). It has a certain height, and its appearance
 * is determined by its floors.
 */
struct sector {

    //--- Members ---
    
    //Its type.
    SECTOR_TYPE type = SECTOR_TYPE_NORMAL;
    
    //Is it a bottomless pit?
    bool is_bottomless_pit = false;
    
    //Z coordinate of the floor.
    float z = 0.0f;
    
    //Extra information, if any.
    string tag;
    
    //Brightness.
    unsigned char brightness = GEOMETRY::DEF_SECTOR_BRIGHTNESS;
    
    //Information about its texture.
    sector_texture_t texture_info;
    
    //Is this sector meant to fade textures from neighboring sectors?
    bool fade = false;
    
    //String representing its hazards. Used for the editor.
    string hazards_str;
    
    //List of hazards.
    vector<hazard*> hazards;
    
    //Is only floor hazardous, or the air as well?
    bool hazard_floor = true;
    
    //Time left to drain the liquid in the sector.
    float liquid_drain_left = 0.0f;
    
    //Is it currently draining its liquid?
    bool draining_liquid = false;
    
    //Scrolling speed, if any.
    point scroll;
    
    //Index number of the edges that make up this sector.
    vector<size_t> edge_idxs;
    
    //Edges that make up this sector.
    vector<edge*> edges;
    
    //Triangles it is composed of.
    vector<triangle> triangles;
    
    //Bounding box.
    point bbox[2];
    
    
    //--- Function declarations ---
    
    ~sector();
    void add_edge(edge* e_ptr, size_t e_idx);
    void calculate_bounding_box();
    void clone(sector* destination) const;
    vertex* get_rightmost_vertex() const;
    void get_texture_merge_sectors(sector** s1, sector** s2) const;
    bool is_clockwise() const;
    bool is_point_in_sector(const point &p) const;
    void remove_edge(const edge* e_ptr);
    void get_neighbor_sectors_conditionally(
        const std::function<bool(sector* s_ptr)> &condition,
        vector<sector*> &sector_list
    );
    
};


sector* get_sector(
    const point &p, size_t* out_sector_idx, bool use_blockmap
);
