#include "pellet_type.h"

pellet_type::pellet_type(float size, unsigned max_carriers,  unsigned number, unsigned match_seeds, unsigned non_match_seeds){
	this->size = size;
	this->max_carriers = max_carriers;
	this->number = number;
	this->match_seeds = match_seeds;
	this->non_match_seeds = non_match_seeds;
}