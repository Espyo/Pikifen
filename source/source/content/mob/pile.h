/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the pile class and pile-related functions.
 */

#pragma once

#include "../mob_type/pile_type.h"
#include "mob.h"


/**
 * @brief A pile is an object that represents a collection of
 * resource-type mobs.
 * Pikmin attack it in some form, and it ends up yielding a resource, bit by
 * bit, until it is exhausted.
 */
class Pile : public Mob, public MobWithAnimGroups {

public:

    //--- Members ---
    
    //What type of pile it is.
    PileType* pilType = nullptr;
    
    //Current amount of resources.
    size_t amount = 0;
    
    //Time left until it recharges.
    Timer rechargeTimer;
    
    
    //--- Function declarations ---
    
    Pile(const Point& pos, PileType* type, float angle);
    void changeAmount(int change);
    void recharge();
    void update();
    FRACTION_NR_VISIBILITY getFractionNumbersInfo(
        float* outValueNr, float* outReqNr, ALLEGRO_COLOR* outColor
    ) const override;
    void readScriptVars(const ScriptVarReader& svr) override;
    
protected:

    //--- Function declarations ---
    
    void tickClassSpecifics(float deltaT) override;
    
};
