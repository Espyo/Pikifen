#ifndef SECTOR_INCLUDED
#define SECTOR_INCLUDED

#include <allegro5\allegro.h>
#include <allegro5\allegro_image.h>

#include <vector>

#include "element.h"

using namespace std;

class floor_info{
public:
	float z;
	float scale;
	float trans_x;
	float trans_y;
	float rot;
	unsigned char terrain_sound; //Used for walking noises.
	ALLEGRO_BITMAP* texture;
	
	floor_info();
};

class sector{
public:
	floor_info floors[2];
	unsigned short type;
	vector<element> elements;

	sector();
};

enum SECTOR_TYPES{
	SECTOR_TYPE_NORMAL,
	SECTOR_TYPE_BOTTOMLESS_PIT,
	SECTOR_TYPE_BASE,
};

enum TERRAIN_SOUNDS{
	TERRAIN_SOUND_NONE,
	TERRAIN_SOUND_DIRT,
	TERRAIN_SOUND_GRASS,
	TERRAIN_SOUND_STONE,
	TERRAIN_SOUND_WOOD,
	TERRAIN_SOUND_METAL,
	TERRAIN_SOUND_WATER,
};

#endif //ifndef SECTOR_INCLUDED