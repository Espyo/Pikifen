/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the enemy class and enemy-related functions.
 */

#ifndef ENEMY_INCLUDED
#define ENEMY_INCLUDED

#include "../mob_types/enemy_type.h"
#include "mob.h"

/* ----------------------------------------------------------------------------
 * I don't need to explain what an enemy is.
 */
class enemy : public mob {
public:
    //Technical things.
    enemy_type* ene_type;
    
    //Spawn and respawn things.
    //Enemy only spawns after these many seconds, a la Waterwraith.
    float spawn_delay;
    //Days needed until it respawns.
    unsigned char respawn_days_left;
    unsigned char respawns_after_x_days;
    //This enemy only appears from this day onwards.
    unsigned int appears_after_day;
    unsigned int appears_before_day;
    unsigned int appears_every_x_days;
    
    enemy(const point &pos, enemy_type* type, const float angle);
    virtual void draw_mob(bitmap_effect_manager* effect_manager = NULL);
    virtual void read_script_vars(const string &vars);
    
    virtual bool can_receive_status(status_type* s);
    
};

#endif //ifndef ENEMY_INCLUDED
