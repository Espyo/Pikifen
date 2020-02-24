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


/* ----------------------------------------------------------------------------
 * An Onion is where Pikmin are stored.
 */
class onion : public mob {
public:
    //What type of Onion it is.
    onion_type* oni_type;
    
    //Is this Onion currently activated?
    bool activated;
    //How many Pikmin are inside, per maturity.
    size_t pikmin_inside[N_MATURITIES];
    //How many Pikmin are queued up to be spat.
    //TODO this needs to be a proper queue (Master Onion).
    size_t spew_queue;
    //Time left until it starts spewing queued seeds.
    timer full_spew_timer;
    //Time left until it spews the next seed in the queue.
    timer next_spew_timer;
    //Angle at which the next seed will be spit.
    float next_spew_angle;
    //The Onion's alpha.
    unsigned char seethrough;
    
    //Call a Pikmin out.
    void call_pikmin();
    //Spit a new seed.
    void spew();
    //Store a Pikmin inside.
    void stow_pikmin();
    
    //Constructor.
    onion(const point &pos, onion_type* type, const float angle);
    //Mob drawing routine.
    virtual void draw_mob(bitmap_effect_manager* effect_manager = NULL);
    //Read script variables from the area data.
    virtual void read_script_vars(const string &vars);
    
protected:
    //Tick class-specific logic.
    virtual void tick_class_specifics(const float delta_t);
};

#endif //ifndef ONION_INCLUDED
