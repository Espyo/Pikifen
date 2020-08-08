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


enum BOUNCER_RIDERS {
    BOUNCER_RIDER_PIKMIN = 1,
    BOUNCER_RIDER_LEADERS = 2,
};


enum BOUNCER_RIDING_POSES {
    BOUNCER_RIDING_POSE_STOPPED,
    BOUNCER_RIDING_POSE_SOMERSAULT,
};


enum BOUNCER_ANIMATIONS {
    BOUNCER_ANIM_IDLING,
    BOUNCER_ANIM_BOUNCING,
};


enum BOUNCER_STATES {
    BOUNCER_STATE_IDLING,
    BOUNCER_STATE_BOUNCING,
    N_BOUNCER_STATES,
};


/* ----------------------------------------------------------------------------
 * A type of bouncer. Something that grabs another mob and bounces it away
 * to a specific location, making that mob do a specific animation.
 */
class bouncer_type : public mob_type {
public:

    unsigned char riders;
    unsigned char riding_pose;
    
    bouncer_type();
    void load_properties(data_node* file);
    anim_conversion_vector get_anim_conversions() const;
};


#endif //ifndef BOUNCER_TYPE_INCLUDED
