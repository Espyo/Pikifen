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
    DecorationType* dec_type = nullptr;
    
    //Whether this decoration is allowed to use the random anim delay or not.
    bool individual_random_anim_delay = true;
    
    //This decoration's random tint, if any.
    ALLEGRO_COLOR individual_tint = COLOR_WHITE;
    
    //This decoration's random scale, if any.
    float individual_scale = 1.0f;
    
    //This decoration's random rotation, if any.
    float individual_rotation = 0.0f;
    
    
    //--- Function declarations ---
    
    Decoration(const Point &pos, DecorationType* dec_type, float angle);
    void draw_mob() override;
    void read_script_vars(const ScriptVarReader &svr) override;
    
};
