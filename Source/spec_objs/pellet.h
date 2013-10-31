#ifndef PELLET_INCLUDED
#define PELLET_INCLUDED

#include "../mob.h"
#include "../pikmin_type.h"

class pellet : public mob{
public:
	pikmin_type* type;
	unsigned short number; //Number on the pellet, and hence, its weight.
	unsigned short match_seeds; //Number of seeds given out if the pellet's taken to a matching Onion.
	unsigned short non_match_seeds; //Number of seeds given out if the pellet's taken to a non-matching Onion.

	pellet(float x, float y, sector* s, pikmin_type* type, float size, unsigned short number, unsigned short max_carriers, unsigned short match_seeds, unsigned short non_match_seeds);
};

#endif //ifndef PELLET_INCLUDED