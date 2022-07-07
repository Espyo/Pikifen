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
    //Contents of the pile.
    resource_type* contents;
    //How often the pile recharges its contents, if it at all does.
    float recharge_interval;
    //When recharging its contents, it adds these many to the pile.
    int recharge_amount;
    //Maximum amount of contents it can hold.
    size_t max_amount;
    //How much health must it lose before it drops a resource.
    float health_per_resource;
    //If true, it can drop multiple resources at once if the health checks out.
    bool can_drop_multiple;
    //Should it show the amount above it?
    bool show_amount;
    //Should the mob be hidden when it is empty?
    bool hide_when_empty;
    //Should the mob be deleted when it is no longer needed?
    bool delete_when_finished;
    
    pile_type();
    void load_properties(data_node* file) override;
    anim_conversion_vector get_anim_conversions() const override;
};


#endif //ifndef PILE_TYPE_INCLUDED
