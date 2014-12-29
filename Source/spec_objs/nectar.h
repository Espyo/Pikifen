/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the nectar class and nectar-related functions.
 */

#ifndef NECTAR_INCLUDED
#define NECTAR_INCLUDED

#include "../mob.h"

/*
 * Nectar is a yellow blob lying on the ground.
 * When leaf/bud Pikmin touch it, they drink it
 * and instantly mature to flower.
 * There are two ways to make nectars work:
 * the classic way, in which a single Pikmin can
 * selfishly drink the whole thing, or the new
 * method, which allows 4 individual
 * Pikmin to drink it without it draining.
 */
class nectar : public mob {
public:
    unsigned char amount_left;
    
    nectar(float x, float y, const string &vars);
};

#endif //ifndef NECTAR_INCLUDED
