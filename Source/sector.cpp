#include <cfloat>

#include "sector.h"

floor_info::floor_info(){
	z = 0;
	scale = 0;
	trans_x = trans_y = 0;
	rot = 0;
}

sector::sector(){
	type = SECTOR_TYPE_NORMAL;
}