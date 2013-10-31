#include "pellet.h"

pellet::pellet(float x, float y, sector* s, pikmin_type* type, float size, unsigned short number, unsigned short max_carriers, unsigned short match_seeds, unsigned short non_match_seeds)
: mob(x, y, s->floors[0].z, 80, s){

	this->type = type;
	this->number = number;
	this->weight = number;
	this->match_seeds = match_seeds;
	this->non_match_seeds = non_match_seeds;
	this->size = size;

	this->carrier_info = new carrier_info_struct(this, max_carriers, false);
}