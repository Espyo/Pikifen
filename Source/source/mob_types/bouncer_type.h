/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the bouncer type class and bouncer type-related functions.
 */

#ifndef BOUNCER_TYPE_INCLUDED
#define BOUNCER_TYPE_INCLUDED

#include "../libs/data_file.h"
#include "mob_type.h"


//Flags for what sorts of mobs can ride on a bouncer.
enum BOUNCER_RIDER_FLAG {

    //Pikmin.
    BOUNCER_RIDER_FLAG_PIKMIN = 1 << 0,
    
    //Leaders.
    BOUNCER_RIDER_FLAG_LEADERS = 1 << 1,
    
};


//Poses for riders to take.
enum BOUNCER_RIDING_POSE {

    //Stopped. Basically the idling pose.
    BOUNCER_RIDING_POSE_STOPPED,
    
    //Somersaulting.
    BOUNCER_RIDING_POSE_SOMERSAULT,

};


//Bouncer object animations.
enum BOUNCER_ANIM {

    //Idling.
    BOUNCER_ANIM_IDLING,
    
    //Bouncing something.
    BOUNCER_ANIM_BOUNCING,

};


//Bouncer object states.
enum BOUNCER_STATE {
    
    //Idling.
    BOUNCER_STATE_IDLING,
    
    //Bouncing something.
    BOUNCER_STATE_BOUNCING,
    
    //Total amount of bouncer object states.
    N_BOUNCER_STATES,
    
};


/**
 * @brief A type of bouncer. Something that grabs another mob and bounces it
 * away to a specific location, making that mob do a specific animation.
 */
class bouncer_type : public mob_type {

public:
    
    //--- Members ---

    //Flags representing which mobs can ride on it.
    unsigned char riders = BOUNCER_RIDER_FLAG_PIKMIN;
    
    //Pose that riders should take.
    BOUNCER_RIDING_POSE riding_pose = BOUNCER_RIDING_POSE_STOPPED;
    

    //--- Function declarations ---
    
    bouncer_type();
    void load_properties(data_node* file) override;
    anim_conversion_vector get_anim_conversions() const override;
    
};


#endif //ifndef BOUNCER_TYPE_INCLUDED
