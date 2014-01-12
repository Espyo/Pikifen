#include "pellet.h"

pellet::pellet(float x, float y, sector* s, pellet_type* type)
    : mob(x, y, s->floors[0].z, type, s) {
    
    pel_type = type;
    this->carrier_info = new carrier_info_struct(this, type->max_carriers, false);
}