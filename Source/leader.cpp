#include "const.h"
#include "leader.h"

leader::leader(float x, float y, sector* sec)
: mob(x, y, sec->floors[0].z, LEADER_MOVE_SPEED, sec){
	holding_pikmin = NULL;
	health = 10; //ToDo
	size = 32; //ToDo
}