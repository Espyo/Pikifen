#include "../const.h"
#include "nectar.h"
#include "../vars.h"

nectar::nectar(float x, float y, sector* sec)
    : mob(x, y, sec->floors[0].z, nectar_mob_type, sec) {
    
    amount_left = NECTAR_AMOUNT;
}