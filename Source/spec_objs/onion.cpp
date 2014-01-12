#include "onion.h"

onion::onion(float x, float y, sector* sec, onion_type* type)
    : mob(x, y, sec->floors[0].z, type, sec) {
    
    oni_type = type;
}