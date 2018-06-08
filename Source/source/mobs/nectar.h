/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the nectar class and nectar-related functions.
 */

#ifndef NECTAR_INCLUDED
#define NECTAR_INCLUDED

#include "mob.h"

/* ----------------------------------------------------------------------------
 * Nectar is a yellow blob lying on the ground.
 * When leaf/bud Pikmin touch it, they drink it
 * and instantly mature to flower.
 * There are two ways to make nectars work:
 * the classic way, in which a single Pikmin can
 * selfishly drink the whole thing, or the new
 * method, which allows X individual
 * Pikmin to drink it without it draining.
 */
class nectar : public mob {
public:
    unsigned char amount_left;
    
    nectar(const point &pos, const string &vars);
    
    virtual void draw(bitmap_effect_manager* effect_manager = NULL);
};

#endif //ifndef NECTAR_INCLUDED
