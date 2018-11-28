/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the pile type class and pile type-related functions.
 */

#ifndef PILE_TYPE_INCLUDED
#define PILE_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "../data_file.h"
#include "../misc_structs.h"
#include "mob_type.h"

enum PILE_ANIMATIONS {
    PILE_ANIM_IDLING,
};

/* ----------------------------------------------------------------------------
 * A type of resource pile (gold nugget pile, Burgeoning Spiderwort, etc.).
 */
class pile_type : public mob_type {
public:

    pile_type();
    ~pile_type();
    void load_parameters(data_node* file);
    anim_conversion_vector get_anim_conversions();
};

#endif //ifndef PILE_TYPE_INCLUDED
