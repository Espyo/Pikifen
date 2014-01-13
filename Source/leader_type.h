#ifndef LEADER_TYPE_INCLUDED
#define LEADER_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "misc_structs.h"
#include "mob_type.h"

class leader_type : public mob_type {
public:
    float whistle_range;
    unsigned int punch_strength;
    
    sample_struct sfx_whistle;
    sample_struct sfx_dismiss;
    sample_struct sfx_name_call;
    
    ALLEGRO_COLOR main_color;
};

#endif //ifndef LEADER_TYPE_INCLUDED