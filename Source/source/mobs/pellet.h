/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the pellet class and pellet-related functions.
 */

#ifndef PELLET_INCLUDED
#define PELLET_INCLUDED

#include "../mob_types/pellet_type.h"
#include "../mob_types/pikmin_type.h"
#include "mob.h"


/* ----------------------------------------------------------------------------
 * A pellet can be delivered to an Onion in
 * order to generate more Pikmin.
 * Delivering a pellet to the matching Onion
 * results in more Pikmin being created.
 */
class pellet : public mob {
public:
    pellet_type* pel_type;
    
    pellet(const point &pos, pellet_type* type, const float angle);
    virtual void draw_mob(bitmap_effect_manager* effect_manager = NULL);
};

#endif //ifndef PELLET_INCLUDED
