#include "treasure.h"

treasure::treasure(float x, float y, float z, float radius, float max_move_speed, sector* sec, unsigned int weight, unsigned int max_carriers)
: mob(x, y, z, max_move_speed, sec){
	this->weight = weight;
	this->max_carriers = max_carriers;
	size = radius;

	for(unsigned int s=0; s<max_carriers; s++){
		carrier_spots.push_back(NULL);
	}
}