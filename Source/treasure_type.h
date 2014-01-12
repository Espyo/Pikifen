#ifndef TREASURE_TYPE_INCLUDED
#define TREASURE_TYPE_INCLUDED

#include "mob_type.h"

class treasure_type : public mob_type {
public:
    unsigned int max_carriers;
    
    treasure_type(float size, unsigned int weight, unsigned int max_carriers) {
        this->size = size;
        this->weight = weight;
        this->max_carriers = max_carriers;
        move_speed = 60; //ToDo should this be here?
    }
};

#endif //ifndef TREASURE_TYPE_INCLUDED