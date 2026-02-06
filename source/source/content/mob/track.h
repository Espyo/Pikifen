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

#include "../mob_type/track_type.h"
#include "mob.h"


/**
 * @brief An object that moves others around as if on a track.
 */
class Track : public Mob {

public:

    //--- Public members ---
    
    //What type of track it is.
    TrackType* traType = nullptr;
    
    
    //--- Public function declarations ---
    
    Track(const Point& pos, TrackType* type, float angle);
    
};
