/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the scale type class and scale type-related functions.
 */

#pragma once

#include "mob_type.h"


/**
 * @brief A type of scale (seesaw block, crushable paper bag, etc.).
 */
class ScaleType : public MobType {

public:

    //--- Members ---
    
    //Default weight number that must be met to reach a goal. 0 for none.
    size_t goal_number = 0;
    
    
    //--- Function declarations ---
    
    ScaleType();
    void load_cat_properties(DataNode* file) override;
    
};
