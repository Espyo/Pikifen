/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the scale class and scale-related functions.
 */

#pragma once

#include "mob.h"

#include "../mob_type/scale_type.h"


/**
 * @brief A scale is something that measures the weight being applied on
 * top of it, and does something depending on the value.
 */
class Scale : public Mob {

public:

    //--- Members ---
    
    //What type of scale it is.
    ScaleType* scaType = nullptr;
    
    //Weight number that must be met to reach a goal. 0 for none. Type override.
    size_t goalNumber = 0;
    
    
    //--- Function declarations ---
    
    Scale(const Point &pos, ScaleType* type, float angle);
    float calculateCurWeight() const;
    bool getFractionNumbersInfo(
        float* fraction_value_nr, float* fraction_req_nr,
        ALLEGRO_COLOR* fraction_color
    ) const override;
    void readScriptVars(const ScriptVarReader &svr) override;
    
};
