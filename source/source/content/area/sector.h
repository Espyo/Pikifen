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

#include "../../util/drawing_utils.h"
#include "../../util/geometry_utils.h"
#include "../other/hazard.h"
#include "edge.h"
#include "geometry.h"


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
struct SectorTexture {

    //--- Members ---
    
    //Texture scale.
    Point scale = Point(1.0f);
    
    //Texture translation.
    Point translation;
    
    //Texture rotation.
    float rot = 0.0f;
    
    //Texture bitmap.
    ALLEGRO_BITMAP* bitmap = nullptr;
    
    //Texture tint.
    ALLEGRO_COLOR tint = COLOR_WHITE;
    
    //Internal name of the texture bitmap.
    string bmpName;
    
};


/**
 * @brief A sector, like the ones in DOOM.
 *
 * It's composed of edges (linedefs), so it's essentially
 * a polygon (or multiple). It has a certain height, and its appearance
 * is determined by its floors.
 */
struct Sector {

    //--- Members ---
    
    //Its type.
    SECTOR_TYPE type = SECTOR_TYPE_NORMAL;
    
    //Is it a bottomless pit?
    bool isBottomlessPit = false;
    
    //Z coordinate of the floor.
    float z = 0.0f;
    
    //Extra information, if any.
    string tag;
    
    //Brightness.
    unsigned char brightness = GEOMETRY::DEF_SECTOR_BRIGHTNESS;
    
    //Information about its texture.
    SectorTexture textureInfo;
    
    //Is this sector meant to fade textures from neighboring sectors?
    bool fade = false;
    
    //Hazard, if any.
    Hazard* hazard = nullptr;
    
    //Is only floor hazardous, or the air as well?
    bool hazardFloor = true;
    
    //Time left to drain the liquid in the sector.
    float liquidDrainLeft = 0.0f;
    
    //Is it currently draining its liquid?
    bool drainingLiquid = false;
    
    //Scrolling speed, if any.
    Point scroll;
    
    //Index number of the edges that make up this sector.
    vector<size_t> edgeIdxs;
    
    //Edges that make up this sector.
    vector<Edge*> edges;
    
    //Triangles it is composed of.
    vector<Triangle> triangles;
    
    //Bounding box.
    Point bbox[2];
    
    
    //--- Function declarations ---
    
    ~Sector();
    void addEdge(Edge* ePtr, size_t eIdx);
    void calculateBoundingBox();
    void clone(Sector* destination) const;
    Vertex* getRightmostVertex() const;
    void getTextureMergeSectors(Sector** s1, Sector** s2) const;
    bool isClockwise() const;
    bool isPointInSector(const Point &p) const;
    void removeEdge(const Edge* ePtr);
    void getNeighborSectorsConditionally(
        const std::function<bool(Sector* s_ptr)> &condition,
        vector<Sector*> &sectorList
    );
    
};


Sector* getSector(
    const Point &p, size_t* outSectorIdx, bool useBlockmap
);
