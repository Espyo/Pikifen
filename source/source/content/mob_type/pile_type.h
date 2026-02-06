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

    //--- Public members ---
    
    //Contents of the pile.
    ResourceType* contents = nullptr;
    
    //How often the pile recharges its contents, if it at all does.
    float rechargeInterval = 0.0f;
    
    //When recharging its contents, it adds these many to the pile.
    int rechargeAmount = 0;
    
    //Maximum amount of contents it can hold.
    size_t maxAmount = 1;
    
    //How much health must it lose before it drops a resource.
    float healthPerResource = 1.0f;
    
    //If true, it can drop multiple resources at once if the health checks out.
    bool canDropMultiple = false;
    
    //Should it show the amount above it?
    bool showAmount = true;
    
    //Should the mob be hidden when it is empty?
    bool hideWhenEmpty = true;
    
    //Auto-radius-shrinking's radius when there's only 1 resource. 0 = off.
    float autoShrinkSmallestRadius = 0.0f;
    
    //Should the mob be deleted when it is no longer needed?
    bool deleteWhenFinished = true;
    
    
    //--- Public function declarations ---
    
    PileType();
    void loadCatProperties(DataNode* file) override;
    AnimConversionVector getAnimConversions() const override;
    
};
