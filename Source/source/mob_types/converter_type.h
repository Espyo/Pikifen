/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the converter type class and converter type-related functions.
 */

#ifndef CONVERTER_TYPE_INCLUDED
#define CONVERTER_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "mob_type.h"

enum CONVERTER_ANIMATIONS {
    CONVERTER_ANIM_IDLING,
};


/* ----------------------------------------------------------------------------
 * A type of converter, which is a mob that can convert Pikmin from one type
 * to another.
 */
class converter_type : public mob_type {
public:


    converter_type();
    ~converter_type();
    void load_parameters(data_node* file);
    anim_conversion_vector get_anim_conversions();
};

#endif //ifndef CONVERTER_TYPE_INCLUDED
