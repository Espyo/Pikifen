#include "treasure.h"

treasure::treasure(float x, float y, float radius, sector* sec, unsigned int weight, unsigned int max_carriers)
: mob(x, y, sec->floors[0].z, 20, sec){
	this->weight = weight;
	this->max_carriers = max_carriers;
	size = radius;

	carrier_info = new carrier_info_struct(this);
}