/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Treasure class and treasure-related functions.
 */

#include "treasure.h"

#include "../../core/drawing.h"
#include "../../core/game.h"
#include "../../core/misc_functions.h"
#include "../../util/string_utils.h"
#include "ship.h"


/**
 * @brief Constructs a new treasure object.
 *
 * @param pos Starting coordinates.
 * @param type Treasure type this mob belongs to.
 * @param angle Starting angle.
 */
Treasure::Treasure(const Point& pos, TreasureType* type, float angle) :
    Mob(pos, type, angle),
    treType(type) {
    
    becomeCarriable(CARRY_DESTINATION_SHIP);
    
    setAnimation(
        MOB_TYPE::ANIM_IDLING, START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
    
    ParticleGenerator pg =
        standardParticleGenSetup(
            game.sysContentNames.parTreasure, this
        );
    pg.emission.circleOuterDist *= radius;
    pg.baseParticle.priority = PARTICLE_PRIORITY_LOW;
    particleGenerators.push_back(pg);
    
}


/**
 * @brief Returns how many mission points this mob is currently worth, or
 * 0 if not applicable.
 *
 * @param applicableInThisMission If not nullptr, whether the points are
 * applicable in this mission or not is returned here.
 * @return The point amount.
 */
int Treasure::getMissionPoints(bool* applicableInThisMission) const {
    if(parent) return parent->m->getMissionPoints(applicableInThisMission);
    
    if(applicableInThisMission) {
        const auto checkMetric = [] (MISSION_METRIC metric) {
            return
                metric == MISSION_METRIC_OBJECT_COLLECTION_PTS ||
                metric == MISSION_METRIC_TREASURE_COLLECTION_PTS;
        };
        
        *applicableInThisMission = false;
        
        for(size_t c = 0; c < game.curArea->mission.endConds.size(); c++) {
            MissionEndCond* cPtr =
                &game.curArea->mission.endConds[c];
            if(!cPtr->usesMetric()) continue;
            if(!checkMetric(cPtr->metricType)) continue;
            *applicableInThisMission = true;
        }
        for(size_t c = 0; c < game.curArea->mission.scoreCriteria.size(); c++) {
            MissionScoreCriterion* cPtr =
                &game.curArea->mission.scoreCriteria[c];
            if(!checkMetric(cPtr->metricType)) continue;
            *applicableInThisMission = true;
        }
        for(size_t i = 0; i < game.curArea->mission.hudItems.size(); i++) {
            MissionHudItem* iPtr =
                &game.curArea->mission.hudItems[i];
            if(!iPtr->usesMetric()) continue;
            if(!checkMetric(iPtr->metricType)) continue;
            *applicableInThisMission = true;
        }
    }

    return (int) treType->points;
}
