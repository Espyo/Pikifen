#include "const.h"
#include "leader.h"

leader::leader(float x, float y, sector* sec) : mob(x, y, 0.0, LEADER_MOVE_SPEED, sec){
	holding_pikmin = NULL;
	health = 10; //ToDo
}