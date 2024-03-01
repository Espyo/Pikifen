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

#include "../libs/data_file.h"
#include "mob_type.h"


//Flags for what sorts of mob can ride on a track.
enum TRACK_RIDERS {

    //Pikmin.
    TRACK_RIDER_PIKMIN = 1,
    
    //Leaders.
    TRACK_RIDER_LEADERS = 2,

};


//Poses that a mob riding on a track can take.
enum TRACK_RIDING_POSES {

    //Stopped.
    TRACK_RIDING_POSE_STOPPED,
    
    //Climbing.
    TRACK_RIDING_POSE_CLIMBING,
    
    //Sliding.
    TRACK_RIDING_POSE_SLIDING,

};


//Track object animations.
enum TRACK_ANIMATIONS {

    //Idling.
    TRACK_ANIM_IDLING,

};


//Track object states.
enum TRACK_STATES {
    
    //Idling.
    TRACK_STATE_IDLING,
    
    //Total amount of track object states.
    N_TRACK_STATES,
    
};


/**
 * @brief A type of track. Something that transports the mob up,
 * down, in a loop, etc.
 */
class track_type : public mob_type {

public:
    
    //--- Members ---

    //Flags representing possible riders.
    unsigned char riders;

    //Pose that riders must take.
    TRACK_RIDING_POSES riding_pose;

    //How quickly riders ride the track, in ratio per second.
    float ride_speed;
    
    //Can the ride be cancelled if the rider is whistled?
    bool cancellable_with_whistle;
    

    //--- Function declarations ---
    
    track_type();
    void load_properties(data_node* file) override;
    void load_resources(data_node* file) override;
    anim_conversion_vector get_anim_conversions() const override;
    
};


#endif //ifndef TRACK_TYPE_INCLUDED
