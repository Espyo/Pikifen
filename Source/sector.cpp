#include <cfloat>

#include "sector.h"

floor_info::floor_info() {
    z = 0;
    scale = 0;
    trans_x = trans_y = 0;
    rot = 0;
}

linedef::linedef(const float x1, const float y1, const float x2, const float y2, const unsigned fs, const unsigned bs) {
    this->x1 = x1; this->y1 = y1;
    this->x2 = x2; this->y2 = y2;
    this->front_sector = fs;
    this->back_sector = bs;
}

linedef::linedef() {
    x1 = y1 = x2 = y2 = 0;
    front_sector = UINT_MAX;
    back_sector = UINT_MAX;
}

sector::sector() {
    type = SECTOR_TYPE_NORMAL;
}