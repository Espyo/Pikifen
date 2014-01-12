#ifndef ONION_TYPE_INCLUDED
#define ONION_TYPE_INCLUDED

#include "mob_type.h"
#include "pikmin_type.h"

class onion_type : public mob_type {
public:
    pikmin_type* pik_type;
    
    onion_type(pikmin_type* pik_type) {
        this->pik_type = pik_type;
    }
};

#endif //ifndef ONION_TYPE_INCLUDED