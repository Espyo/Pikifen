/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the track class and track-related functions.
 */

#pragma once

#include <vector>

#include "../mob_types/track_type.h"
#include "mob.h"


/**
 * @brief An object that moves others around as if on a track.
 */
class track : public mob {

public:
    
    //--- Members ---

    //What type of track it is.
    track_type* tra_type = nullptr;
    
    
    //--- Function declarations ---

    track(const point &pos, track_type* type, float angle);
    
};
