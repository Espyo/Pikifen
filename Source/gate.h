/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the gate class and gate-related functions.
 */

#ifndef GATE_INCLUDED
#define GATE_INCLUDED

#include <string>

#include "gate_type.h"
#include "mob.h"
#include "sector.h"

using namespace std;

/* ----------------------------------------------------------------------------
 * A gate is an invisible mob that Pikmin attack.
 * When the gate's health is all gone, its associated sector
 * (a wall-like obstacle) lowers, and allows passage.
 * To make sense, the area designer SHOULD put the gate inside
 * the sector, so it looks like the Pikmin are attacking the gate itself.
 */
class gate : public mob {
public:
    gate_type* gat_type;
    sector* sec;
    
    gate(const float x, const float y, gate_type* type, const string &vars);
    
    static void open(mob* m, void* info1, void* info2);
    static void take_damage(mob* m, void* info1, void* info2);
    static void set_anim(mob* m, void* info1, void* info2);
};

#endif //ifndef GATE_INCLUDED