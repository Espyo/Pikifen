/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the hitbox class and hitbox-related functions.
 */

#pragma once

#include <vector>

#include "../../core/const.h"
#include "../../util/general_utils.h"
#include "../../util/geometry_utils.h"


using std::string;
using std::vector;


//Types of hitboxes.
enum HITBOX_TYPE {

    //Can be hurt by "attack"-type hitboxes.
    HITBOX_TYPE_NORMAL,
    
    //Hurts "normal"-type hitboxes.
    HITBOX_TYPE_ATTACK,
    
    //Currently disabled.
    HITBOX_TYPE_DISABLED,
    
};


struct Hazard;


/**
 * @brief A body part.
 */
class BodyPart {

public:

    //--- Members ---
    
    //The body part's name.
    string name;
    
    
    //--- Function declarations ---
    
    explicit BodyPart(const string &name = "");
    
};


/**
 * @brief A hitbox in a sprite. Despite the name, it is a cilinder.
 */
class Hitbox {

public:

    //--- Members ---
    
    //The name of the body part to use.
    string bodyPartName;
    
    //Index of the body part. Cache for performance.
    size_t bodyPartIdx;
    
    //Pointer to the body part. Cache for performance.
    BodyPart* bodyPartPtr = nullptr;
    
    //Center of the hitbox (relative coordinates).
    Point pos;
    
    //Bottom of the hitbox (relative coordinates).
    float z = 0.0f;
    
    //Total hitbox height.
    float height = 128.0f;
    
    //Hitbox radius.
    float radius = 32.0f;
    
    //Type of hitbox.
    HITBOX_TYPE type = HITBOX_TYPE_NORMAL;
    
    //Hazard, if any.
    Hazard* hazard = nullptr;
    
    //If it's a normal hitbox, this is the defense multiplier.
    //If it's an attack one, the attack power.
    float value = 1.0f;
    
    //If true, the Pikmin is knocked away from the center.
    bool knockbackOutward = false;
    
    //Knockback angle.
    float knockbackAngle = 0.0f;
    
    //Knockback strength.
    float knockback = 0.0f;
    
    //Chance of this attack withering a Pikmin's maturity (0-100).
    unsigned char witherChance = 0.0f;
    
    //Can the Pikmin latch on to this hitbox to continue inflicting damage?
    //Example of a non-latchable hitbox: Goolix' larger core.
    bool canPikminLatch = false;
    
    
    //--- Function declarations ---
    
    explicit Hitbox(
        const string &bpn = "", size_t bpi = INVALID, BodyPart* bpp = nullptr,
        const Point &pos = Point(), float z = 0,
        float height = 128, float radius = 32
    );
    Point getCurPos(
        const Point &mobPos, float mobAngle
    ) const;
    Point getCurPos(
        const Point &mobPos,
        float mobAngleCos, float mobAngleSin
    ) const;
    
};
