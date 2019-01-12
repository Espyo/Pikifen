/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Onion class and Onion-related functions.
 */

#ifndef ONION_INCLUDED
#define ONION_INCLUDED

#define _USE_MATH_DEFINES
#include <math.h>

#include "../misc_structs.h"
#include "../mob_types/onion_type.h"
#include "../mob_types/pikmin_type.h"
#include "mob.h"

enum ONION_STATES {
    ONION_STATE_IDLING,
    
    N_ONION_STATES,
};

const float ONION_FULL_SPEW_DELAY          = 2.5f;
const float ONION_NEXT_SPEW_DELAY          = 0.10f;
const unsigned char ONION_SEETHROUGH_ALPHA = 64;
const float ONION_FADE_SPEED               = 255; //Values per second.


/* ----------------------------------------------------------------------------
 * An Onion is where Pikmin are stored.
 */
class onion : public mob {
protected:
    virtual void tick_class_specifics();
    
public:
    onion_type* oni_type;
    bool activated;
    //How many Pikmin are inside, per maturity.
    size_t pikmin_inside[N_MATURITIES];
    size_t spew_queue; //TODO this needs to be a proper queue (Master Onion).
    //Time left until it starts spewing queued seeds.
    timer full_spew_timer;
    //Time left until it spews the next seed in the queue.
    timer next_spew_timer;
    //Angle at which the next seed will be spit.
    float next_spew_angle;
    //The Onion's alpha.
    unsigned char seethrough;
    
    onion(const point &pos, onion_type* type, const float angle);
    virtual void draw_mob(bitmap_effect_manager* effect_manager = NULL);
    virtual void read_script_vars(const string &vars);
    
    void call_pikmin();
    void spew();
    void stow_pikmin();
};

#endif //ifndef ONION_INCLUDED
