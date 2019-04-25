/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
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
 * pos:    Hitbox's coordinates, from the center of the mob.
 * z:      Z coordinate of the bottom point of the hitbox.
 * height: The hitbox's total height.
 *   0 means it spans indefinitely across the Z axis.
 * radius: Hitbox radius.
 */
hitbox::hitbox(
    const string &bpn, size_t bpi, body_part* bpp, const point &pos,
    const float z, const float height, const float radius
) :
    body_part_name(bpn),
    body_part_index(bpi),
    body_part_ptr(bpp),
    pos(pos),
    z(z),
    height(height),
    radius(radius),
    type(HITBOX_TYPE_NORMAL),
    value(1),
    knockback_outward(true),
    knockback_angle(0),
    knockback(1),
    wither_chance(0),
    can_pikmin_latch(false) {
    
}


/* ----------------------------------------------------------------------------
 * Returns the coordinates of the hitbox given the mob's location and angle.
 */
point hitbox::get_cur_pos(
    const point &mob_pos, const float &mob_angle_cos, const float &mob_angle_sin
) {
    return
        point(
            mob_pos.x + (pos.x * mob_angle_cos - pos.y * mob_angle_sin),
            mob_pos.y + (pos.x * mob_angle_sin + pos.y * mob_angle_cos)
        );
}
