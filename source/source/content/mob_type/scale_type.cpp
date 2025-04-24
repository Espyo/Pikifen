/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Scale type class and scale type-related functions.
 */

#include "scale_type.h"

#include "../../util/string_utils.h"
#include "../mob/scale.h"


/**
 * @brief Constructs a new scale type object.
 */
ScaleType::ScaleType() :
    MobType(MOB_CATEGORY_SCALES) {
    
    targetType = MOB_TARGET_FLAG_NONE;
    walkable = true;
    
    AreaEditorProp aepGoal;
    aepGoal.name = "Goal weight";
    aepGoal.var = "goal_number";
    aepGoal.type = AEMP_TYPE_INT;
    aepGoal.defValue = i2s(goalNumber);
    aepGoal.tooltip = "Pikmin weight required for the goal, if any.";
    areaEditorProps.push_back(aepGoal);
}


/**
 * @brief Loads properties from a data file.
 *
 * @param file File to read from.
 */
void ScaleType::loadCatProperties(DataNode* file) {
    ReaderSetter sRS(file);
    
    sRS.set("goal_number", goalNumber);
    
    areaEditorProps.back().defValue = i2s(goalNumber);
}
