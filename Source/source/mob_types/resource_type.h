/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the resource type class and resource type-related functions.
 */

#ifndef RESOURCE_TYPE_INCLUDED
#define RESOURCE_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "../data_file.h"
#include "../misc_structs.h"
#include "mob_type.h"

enum RESOURCE_ANIMATIONS {
    RESOURCE_ANIM_IDLING,
};

/* ----------------------------------------------------------------------------
 * A type of resource (gold nugget, bridge fragment, spray ingredient, etc.).
 */
class resource_type : public mob_type {
public:

    resource_type();
    ~resource_type();
    void load_parameters(data_node* file);
    anim_conversion_vector get_anim_conversions();
};

#endif //ifndef RESOURCE_TYPE_INCLUDED
