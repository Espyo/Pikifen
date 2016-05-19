/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2016.
 * The following source file belongs to the open-source project
 * Pikmin fangame engine. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Hitbox class and hitbox-related functions.
 */

#include "hitbox.h"

/* ----------------------------------------------------------------------------
 * Creates a hitbox.
 */
hitbox::hitbox(const string &name) :
    name(name) {

}


/* ----------------------------------------------------------------------------
 * Creates an instance of a hitbox.
 * hn:     Name of the hitbox.
 * hnr:    Number of the hitbox.
 * hp:     Pointer to the hitbox.
 * x, y:   Instance's coordinates, from the center of the mob.
 * z:      Z coordinate of the bottom point of the instance.
 * height: The hitbox instance's total height.
   * 0 means it spans indefinitely across the Z axis.
 * radius: Hitbox radius for X and Y.
 */
hitbox_instance::hitbox_instance(
    const string &hn, size_t hnr, hitbox* hp, const float x, const float y,
    const float z, const float height, const float radius
) :
    hitbox_name(hn),
    hitbox_nr(hnr),
    hitbox_ptr(hp),
    x(x),
    y(y),
    z(z),
    height(height),
    radius(radius),
    type(HITBOX_TYPE_NORMAL),
    multiplier(1),
    knockback_outward(true),
    knockback_angle(0),
    knockback(1),
    can_pikmin_latch(false) {

}
