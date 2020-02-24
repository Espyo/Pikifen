/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the track class and track-related functions.
 */

#ifndef TRACK_INCLUDED
#define TRACK_INCLUDED

#include <vector>

#include "../mob_types/track_type.h"
#include "mob.h"


/* ----------------------------------------------------------------------------
 * An object that moves others around as if on a track.
 */
class track : public mob {
public:
    //What type of track it is.
    track_type* tra_type;
    
    //Constructor.
    track(const point &pos, track_type* type, const float angle);
};

#endif //ifndef TRACK_INCLUDED
