/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the drop class and drop-related functions.
 */

#pragma once

#include "../mob_type/drop_type.h"
#include "mob.h"


/**
 * @brief A drop mob.
 *
 * This is a droplet that sits on the ground and can be consumed
 * by certain mobs. When that happens, some effect is triggered, depending
 * on what the drop is.
 */
class Drop : public Mob {

public:

    //--- Members ---
    
    //What type of drop it is.
    DropType* dro_type = nullptr;
    
    //Current scale. Used for shrinking.
    float cur_scale = 1.0f;
    
    //How many doses are left.
    size_t doses_left = 0;
    
    
    //--- Function declarations ---
    
    Drop(const Point &pos, DropType* dro_type, float angle);
    void draw_mob() override;
    
protected:

    //--- Function declarations ---
    
    void tick_class_specifics(float delta_t) override;
    
};
