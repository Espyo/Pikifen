/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2015.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Status effect class and status effect-related functions.
 */

#include "status.h"

/* ----------------------------------------------------------------------------
 * Creates a status effect.
 */
status::status(const float speed_multiplier, const float attack_multiplier, const float defense_multiplier, const bool freezes_everything, const ALLEGRO_COLOR &color, const unsigned char affects) :
    speed_multiplier(speed_multiplier),
    attack_multiplier(attack_multiplier),
    defense_multiplier(defense_multiplier),
    freezes_everything(freezes_everything),
    color(color),
    affects(affects) {
    
}
