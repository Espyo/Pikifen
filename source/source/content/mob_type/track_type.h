/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the track type class and track type-related functions.
 */

#pragma once

#include "../../lib/data_file/data_file.h"
#include "mob_type.h"


//Flags for what sorts of mob can ride on a track.
enum TRACK_RIDER_FLAG {

    //Pikmin.
    TRACK_RIDER_FLAG_PIKMIN = 1 << 0,
    
    //Leaders.
    TRACK_RIDER_FLAG_LEADERS = 1 << 1,
    
};


//Poses that a mob riding on a track can take.
enum TRACK_RIDING_POSE {

    //Stopped.
    TRACK_RIDING_POSE_STOPPED,
    
    //Climbing.
    TRACK_RIDING_POSE_CLIMBING,
    
    //Sliding.
    TRACK_RIDING_POSE_SLIDING,
    
};


//Track object animations.
enum TRACK_ANIM {

    //Idling.
    TRACK_ANIM_IDLING,
    
};


//Track object states.
enum TRACK_STATE {

    //Idling.
    TRACK_STATE_IDLING,
    
    //Total amount of track object states.
    N_TRACK_STATES,
    
};


/**
 * @brief A type of track. Something that transports the mob up,
 * down, in a loop, etc.
 */
class TrackType : public MobType {

public:

    //--- Members ---
    
    //Flags representing possible riders.
    unsigned char riders = TRACK_RIDER_FLAG_PIKMIN;
    
    //Pose that riders must take.
    TRACK_RIDING_POSE riding_pose = TRACK_RIDING_POSE_STOPPED;
    
    //How quickly riders ride the track, in ratio per second.
    float ride_speed = 0.5f;
    
    //Can the ride be cancelled if the rider is whistled?
    bool cancellable_with_whistle = false;
    
    
    //--- Function declarations ---
    
    TrackType();
    void loadCatProperties(DataNode* file) override;
    void loadCatResources(DataNode* file) override;
    anim_conversion_vector getAnimConversions() const override;
    
};
