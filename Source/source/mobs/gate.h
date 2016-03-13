/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
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
#include "../sector.h"

using namespace std;

enum GATE_STATES {
    GATE_STATE_IDLE,
    GATE_STATE_DEAD,
    
    N_GATE_STATES
};


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
    
    gate(const float x, const float y, gate_type* type, const float angle, const string &vars);
    
};

#endif //ifndef GATE_INCLUDED
