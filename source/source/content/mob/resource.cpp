/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Resource class and resource-related functions.
 */

#include "resource.h"

#include "../../core/drawing.h"
#include "../../core/game.h"
#include "../../core/misc_functions.h"


/**
 * @brief Constructs a new resource object.
 *
 * @param pos Starting coordinates.
 * @param type Resource type this mob belongs to.
 * @param angle Starting angle.
 */
Resource::Resource(const Point& pos, ResourceType* type, float angle) :
    Mob(pos, type, angle),
    resType(type),
    originPile(nullptr) {
    
    becomeCarriable(resType->carryingDestination);
}


/**
 * @brief Returns how many mission points this mob is currently worth, or
 * 0 if not applicable.
 *
 * @param applicableInThisMission If not nullptr, whether the points are
 * applicable in this mission or not is returned here.
 * @return The point amount.
 */
int Resource::getMissionPoints(bool* applicableInThisMission) const {
    if(applicableInThisMission) {
        *applicableInThisMission =
            game.curAreaData->missionOld.pointsPerTreasurePoint != 0;
    }
    if(parent) return parent->m->getMissionPoints(applicableInThisMission);
    if(
        resType->deliveryResult ==
        RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS
    ) {
        return resType->pointAmount;
    }
    return 0;
}
