#ifndef LEADER_TYPE_INCLUDED
#define LEADER_TYPE_INCLUDED

#include "mob_type.h"

class leader_type : public mob_type {
public:
    float whistle_range;
    unsigned int punch_strength;
};

#endif //ifndef LEADER_TYPE_INCLUDED