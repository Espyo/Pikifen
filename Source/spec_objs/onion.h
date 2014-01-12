#ifndef ONION_INCLUDED
#define ONION_INCLUDED

#include "../mob.h"
#include "../pikmin_type.h"
#include "../onion_type.h"

class onion : public mob {
public:
    onion_type* oni_type;
    
    onion(float x, float y, sector* sec, onion_type* type);
};

#endif //ifndef ONION_INCLUDED