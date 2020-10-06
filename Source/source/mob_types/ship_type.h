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
#include "../utils/data_file.h"
#include "mob_type.h"


enum SHIP_ANIMATIONS {
    SHIP_ANIM_IDLING,
};


enum SHIP_STATES {
    SHIP_STATE_IDLING,
    
    N_SHIP_STATES,
};


/* ----------------------------------------------------------------------------
 * A type of ship (Hocotate ship, research pod, golden HS, S.S. Drake, etc.).
 */
class ship_type : public mob_type {
public:
    //Nest data.
    pikmin_nest_type_struct* nest;
    
    //Can a leader heal at this ship?
    bool can_heal;
    //The ship's beam is offset this much from the ship object's center.
    point beam_offset;
    //Ship beam radius.
    float beam_radius;
    
    ship_type();
    ~ship_type();
    void load_properties(data_node* file);
    void load_resources(data_node* file);
    anim_conversion_vector get_anim_conversions() const;
};


#endif //ifndef SHIP_TYPE_INCLUDED
