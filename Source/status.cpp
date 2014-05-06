/*
 * Copyright (c) André 'Espyo' Silva 2014.
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
status::status(const float speed_multiplier, const float attack_multiplier, const float defense_multiplier, const bool freezes_everything, const ALLEGRO_COLOR color, const unsigned char affects) {
    this->speed_multiplier = speed_multiplier;
    this->attack_multiplier = attack_multiplier;
    this->defense_multiplier = defense_multiplier;
    this->freezes_everything = freezes_everything;
    this->color = color;
    this->affects = affects;
}