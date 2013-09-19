#include "onion.h"

onion::onion(float x, float y, sector *sec, pikmin_type* type)
: mob(x, y, sec->floors[0].z, 0, sec){
	this->type = type;
	size = 32;  //ToDo fix this?
}