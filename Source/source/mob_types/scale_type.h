/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the scale type class and scale type-related functions.
 */

#ifndef SCALE_TYPE_INCLUDED
#define SCALE_TYPE_INCLUDED

#include "mob_type.h"


/**
 * @brief A type of scale (seesaw block, crushable paper bag, etc.).
 */
class scale_type : public mob_type {

public:
    
    //--- Members ---

    //Default weight number that must be met to reach a goal. 0 for none.
    size_t goal_number;
    

    //--- Function declarations ---
    
    scale_type();
    void load_properties(data_node* file) override;
    
};


#endif //ifndef SCALE_TYPE_INCLUDED
