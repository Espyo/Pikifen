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
#include "../../core/misc_functions.h"
#include "../../core/game.h"
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
    tre_type(type) {
    
    becomeCarriable(CARRY_DESTINATION_SHIP);
    
    setAnimation(
        MOB_TYPE::ANIM_IDLING, START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
    
    ParticleGenerator pg =
        standardParticleGenSetup(
            game.sys_content_names.part_treasure, this
        );
    pg.emission.circle_outer_dist *= radius;
    pg.base_particle.priority = PARTICLE_PRIORITY_LOW;
    particle_generators.push_back(pg);
    
}
