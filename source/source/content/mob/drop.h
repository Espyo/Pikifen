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
    DropType* droType = nullptr;
    
    //Current scale. Used for shrinking.
    float curScale = 1.0f;
    
    //How many doses are left.
    size_t dosesLeft = 0;
    
    
    //--- Function declarations ---
    
    Drop(const Point &pos, DropType* type, float angle);
    void drawMob() override;
    
protected:

    //--- Function declarations ---
    
    void tickClassSpecifics(float deltaT) override;
    
};
