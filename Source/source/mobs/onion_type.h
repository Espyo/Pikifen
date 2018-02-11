/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Onion type class and Onion type-related functions.
 */

#ifndef ONION_TYPE_INCLUDED
#define ONION_TYPE_INCLUDED

#include "../data_file.h"
#include "mob_type.h"
#include "pikmin_type.h"

/* ----------------------------------------------------------------------------
 * An Onion type.
 * It's basically associated with a Pikmin type.
 */
class onion_type : public mob_type {
public:
    pikmin_type* pik_type;
    
    onion_type();
    
    void load_parameters(data_node* file);
    anim_conversion_vector get_anim_conversions();
};

#endif //ifndef ONION_TYPE_INCLUDED
