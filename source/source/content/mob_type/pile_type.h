/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the pile type class and pile type-related functions.
 */

#pragma once

#include <allegro5/allegro.h>

#include "../../core/misc_structs.h"
#include "../../lib/data_file/data_file.h"
#include "mob_type.h"
#include "resource_type.h"


//Pile object animations.
enum PILE_ANIM {

    //Idling.
    PILE_ANIM_IDLING,
    
    //Total amount of pile object animations.
    N_PILE_ANIMS,
    
};


//Pile object states.
enum PILE_STATE {

    //Idling.
    PILE_STATE_IDLING,
    
    //Total amount of pile object states.
    N_PILE_STATES,
    
};


/**
 * @brief A type of resource pile (gold nugget pile,
 * Burgeoning Spiderwort, etc.).
 */
class PileType : public MobType, public MobTypeWithAnimGroups {

public:

    //--- Members ---
    
    //Contents of the pile.
    ResourceType* contents = nullptr;
    
    //How often the pile recharges its contents, if it at all does.
    float recharge_interval = 0.0f;
    
    //When recharging its contents, it adds these many to the pile.
    int recharge_amount = 0;
    
    //Maximum amount of contents it can hold.
    size_t max_amount = 1;
    
    //How much health must it lose before it drops a resource.
    float health_per_resource = 1.0f;
    
    //If true, it can drop multiple resources at once if the health checks out.
    bool can_drop_multiple = false;
    
    //Should it show the amount above it?
    bool show_amount = true;
    
    //Should the mob be hidden when it is empty?
    bool hide_when_empty = true;
    
    //Auto-radius-shrinking's radius when there's only 1 resource. 0 = off.
    float auto_shrink_smallest_radius = 0.0f;
    
    //Should the mob be deleted when it is no longer needed?
    bool delete_when_finished = true;
    
    
    //--- Function declarations ---
    
    PileType();
    void loadCatProperties(DataNode* file) override;
    anim_conversion_vector getAnimConversions() const override;
    
};
