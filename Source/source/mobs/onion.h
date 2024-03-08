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


namespace ONION {
extern const float FADE_SPEED;
extern const float FULL_SPEW_DELAY;
extern const float NEXT_SPEW_DELAY;
extern const unsigned char SEETHROUGH_ALPHA;
}


/**
 * @brief An Onion is where Pikmin are stored.
 */
class onion : public mob {

public:

    //--- Misc. declarations ---

    static const float ONION_NEW_SEED_Z_OFFSET;
    static const float ONION_SPEW_ANGLE_SHIFT;
    static const float ONION_SPEW_H_SPEED;
    static const float ONION_SPEW_H_SPEED_DEVIATION;
    static const float ONION_SPEW_V_SPEED;

    
    //--- Members ---

    //What type of Onion it is.
    onion_type* oni_type = nullptr;
    
    //Nest data.
    pikmin_nest_struct* nest = nullptr;
    
    //Is this Onion currently activated?
    bool activated = true;

    //How many seeds are queued up to be spat, of each type.
    vector<size_t> spew_queue;

    //Time left until it starts spewing queued seeds.
    timer full_spew_timer = timer(ONION::FULL_SPEW_DELAY);

    //Time left until it spews the next seed in the queue.
    timer next_spew_timer = timer(ONION::NEXT_SPEW_DELAY);

    //Angle at which the next seed will be spit.
    float next_spew_angle = 0.0f;
    
    //The Onion's alpha.
    unsigned char seethrough = 255;
    
    //Spit a new seed.
    void spew();
    

    //--- Function declarations ---

    onion(const point &pos, onion_type* type, const float angle);
    ~onion();
    void draw_mob() override;
    void read_script_vars(const script_var_reader &svr) override;
    
    
protected:
    
    //--- Function declarations ---

    void tick_class_specifics(const float delta_t) override;
    
};


#endif //ifndef ONION_INCLUDED
