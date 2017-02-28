/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the treasure class and treasure-related functions.
 */

#ifndef TREASURE_INCLUDED
#define TREASURE_INCLUDED

#include "mob.h"
#include "pikmin.h"
#include "treasure_type.h"

enum TREASURE_STATES {
    TREASURE_STATE_IDLE_WAITING,
    TREASURE_STATE_IDLE_MOVING,
    TREASURE_STATE_BEING_DELIVERED,
    
    N_TREASURE_STATES,
};


/* ----------------------------------------------------------------------------
 * "Treasure" is the catch-all term for the
 * main collectible in the game.
 * Without it, you cannot complete the game,
 * and normally you need to collect them all;
 * collecting specific ones makes the story
 * move forward.
 * These are the ship parts in P, treasures
 * in P2, and fruits in P3.
 * They're called treasures because I had to
 * settle with some familiar name, and all
 * three types of major collectibles
 * in the canon games are very valuable, so...
 * Without the ship parts, Olimar would die,
 * without the treasures, the Freight would
 * go bankrupt (plus the treasures are worth
 * a monetary amount in and of themselves),
 * and without the fruits, Koppai would starve.
 */
class treasure : public mob {
public:

    treasure_type* tre_type;
    float buried; //0: fully unburried. 1: fully buried.
    
    treasure(
        const point pos, treasure_type* type,
        const float angle, const string &vars
    );
    void draw(sprite_effect_manager* effect_manager = NULL);
};

#endif //ifndef TREASURE_INCLUDED
