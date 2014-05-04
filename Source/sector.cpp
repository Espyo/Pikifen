#include <cfloat>

#include "sector.h"

floor_info::floor_info() {
    z = 0;
    scale = 0;
    trans_x = trans_y = 0;
    rot = 0;
}

linedef::linedef(vertex* v1, vertex* v2, sector* fs, sector* bs) {
    this->vertex1 = v1; this->vertex2 = v2;
    this->front_sector = fs;
    this->back_sector = bs;
}

sector::sector() {
    type = SECTOR_TYPE_NORMAL;
}

void area_map::clear() {
    vertices.clear();
    linedefs.clear();
    sectors.clear();
}