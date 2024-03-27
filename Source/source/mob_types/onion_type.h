/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Onion type class and Onion type-related functions.
 */

#ifndef ONION_TYPE_INCLUDED
#define ONION_TYPE_INCLUDED

#include "../libs/data_file.h"
#include "../mobs/mob_utils.h"
#include "mob_type.h"
#include "pikmin_type.h"


//Onion object states.
enum ONION_STATES {
    
    //Idling.
    ONION_STATE_IDLING,
    
    //Total amount of Onion object states.
    N_ONION_STATES,
    
};


/**
 * @brief An Onion type.
 * It's basically associated with one or more Pikmin types.
 */
class onion_type : public mob_type {

public:

    //--- Members ---

    //Nest data.
    pikmin_nest_type_t* nest = nullptr;
    
    //Sound data index for the pop sound. Cache for performance.
    size_t sfx_pop_idx = INVALID;
    
    
    //--- Function declarations ---

    onion_type();
    ~onion_type();
    void load_properties(data_node* file) override;
    void load_resources(data_node* file) override;
    anim_conversion_vector get_anim_conversions() const override;
    
};


#endif //ifndef ONION_TYPE_INCLUDED
