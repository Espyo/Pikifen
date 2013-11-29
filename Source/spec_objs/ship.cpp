#include "ship.h"

ship::ship(float x, float y, sector* sec)
    : mob(x, y, sec->floors[0].z, 0, sec) {
    size = 140;
}