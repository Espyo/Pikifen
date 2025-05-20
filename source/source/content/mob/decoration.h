/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the decoration class and decoration-related functions.
 */

#pragma once

#include "../mob_type/decoration_type.h"
#include "mob.h"


/**
 * @brief A decoration mob, like a plant.
 * They don't do much other than be pretty and get bumped occasionally.
 */
class Decoration : public Mob {

public:

    //--- Members ---
    
    //What type of decoration it is.
    DecorationType* decType = nullptr;
    
    //Whether this decoration is allowed to use the random anim delay or not.
    bool individualRandomAnimDelay = true;
    
    //This decoration's random tint, if any.
    ALLEGRO_COLOR individualTint = COLOR_WHITE;
    
    //This decoration's random scale, if any.
    float individualScale = 1.0f;
    
    //This decoration's random rotation, if any.
    float individualRotation = 0.0f;
    
    
    //--- Function declarations ---
    
    Decoration(const Point& pos, DecorationType* type, float angle);
    void drawMob() override;
    void readScriptVars(const ScriptVarReader& svr) override;
    
};
