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

#include "../utils/data_file.h"
#include "mob_type.h"


//Flags for what sorts of mobs can ride on a bouncer.
enum BOUNCER_RIDERS {
    //Pikmin.
    BOUNCER_RIDER_PIKMIN = 1,
    //Leaders.
    BOUNCER_RIDER_LEADERS = 2,
};


//Poses for riders to take.
enum BOUNCER_RIDING_POSES {
    //Stopped. Basically the idling pose.
    BOUNCER_RIDING_POSE_STOPPED,
    //Somersaulting.
    BOUNCER_RIDING_POSE_SOMERSAULT,
};


//Bouncer object animations.
enum BOUNCER_ANIMATIONS {
    //Idling.
    BOUNCER_ANIM_IDLING,
    //Bouncing something.
    BOUNCER_ANIM_BOUNCING,
};


//Bouncer object states.
enum BOUNCER_STATES {
    //Idling.
    BOUNCER_STATE_IDLING,
    //Bouncing something.
    BOUNCER_STATE_BOUNCING,
    
    //Total amount of bouncer object states.
    N_BOUNCER_STATES,
};


/* ----------------------------------------------------------------------------
 * A type of bouncer. Something that grabs another mob and bounces it away
 * to a specific location, making that mob do a specific animation.
 */
class bouncer_type : public mob_type {
public:
    //Flags representing which mobs can ride on it.
    unsigned char riders;
    //Pose that riders should take.
    BOUNCER_RIDING_POSES riding_pose;
    
    bouncer_type();
    void load_properties(data_node* file);
    anim_conversion_vector get_anim_conversions() const;
};


#endif //ifndef BOUNCER_TYPE_INCLUDED
