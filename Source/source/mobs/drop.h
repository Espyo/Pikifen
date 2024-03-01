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


/**
 * @brief A drop mob.
 *
 * This is a droplet that sits on the ground and can be consumed
 * by certain mobs. When that happens, some effect is triggered, depending
 * on what the drop is.
 */
class drop : public mob {

public:
    
    //--- Members ---

    //What type of drop it is.
    drop_type* dro_type;
    
    //Current scale. Used for shrinking.
    float cur_scale;
    
    //How many doses are left.
    size_t doses_left;
    

    //--- Function declarations ---

    drop(const point &pos, drop_type* dro_type, const float angle);
    void draw_mob() override;
    
protected:
    
    //--- Function declarations ---
    
    void tick_class_specifics(const float delta_t) override;
    
};


#endif //ifndef DROP_INCLUDED
