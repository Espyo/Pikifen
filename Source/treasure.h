#ifndef TREASURE_INCLUDED
#define TREASURE_INCLUDED

#include "mob.h"
#include "pikmin.h"

class treasure : public mob{
public:
	treasure(float x, float y, float z, float radius, float max_move_speed, sector* sec, unsigned int weight, unsigned int max_carriers);
};

#endif //ifndef TREASURE_INCLUDED