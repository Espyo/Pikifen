#ifndef PELLET_TYPE_INCLUDED
#define PELLET_TYPE_INCLUDED

#include "mob_type.h"
#include "pikmin_type.h"

class pellet_type : public mob_type {
public:
    pikmin_type* pik_type;
    unsigned number; //Number on the pellet, and hence, its weight.
    unsigned match_seeds; //Number of seeds given out if the pellet's taken to a matching Onion.
    unsigned non_match_seeds; //Number of seeds given out if the pellet's taken to a non-matching Onion.
    
    pellet_type();
};

#endif //ifndef PELLET_TYPE_INCLUDED