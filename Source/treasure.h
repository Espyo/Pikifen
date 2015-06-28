/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
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

    float buried; //0: fully unburried. 1: fully buried.
    
    treasure(const float x, const float y, treasure_type* type, const float angle, const string &vars);
};

#endif //ifndef TREASURE_INCLUDED
