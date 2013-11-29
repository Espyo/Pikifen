#ifndef PELLET_TYPE_INCLUDED
#define PELLET_TYPE_INCLUDED

class pellet_type {
public:
    float size;
    unsigned max_carriers; //Maximum number of Pikmin that can carry it.
    unsigned number; //Number on the pellet, and hence, its weight.
    unsigned match_seeds; //Number of seeds given out if the pellet's taken to a matching Onion.
    unsigned non_match_seeds; //Number of seeds given out if the pellet's taken to a non-matching Onion.
    
    pellet_type(float size, unsigned max_carriers, unsigned number, unsigned match_seeds, unsigned non_match_seeds);
};

#endif //ifndef PELLET_TYPE_INCLUDED