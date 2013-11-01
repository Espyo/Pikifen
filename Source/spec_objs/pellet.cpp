#include "pellet.h"

pellet::pellet(float x, float y, sector* s, pellet_type* pel_type, pikmin_type* pik_type)
: mob(x, y, s->floors[0].z, 80, s){

	this->pel_type = pel_type;
	this->pik_type = pik_type;
	
	this->size = pel_type->size;
	this->weight = pel_type->number;

	this->carrier_info = new carrier_info_struct(this, pel_type->max_carriers, false);
}