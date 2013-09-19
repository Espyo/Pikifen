#ifndef ONION_INCLUDED
#define	ONION_INCLUDED

#include "mob.h"
#include "pikmin_type.h"

class onion : public mob{
public:
	pikmin_type* type;

	onion(float x, float y, sector *sec, pikmin_type* type);
};

#endif //ifndef ONION_INCLUDED