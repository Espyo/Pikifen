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
    if(applicableInThisMission) {
        *applicableInThisMission =
            game.curAreaData->mission.pointsPerTreasurePoint != 0;
    }
    if(parent) return parent->m->getMissionPoints(applicableInThisMission);
    return treType->points;
}
