#ifndef ENEMY_INCLUDED
#define ENEMY_INCLUDED

#include "enemy_state.h"
#include "enemy_type.h"
#include "mob.h"

class enemy : public mob {
public:
    //Technical things.
    enemy_type* ene_type;
    
    //Behavior.
    float original_x, original_y;   //Original coordinates. When it wanders too far, it returns to this spot.
    
    //Spawn and respawn things.
    float spawn_delay; //Enemy only spawns after these many seconds, a la Waterwraith.
    unsigned char respawn_days_left;        //Days needed until it respawns.
    unsigned char respawns_after_x_days;
    unsigned int appears_after_day; //This enemy only appears from this day onwards.
    unsigned int appears_before_day;
    unsigned int appears_every_x_days;
    
    enemy(float x, float y, sector* s, enemy_type* type);
};

#endif //ifndef ENEMY_INCLUDED
