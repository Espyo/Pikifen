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

#include "../drawing.h"
#include "../functions.h"
#include "../game.h"
#include "../utils/string_utils.h"
#include "ship.h"


/**
 * @brief Constructs a new treasure object.
 *
 * @param pos Starting coordinates.
 * @param type Treasure type this mob belongs to.
 * @param angle Starting angle.
 */
treasure::treasure(const point &pos, treasure_type* type, float angle) :
    mob(pos, type, angle),
    tre_type(type) {
    
    become_carriable(CARRY_DESTINATION_SHIP);
    
    set_animation(
        MOB_TYPE::ANIM_IDLING, START_ANIM_OPTION_RANDOM_TIME_ON_SPAWN, true
    );
    
    particle pa(
        PARTICLE_TYPE_BITMAP, pos, z + get_drawing_height() + 1.0f,
        28.0f, 1.0f, PARTICLE_PRIORITY_LOW
    );
    pa.bitmap = game.sys_assets.bmp_sparkle;
    pa.speed.y = -30.0f;
    particle_generator pg(0.4f, pa);
    pg.id = MOB_PARTICLE_GENERATOR_ID_SCRIPT;
    pg.follow_mob = this;
    pg.follow_angle = &this->angle;
    pg.follow_z_offset = z + get_drawing_height() + 1.0f;
    pg.duration_deviation = 0.1f;
    pg.interval_deviation = 0.05f;
    pg.pos_deviation = point(radius * 0.75f, radius * 0.75f);
    pg.size_deviation = 4.0f;
    particle_generators.push_back(pg);
    
}
