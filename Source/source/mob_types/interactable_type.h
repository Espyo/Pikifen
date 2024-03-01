/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the interactable type class and
 * interactable type-related functions.
 */

#ifndef INTERACTABLE_TYPE_INCLUDED
#define INTERACTABLE_TYPE_INCLUDED

#include "mob_type.h"


/**
 * @brief A type of "interactable" mob. This can be a readable sign,
 * a switch, etc.
 */
class interactable_type : public mob_type {

public:
    
    //--- Members ---

    //Text to display above the mob, prompting the player on what to do.
    string prompt_text;
    
    //How close the leader must be before the player can interact with it.
    float trigger_range;
    

    //--- Function declarations ---

    interactable_type();
    void load_properties(data_node* file) override;
    
};


#endif //ifndef INTERACTABLE_TYPE_INCLUDED
