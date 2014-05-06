/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the sector, linedef, etc. classes and related functions.
 */

#ifndef SECTOR_INCLUDED
#define SECTOR_INCLUDED

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include <vector>

#include "element.h"

using namespace std;


/*
 * A structure with info on a floor,
 * like the texture.
 */
struct floor_info {
    float z;       //Height.
    float scale;   //Texture scale...
    float trans_x; //X translation...
    float trans_y; //Y translation...
    float rot;     //And rotation.
    float brightness;
    unsigned char terrain_sound; //Used for walking noises.
    ALLEGRO_BITMAP* texture;
    
    floor_info();
};


/*
 * A sector, like the ones in Doom.
 * It's composed of lines, so it's essentially
 * a polygon. It has a certain height, and its looks
 * is determined by its floors.
 */
struct sector {
    floor_info floors[2];
    unsigned short type;
    unsigned int tag;
    vector<element*> elements;
    vector<size_t> linedefs;
    
    sector();
};


/*
 * A sub-sector is a convex polygon, used
 * for drawing, seeing as OpenGL drawing doesn't
 * support concave polygons.
 * A sector is made of sub-sectors as a result.
 */
struct sub_sector {
    sector* s;
};


/*
 * A vertex is a 2D point, used to determine
 * the end-points of a linedef.
 */
struct vertex {
    float x, y;
    vertex(float x, float y) {
        this->x = x; this->y = y;
    }
};


/*
 * A line that delimits a sector.
 */
struct linedef {
    vertex* vertex1;
    vertex* vertex2;
    sector* front_sector;
    sector* back_sector;
    size_t vertex1_nr;
    size_t vertex2_nr;
    size_t front_sector_nr;
    size_t back_sector_nr;
    
    linedef(vertex* v1 = NULL, vertex* v2 = NULL, sector* fs = NULL, sector* bs = NULL);
};


/*
 * The blockmap divides the entire area
 * in a grid, so that collision detections only
 * happen between stuff in the same grid cell.
 * This is to avoid having, for instance, a Pikmin
 * on the lake part of TIS check for collisions with
 * a wall on the landing site part of TIS.
 */
struct blockmap {
    float x1, y2;
    unsigned n_cols, n_rows;
    vector<vector<size_t> > linedefs_in_blocks;
};


/*
 * A structure that holds all of the
 * info about the current area, so that
 * the sectors know how to communicate with
 * the linedefs, the linedefs with the
 * vertices, etc.
 */
struct area_map {
    blockmap bmap;
    vector<vertex> vertices;
    vector<linedef> linedefs;
    vector<sector> sectors;
    
    void clear();
};



enum SECTOR_TYPES {
    SECTOR_TYPE_NORMAL,
    SECTOR_TYPE_BOTTOMLESS_PIT,
    SECTOR_TYPE_BASE,
};



enum TERRAIN_SOUNDS {
    TERRAIN_SOUND_NONE,
    TERRAIN_SOUND_DIRT,
    TERRAIN_SOUND_GRASS,
    TERRAIN_SOUND_STONE,
    TERRAIN_SOUND_WOOD,
    TERRAIN_SOUND_METAL,
    TERRAIN_SOUND_WATER,
};



#endif //ifndef SECTOR_INCLUDED