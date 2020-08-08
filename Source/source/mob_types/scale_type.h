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


/* ----------------------------------------------------------------------------
 * A type of scale (seesaw block, crushable paper bag, etc.).
 */
class scale_type : public mob_type {
public:
    size_t goal_number;
    
    scale_type();
    void load_properties(data_node* file);
};


#endif //ifndef SCALE_TYPE_INCLUDED
