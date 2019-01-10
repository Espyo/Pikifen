/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the drop class and drop-related functions.
 */

#ifndef DROP_INCLUDED
#define DROP_INCLUDED

#include "../mob_types/drop_type.h"
#include "mob.h"


enum DROP_STATES {
    DROP_STATE_IDLING,
    DROP_STATE_FALLING,
    DROP_STATE_LANDING,
    DROP_STATE_BUMPED,
    
    N_DROP_STATES,
};


/* ----------------------------------------------------------------------------
 * A drop mob. This is a droplet that sits on the ground and can be consumed
 * by certain mobs. When that happens, some effect is triggered, depending
 * on what the drop is.
 */
class drop : public mob {
public:
    drop_type* dro_type;
    float cur_scale;
    size_t doses_left;
    
    drop(const point &pos, drop_type* dro_type, const float angle);
    
    virtual void draw_mob(bitmap_effect_manager* effect_manager = NULL);
    virtual void tick_class_specifics();
    
};

#endif //ifndef DROP_INCLUDED
