#ifndef PELLET_INCLUDED
#define PELLET_INCLUDED

#include "../mob.h"
#include "../pikmin_type.h"

class pellet : public mob{
public:
	pikmin_type* type;
	unsigned short number; //Number on the pellet, and hence, its weight.

	pellet(float x, float y, sector* s, pikmin_type* type, unsigned short number);
};

#endif //ifndef PELLET_INCLUDED