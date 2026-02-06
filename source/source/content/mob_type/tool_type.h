/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the tool type class and tool type-related functions.
 */

#pragma once

#include "../../lib/data_file/data_file.h"
#include "mob_type.h"


/**
 * @brief A type of tool. A type of hand-held explosive, for instance.
 */
class ToolType : public MobType {

public:

    //--- Public members ---
    
    //Icon used to represent this tool in the HUD group info.
    ALLEGRO_BITMAP* bmpIcon = nullptr;
    
    //If true, the Pikmin holding it can be swapped for the tool, when chomped.
    bool canBeHotswapped = true;
    
    //Should it be dropped if the Pikmin carrying it is whistled?
    bool droppedWhenPikminIsWhistled = false;
    
    //Should it be dropped if the Pikmin carrying it lands from a throw?
    bool droppedWhenPikminLands = true;
    
    //Should it be dropped if the Pikmin carrying it lands on an opponent?
    bool droppedWhenPikminLandsOnOpponent = false;
    
    //Should it stay stuck to the opponent if the thrown Pikmin drops it there?
    bool stuckWhenPikminLandsOnOpponent = false;
    
    //Should the Pikmin return to the group after using this tool?
    bool pikminReturnsAfterUsing = true;
    
    
    //--- Public function declarations ---
    
    ToolType();
    void loadCatProperties(DataNode* file) override;
    void loadCatResources(DataNode* file) override;
    
};
