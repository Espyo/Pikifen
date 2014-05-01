#ifndef TREASURE_INCLUDED
#define TREASURE_INCLUDED

#include "mob.h"
#include "pikmin.h"
#include "treasure_type.h"

class treasure : public mob {
public:
    treasure(const float x, const float y, sector* sec, treasure_type* type);
};

#endif //ifndef TREASURE_INCLUDED