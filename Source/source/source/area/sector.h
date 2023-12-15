/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the sector class and related functions.
 */

#ifndef SECTOR_INCLUDED
#define SECTOR_INCLUDED


#include <functional>
#include <string>

#include <allegro5/allegro.h>
#include <allegro5/allegro_color.h>

#include "../hazard.h"
#include "../utils/geometry_utils.h"
#include "geometry.h"
#include "edge.h"

using std::string;


//Types of sector.
enum SECTOR_TYPES {
    //Normal sector.
    SECTOR_TYPE_NORMAL,
    //Blocks all mob movement.
    SECTOR_TYPE_BLOCKING,
};


/* ----------------------------------------------------------------------------
 * Information about a sector's texture.
 */
struct sector_texture_info {
    //Texture scale.
    point scale;
    //Texture translation.
    point translation;
    //Texture rotation.
    float rot;
    //Texture bitmap.
    ALLEGRO_BITMAP* bitmap;
    //Texture tint.
    ALLEGRO_COLOR tint;
    //File name of the texture bitmap.
    string file_name;
    
    sector_texture_info();
};


/* ----------------------------------------------------------------------------
 * A sector, like the ones in DOOM.
 * It's composed of edges (linedefs), so it's essentially
 * a polygon (or multiple). It has a certain height, and its appearance
 * is determined by its floors.
 */
struct sector {
    //Its type.
    SECTOR_TYPES type;
    //Is it a bottomless pit?
    bool is_bottomless_pit;
    //Z coordinate of the floor.
    float z;
    //Extra information, if any.
    string tag;
    //Brightness.
    unsigned char brightness;
    //Information about its texture.
    sector_texture_info texture_info;
    //Is this sector meant to fade textures from neighboring sectors?
    bool fade;
    //String representing its hazards. Used for the editor.
    string hazards_str;
    //List of hazards.
    vector<hazard*> hazards;
    //Is only floor hazardous, or the air as well?
    bool hazard_floor;
    //Time left to drain the liquid in the sector.
    float liquid_drain_left;
    //Is it currently draining its liquid?
    bool draining_liquid;
    //Scrolling speed, if any.
    point scroll;
    //Index number of the edges that make up this sector.
    vector<size_t> edge_nrs;
    //Edges that make up this sector.
    vector<edge*> edges;
    //Triangles it is composed of.
    vector<triangle> triangles;
    //Bounding box.
    point bbox[2];
    
    sector();
    void add_edge(edge* e_ptr, const size_t e_nr);
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
    ~sector();
};


sector* get_sector(
    const point &p, size_t* sector_nr, const bool use_blockmap
);

#endif //ifndef SECTOR_INCLUDED
