/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the interactable type class and
 * interactable type-related functions.
 */

#pragma once

#include "mob_type.h"


/**
 * @brief A type of "interactable" mob. This can be a readable sign,
 * a switch, etc.
 */
class InteractableType : public MobType {

public:

    //--- Public members ---
    
    //Text to display above the mob, prompting the player on what to do.
    string promptText;
    
    //How close the leader must be before the player can interact with it.
    float triggerRange = 64.0f;
    
    
    //--- Public function declarations ---
    
    InteractableType();
    void loadCatProperties(DataNode* file) override;
    
};
