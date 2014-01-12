#ifndef TREASURE_INCLUDED
#define TREASURE_INCLUDED

#include "mob.h"
#include "pikmin.h"
#include "treasure_type.h"

class treasure : public mob {
public:
    treasure(float x, float y, sector* sec, treasure_type* type);
};

#endif //ifndef TREASURE_INCLUDED