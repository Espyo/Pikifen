#ifndef SECTOR_INCLUDED
#define SECTOR_INCLUDED

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>

#include <vector>

#include "element.h"

using namespace std;



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



struct sector {
    floor_info floors[2];
    unsigned short type;
    unsigned int tag;
    vector<element*> elements;
    vector<size_t> linedefs;
    
    sector();
};



struct sub_sector {
    sector* s;
};



struct vertex {
    float x, y;
    vertex(float x, float y) {
        this->x = x; this->y = y;
    }
};



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



struct blockmap {
    float x1, y2;
    unsigned n_cols, n_rows;
    vector<vector<size_t> > linedefs_in_blocks;
};



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