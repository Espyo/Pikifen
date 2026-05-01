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
 * @param center Starting center coordinates.
 * @param type Resource type this mob belongs to.
 * @param angle Starting angle.
 */
Resource::Resource(const Point& center, ResourceType* type, float angle) :
    Mob(center, type, angle),
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
    if(parent) return parent->m->getMissionPoints(applicableInThisMission);
    
    if(applicableInThisMission) {
        const auto checkMetric = [] (MISSION_METRIC metric) {
            return
                metric == MISSION_METRIC_OBJECT_COLLECTION_PTS ||
                metric == MISSION_METRIC_TREASURE_COLLECTION_PTS;
        };
        
        *applicableInThisMission = false;
        
        forIdx(c, game.curArea->mission.endConds) {
            MissionEndCond* cPtr =
                &game.curArea->mission.endConds[c];
            if(!cPtr->usesMetric()) continue;
            if(!checkMetric(cPtr->metricType)) continue;
            *applicableInThisMission = true;
        }
        forIdx(c, game.curArea->mission.scoreCriteria) {
            MissionScoreCriterion* cPtr =
                &game.curArea->mission.scoreCriteria[c];
            if(!checkMetric(cPtr->metricType)) continue;
            *applicableInThisMission = true;
        }
        forIdx(i, game.curArea->mission.hudItems) {
            MissionHudItem* iPtr =
                &game.curArea->mission.hudItems[i];
            if(!iPtr->usesMetric()) continue;
            if(!checkMetric(iPtr->metricType)) continue;
            *applicableInThisMission = true;
        }
    }
    
    if(
        resType->deliveryResult ==
        RESOURCE_DELIVERY_RESULT_ADD_TREASURE_POINTS
    ) {
        return resType->pointAmount;
    }
    return 0;
}
