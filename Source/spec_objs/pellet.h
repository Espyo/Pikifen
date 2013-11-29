#ifndef PELLET_INCLUDED
#define PELLET_INCLUDED

#include "../mob.h"
#include "../pellet_type.h"
#include "../pikmin_type.h"

class pellet : public mob {
public:
    pellet_type* pel_type;
    pikmin_type* pik_type;
    
    pellet(float x, float y, sector* s, pellet_type* pel_type, pikmin_type* pik_type);
};

#endif //ifndef PELLET_INCLUDED