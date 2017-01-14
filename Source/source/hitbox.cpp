/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2017.
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
 * Creates a body part.
 */
body_part::body_part(const string &name) :
    name(name) {
    
}


/* ----------------------------------------------------------------------------
 * Creates a hitbox.
 * bpn:    Name of the body part.
 * bpi:    Index of the body part in the animation database.
 * bpp:    Pointer to the body part.
 * x, y:   Hitbox's coordinates, from the center of the mob.
 * z:      Z coordinate of the bottom point of the hitbox.
 * height: The hitbox's total height.
   * 0 means it spans indefinitely across the Z axis.
 * radius: Hitbox radius.
 */
hitbox::hitbox(
    const string &bpn, size_t bpi, body_part* bpp, const float x, const float y,
    const float z, const float height, const float radius
) :
    body_part_name(bpn),
    body_part_index(bpi),
    body_part_ptr(bpp),
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
