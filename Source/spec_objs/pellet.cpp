#include "pellet.h"

pellet::pellet(float x, float y, sector* s, pikmin_type* type, unsigned short number)
: mob(x, y, s->floors[0].z, 0, s){

	this->type = type;
	this->number = number;
	this->weight = number;

	this->carrier_info = new carrier_info_struct(this, 10); //ToDo max carriers.
	size = 32;
}