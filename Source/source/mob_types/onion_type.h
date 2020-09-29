/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Onion type class and Onion type-related functions.
 */

#ifndef ONION_TYPE_INCLUDED
#define ONION_TYPE_INCLUDED

#include "../utils/data_file.h"
#include "mob_type.h"
#include "pikmin_type.h"


enum ONION_STATES {
    ONION_STATE_IDLING,
    
    N_ONION_STATES,
};


const float ONION_FULL_SPEW_DELAY          = 2.5f;
const float ONION_NEXT_SPEW_DELAY          = 0.10f;
const unsigned char ONION_SEETHROUGH_ALPHA = 64;
const float ONION_FADE_SPEED               = 255; //Values per second.


/* ----------------------------------------------------------------------------
 * An Onion type.
 * It's basically associated with one or more Pikmin types.
 */
class onion_type : public mob_type {
public:
    vector<pikmin_type*> pik_types;
    vector<string> leg_body_parts;
    float pikmin_enter_speed;
    float pikmin_exit_speed;
    
    onion_type();
    
    void load_properties(data_node* file);
    void load_resources(data_node* file);
    anim_conversion_vector get_anim_conversions() const;
};


#endif //ifndef ONION_TYPE_INCLUDED
