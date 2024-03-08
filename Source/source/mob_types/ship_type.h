/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the ship type class and ship type-related functions.
 */

#ifndef SHIP_TYPE_INCLUDED
#define SHIP_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "../misc_structs.h"
#include "../mobs/mob_utils.h"
#include "../libs/data_file.h"
#include "mob_type.h"


//Ship object animations.
enum SHIP_ANIMATIONS {

    //Idling.
    SHIP_ANIM_IDLING,
    
};


//Ship object states.
enum SHIP_STATES {
    
    //Idling.
    SHIP_STATE_IDLING,
    
    //Total amount of ship object states.
    N_SHIP_STATES,

};


/**
 * @brief A type of ship (Hocotate ship, research pod, golden HS,
 * S.S. Drake, etc.).
 */
class ship_type : public mob_type {

public:
    
    //--- Members ---

    //Nest data.
    pikmin_nest_type_struct* nest = nullptr;

    //Can a leader heal at this ship?
    bool can_heal = false;
    
    //The ship's control point is offset this much from the mob's center.
    point control_point_offset;

    //The ship's receptacle is offset this much from the mob's center.
    point receptacle_offset;
    
    //Ship control point radius.
    float control_point_radius = 45.0f;
    

    //--- Function declarations ---
    
    ship_type();
    ~ship_type();
    void load_properties(data_node* file) override;
    void load_resources(data_node* file) override;
    anim_conversion_vector get_anim_conversions() const override;
    
};


#endif //ifndef SHIP_TYPE_INCLUDED
