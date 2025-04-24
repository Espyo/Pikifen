/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Interactable type class and interactable type-related functions.
 */

#include "interactable_type.h"

#include "../../util/string_utils.h"
#include "../mob/mob.h"


/**
 * @brief Constructs a new interactable type object.
 */
InteractableType::InteractableType() :
    MobType(MOB_CATEGORY_INTERACTABLES) {
    
    targetType = MOB_TARGET_FLAG_NONE;
    
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void InteractableType::loadCatProperties(DataNode* file) {
    ReaderSetter iRS(file);
    
    iRS.set("prompt_text", promptText);
    iRS.set("trigger_range", triggerRange);
}
