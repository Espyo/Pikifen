/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the enemy class and enemy-related functions.
 */

#ifndef ENEMY_INCLUDED
#define ENEMY_INCLUDED

#include "animation.h"
#include "enemy_type.h"
#include "mob.h"

/* ----------------------------------------------------------------------------
 * I don't need to explain what an enemy is.
 */
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
    
    enemy(const float x, const float y, enemy_type* type, const float angle, const string &vars);
    void draw();
};

#endif //ifndef ENEMY_INCLUDED
