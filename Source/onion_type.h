/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Onion type class and Onion type-related functions.
 */

#ifndef ONION_TYPE_INCLUDED
#define ONION_TYPE_INCLUDED

#include "mob_type.h"
#include "pikmin_type.h"

/*
 * An Onion type.
 * It's basically associated with a
 * Pikmin type.
 */
class onion_type : public mob_type {
public:
    pikmin_type* pik_type;
};

#endif //ifndef ONION_TYPE_INCLUDED