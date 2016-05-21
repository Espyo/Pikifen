/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the bridge class and bridge-related functions.
 */

#ifndef BRIDGE_INCLUDED
#define BRIDGE_INCLUDED

#include "mob.h"

enum BRIDGE_ANIMATIONS {
    BRIDGE_ANIM_IDLE,
    BRIDGE_ANIM_NOTHING,
};


/* ----------------------------------------------------------------------------
 * A bridge mob. Bridges on the engine are made up of two parts:
 * the mob itself, which Pikmin damage, and the sectors Pikmin can walk on.
 * The sectors initially start as something else (normally ground at a
 * lower level, or some water), and when the bridge opens, they change
 * into walkable wood.
 */
class bridge : public mob {
private:

    void get_neighbor_bridge_sectors(sector* s_ptr);
    
public:
    vector<sector*> secs;
    
    bridge(const float x, const float y, const float angle, const string &vars);
    
    static void open(mob* m, void* info1, void* info2);
    static void take_damage(mob* m, void* info1, void* info2);
    static void set_anim(mob* m, void* info1, void* info2);
    
};


void init_bridge_mob_type(mob_type* mt);

#endif //ifndef BRIDGE_INCLUDED
