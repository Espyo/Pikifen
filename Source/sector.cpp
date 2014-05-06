/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Sector, linedef, etc. classes, and related functions.
 */

#include <cfloat>

#include "sector.h"

/* ----------------------------------------------------------------------------
 * Creates a structure with floor information.
 */
floor_info::floor_info() {
    z = 0;
    scale = 0;
    trans_x = trans_y = 0;
    rot = 0;
}

/* ----------------------------------------------------------------------------
 * Creates a linedef.
 */
linedef::linedef(vertex* v1, vertex* v2, sector* fs, sector* bs) {
    this->vertex1 = v1; this->vertex2 = v2;
    this->front_sector = fs;
    this->back_sector = bs;
}

/* ----------------------------------------------------------------------------
 * Creates a sector.
 */
sector::sector() {
    type = SECTOR_TYPE_NORMAL;
}

/* ----------------------------------------------------------------------------
 * Clears the info on an area map.
 */
void area_map::clear() {
    vertices.clear();
    linedefs.clear();
    sectors.clear();
}