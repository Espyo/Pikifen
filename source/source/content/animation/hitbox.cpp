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
BodyPart::BodyPart(const string& name) :
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
    const string& bpn, size_t bpi, BodyPart* bpp, const Point& pos,
    float z, float height, float radius
) :
    bodyPartName(bpn),
    bodyPartIdx(bpi),
    bodyPartPtr(bpp),
    pos(pos),
    z(z),
    height(height),
    radius(radius) {
    
}


/**
 * @brief Returns the coordinates of the hitbox given the mob's
 * location and angle.
 *
 * @param mobPos The mob's position.
 * @param mobAngle The angle the mob is facing.
 * @return The position.
 */
Point Hitbox::getCurPos(const Point& mobPos, float mobAngle) const {
    float mobAngleCos = cos(mobAngle);
    float mobAngleSin = sin(mobAngle);
    return
        Point(
            mobPos.x + (pos.x * mobAngleCos - pos.y * mobAngleSin),
            mobPos.y + (pos.x * mobAngleSin + pos.y * mobAngleCos)
        );
}


/**
 * @brief Returns the coordinates of the hitbox given the mob's
 * location and angle.
 * If the angle's sine and cosine are known from having been calculated
 * previously, use this function, since it's faster.
 *
 * @param mobPos The mob's position.
 * @param mobAngleCos Cosine of the angle the mob is facing.
 * @param mobAngleSin Sine of the angle the mob is facing.
 * @return The position.
 */
Point Hitbox::getCurPos(
    const Point& mobPos, float mobAngleCos, float mobAngleSin
) const {
    return
        Point(
            mobPos.x + (pos.x * mobAngleCos - pos.y * mobAngleSin),
            mobPos.y + (pos.x * mobAngleSin + pos.y * mobAngleCos)
        );
}
