/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the pile type class and pile type-related functions.
 */

#ifndef PILE_TYPE_INCLUDED
#define PILE_TYPE_INCLUDED

#include <allegro5/allegro.h>

#include "../misc_structs.h"
#include "../utils/data_file.h"
#include "mob_type.h"
#include "resource_type.h"


//Pile object animations.
enum PILE_ANIMATIONS {
    //Idling.
    PILE_ANIM_IDLING,
    
    //Total amount of pile object animations.
    N_PILE_ANIMS,
};


//Pile object states.
enum PILE_STATES {
    //Idling.
    PILE_STATE_IDLING,
    
    //Total amount of pile object states.
    N_PILE_STATES,
};


/* ----------------------------------------------------------------------------
 * A type of resource pile (gold nugget pile, Burgeoning Spiderwort, etc.).
 */
class pile_type : public mob_type, public mob_type_with_anim_groups {
public:
    resource_type* contents;
    float recharge_interval;
    int recharge_amount;
    size_t max_amount;
    float health_per_resource;
    bool can_drop_multiple;
    bool show_amount;
    bool hide_when_empty;
    bool delete_when_finished;
    
    pile_type();
    void load_properties(data_node* file);
    anim_conversion_vector get_anim_conversions() const;
};


#endif //ifndef PILE_TYPE_INCLUDED
