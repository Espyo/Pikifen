/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the treasure mob category class.
 */

#pragma once

#include <string>
#include <vector>

#include "../../core/const.h"
#include "mob_category.h"


using std::string;
using std::vector;


/**
 * @brief Mob category for the treasures.
 */
class TreasureCategory : public MobCategory {

public:

    //--- Public function declarations ---
    
    TreasureCategory();
    void getTypeNames(vector<string>& list) const override;
    MobType* getType(const string& internalName) const override;
    MobType* createType() override;
    void registerType(const string& internalName, MobType* type) override;
    Mob* createMob(
        const Point& pos, MobType* type, float angle
    ) override;
    void deleteMob(Mob* m) override;
    void clearTypes() override;
    
};
