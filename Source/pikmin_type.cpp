/*
 * Copyright (c) André 'Espyo' Silva 2014.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included README file
 * for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pikmin type class and Pikmin type-related functions.
 */

#include "pikmin_type.h"
#include "const.h"

/* ----------------------------------------------------------------------------
 * Creates a Pikmin type.
 */
pikmin_type::pikmin_type() {
    //ToDo
    attack_attribute = 0;
    carry_strength = 1;
    attack_power = 1;
    weight = 1;
    carry_speed = 1;
    attack_interval = 0.8;
    size = DEF_PIKMIN_SIZE;
    has_onion = true;
    can_dig = false;
    can_fly = false;
    can_swim = false;
    can_latch = true;
    can_carry_bomb_rocks = false;
    bmp_top[0] = NULL;
    bmp_top[1] = NULL;
    bmp_top[2] = NULL;
    bmp_icon[0] = NULL;
    bmp_icon[1] = NULL;
    bmp_icon[2] = NULL;
}