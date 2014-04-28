#ifndef LEADER_TYPE_INCLUDED
#define LEADER_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "misc_structs.h"
#include "mob_type.h"

class leader_type : public mob_type {
public:
    float whistle_range;
    unsigned int punch_strength;
    float pluck_delay; //Time until the Pikmin is actually popped out of the ground.
    
    sample_struct sfx_whistle;
    sample_struct sfx_dismiss;
    sample_struct sfx_name_call;
    
    ALLEGRO_COLOR main_color;
    
    leader_type() {
        whistle_range = DEF_WHISTLE_RANGE;
        punch_strength = DEF_PUNCH_STRENGTH;
        pluck_delay = 0.6;
        main_color = al_map_rgb(128, 128, 128);
    }
};

#endif //ifndef LEADER_TYPE_INCLUDED