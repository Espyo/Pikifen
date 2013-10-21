#include "../const.h"
#include "nectar.h"

nectar::nectar(float x, float y, sector* sec)
: mob(x, y, sec->floors[0].z, 0, sec){
	amount_left = NECTAR_AMOUNT;

	size = 12;
}