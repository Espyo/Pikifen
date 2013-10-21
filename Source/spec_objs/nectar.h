#ifndef NECTAR_INCLUDED
#define NECTAR_INCLUDED

#include "../mob.h"

class nectar : public mob{
public:
	unsigned char amount_left;

	nectar(float x, float y, sector* sec);
};

#endif //ifndef NECTAR_INCLUDED