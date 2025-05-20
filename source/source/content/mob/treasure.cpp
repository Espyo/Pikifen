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
Treasure::Treasure(const Point &pos, TreasureType* type, float angle) :
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
