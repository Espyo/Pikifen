/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the ship type class and ship type-related functions.
 */

#pragma once

#include <allegro5/allegro.h>

#include "../../core/misc_structs.h"
#include "../../lib/data_file/data_file.h"
#include "../mob/mob_utils.h"
#include "mob_type.h"


//Ship object animations.
enum SHIP_ANIM {

    //Idling.
    SHIP_ANIM_IDLING,
    
};


//Ship object states.
enum SHIP_STATE {

    //Idling.
    SHIP_STATE_IDLING,
    
    //Total amount of ship object states.
    N_SHIP_STATES,
    
};


/**
 * @brief A type of ship (Hocotate ship, research pod, golden HS,
 * S.S. Drake, etc.).
 */
class ShipType : public MobType {

public:

    //--- Public members ---
    
    //Nest data.
    PikminNestType* nest = nullptr;
    
    //Can a leader heal at this ship?
    bool canHeal = false;
    
    //The ship's control point is offset this much from the mob's center.
    Point controlPointOffset;
    
    //The ship's receptacle is offset this much from the mob's center.
    Point receptacleOffset;
    
    //Ship control point radius.
    float controlPointRadius = 45.0f;
    
    //Sound data index for the beam sound. Cache for performance.
    size_t soundBeamIdx = INVALID;
    
    //Sound data index for the object reception sound. Cache for performance.
    size_t soundReceptionIdx = INVALID;
    
    
    //--- Public function declarations ---
    
    ShipType();
    ~ShipType();
    void loadCatProperties(DataNode* file) override;
    void loadCatResources(DataNode* file) override;
    AnimConversionVector getAnimConversions() const override;
    
};
