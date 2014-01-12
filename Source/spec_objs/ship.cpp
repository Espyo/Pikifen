#include "ship.h"
#include "../vars.h"

ship::ship(float x, float y, sector* sec)
    : mob(x, y, sec->floors[0].z, ship_mob_type, sec) {
    
}