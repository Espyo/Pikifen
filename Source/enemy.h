#ifndef ENEMY_INCLUDED
#define ENEMY_INCLUDED

#include "animation.h"
#include "enemy_type.h"
#include "mob.h"

class enemy : public mob {
public:
    //Technical things.
    enemy_type* ene_type;
    
    //Spawn and respawn things.
    float spawn_delay; //Enemy only spawns after these many seconds, a la Waterwraith.
    unsigned char respawn_days_left;        //Days needed until it respawns.
    unsigned char respawns_after_x_days;
    unsigned int appears_after_day; //This enemy only appears from this day onwards.
    unsigned int appears_before_day;
    unsigned int appears_every_x_days;
    
    enemy(const float x, const float y, sector* s, enemy_type* type);
};

#endif //ifndef ENEMY_INCLUDED
