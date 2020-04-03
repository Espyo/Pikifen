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


/* ----------------------------------------------------------------------------
 * A type of "interactable" mob. This can be a readable sign, a switch, etc.
 */
class interactable_type : public mob_type {
public:
    string prompt_text;
    float trigger_range;
    
    interactable_type();
    
    void load_properties(data_node* file);
};

#endif //ifndef INTERACTABLE_TYPE_INCLUDED
