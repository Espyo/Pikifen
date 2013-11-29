#ifndef SHIP_INCLUDED
#define SHIP_INCLUDED

#include "../mob.h"

class ship : public mob {
public:
    ship(float x, float y, sector* sec);
};

#endif //ifndef SHIP_INCLUDED