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

enum TRACK_RIDERS {
    TRACK_RIDER_PIKMIN = 1,
    TRACK_RIDER_LEADERS = 2,
};

enum TRACK_RIDING_POSES {
    TRACK_RIDING_POSE_STOPPED,
    TRACK_RIDING_POSE_CLIMBING,
    TRACK_RIDING_POSE_SLIDING,
};

enum TRACK_ANIMATIONS {
    TRACK_ANIM_IDLING,
};

enum TRACK_STATES {
    TRACK_STATE_IDLING,
    N_TRACK_STATES,
};


/* ----------------------------------------------------------------------------
 * A type of track. Something that transports the mob up, down, in a loop, etc.
 */
class track_type : public mob_type {
public:

    unsigned char riders;
    unsigned char riding_pose;
    float ride_speed;
    bool cancellable_with_whistle;
    
    track_type();
    void load_properties(data_node* file);
    void load_resources(data_node* file);
    anim_conversion_vector get_anim_conversions();
};

#endif //ifndef TRACK_TYPE_INCLUDED
