/*
 * Copyright (c) André 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Onion class and Onion-related functions.
 */

#ifndef ONION_INCLUDED
#define ONION_INCLUDED

#define _USE_MATH_DEFINES
#include <math.h>

#include "misc_structs.h"
#include "mob.h"
#include "pikmin_type.h"
#include "onion_type.h"

/* ----------------------------------------------------------------------------
 * An Onion is where Pikmin are stored.
 */
class onion : public mob {
public:
    onion_type* oni_type;
    size_t spew_queue;
    timer full_spew_timer; //Time left until it starts spewing queued seeds.
    timer next_spew_timer; //Time left until it spews the next seed in the queue.
    float next_spew_angle; //Angle at which the next seed will be spit.
    
    onion(float x, float y, onion_type* type, const float angle, const string &vars);
    virtual void draw();
    
    void receive_mob(size_t seeds);
    void spew();
};

const float ONION_FULL_SPEW_DELAY  = 1.5f;
const float ONION_NEXT_SPEW_DELAY  = 0.15f;
const float ONION_SPEW_ANGLE_SHIFT = M_PI_4 - (M_PI_4 * 0.1);

#endif //ifndef ONION_INCLUDED
