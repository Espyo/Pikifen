#ifndef TREASURE_INCLUDED
#define TREASURE_INCLUDED

#include "mob.h"
#include "pikmin.h"

class treasure : public mob{
public:
	treasure(float x, float y, float radius, sector* sec, unsigned int weight, unsigned int max_carriers);
};

#endif //ifndef TREASURE_INCLUDED