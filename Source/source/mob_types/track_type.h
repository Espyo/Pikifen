/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the track type class and track type-related functions.
 */

#ifndef TRACK_TYPE_INCLUDED
#define TRACK_TYPE_INCLUDED

#include "../data_file.h"
#include "mob_type.h"

/* ----------------------------------------------------------------------------
 * A type of track. Something that transports the mob up, down, in a loop, etc.
 */
class track_type : public mob_type {
public:

    track_type();
    ~track_type();
    void load_parameters(data_node* file);
};

#endif //ifndef TRACK_TYPE_INCLUDED
