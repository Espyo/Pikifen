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


/**
 * @brief Constructs a new body part object.
 *
 * @param name Its name.
 */
BodyPart::BodyPart(const string &name) :
    name(name) {
    
}


/**
 * @brief Constructs a new hitbox object.
 *
 * @param bpn Name of the body part.
 * @param bpi Index of the body part in the animation database.
 * @param bpp Pointer to the body part.
 * @param pos Hitbox's coordinates, from the center of the mob.
 * @param z Z coordinate of the bottom point of the hitbox.
 * @param height The hitbox's total height.
 * 0 means it spans indefinitely across the Z axis.
 * @param radius Hitbox radius.
 */
Hitbox::Hitbox(
    const string &bpn, size_t bpi, BodyPart* bpp, const Point &pos,
    float z, float height, float radius
) :
    body_part_name(bpn),
    body_part_idx(bpi),
    body_part_ptr(bpp),
    pos(pos),
    z(z),
    height(height),
    radius(radius) {
    
}


/**
 * @brief Returns the coordinates of the hitbox given the mob's
 * location and angle.
 *
 * @param mob_pos The mob's position.
 * @param mob_angle The angle the mob is facing.
 * @return The position.
 */
Point Hitbox::getCurPos(const Point &mob_pos, float mob_angle) const {
    float mob_angle_cos = cos(mob_angle);
    float mob_angle_sin = sin(mob_angle);
    return
        Point(
            mob_pos.x + (pos.x * mob_angle_cos - pos.y * mob_angle_sin),
            mob_pos.y + (pos.x * mob_angle_sin + pos.y * mob_angle_cos)
        );
}


/**
 * @brief Returns the coordinates of the hitbox given the mob's
 * location and angle.
 * If the angle's sine and cosine are known from having been calculated
 * previously, use this function, since it's faster.
 *
 * @param mob_pos The mob's position.
 * @param mob_angle_cos Cosine of the angle the mob is facing.
 * @param mob_angle_sin Sine of the angle the mob is facing.
 * @return The position.
 */
Point Hitbox::getCurPos(
    const Point &mob_pos, float mob_angle_cos, float mob_angle_sin
) const {
    return
        Point(
            mob_pos.x + (pos.x * mob_angle_cos - pos.y * mob_angle_sin),
            mob_pos.y + (pos.x * mob_angle_sin + pos.y * mob_angle_cos)
        );
}
