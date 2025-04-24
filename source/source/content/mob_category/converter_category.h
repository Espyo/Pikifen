/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the converter mob category class.
 */

#pragma once

#include <string>
#include <vector>

#include "../../core/const.h"
#include "mob_category.h"


using std::string;
using std::vector;


/**
 * @brief Mob category for mobs that can convert Pikmin from one type
 * to another.
 */
class ConverterCategory : public MobCategory {

public:

    //--- Function declarations ---
    
    ConverterCategory();
    void getTypeNames(vector<string> &list) const override;
    MobType* getType(const string &internalName) const override;
    MobType* createType() override;
    void registerType(const string &internalName, MobType* type) override;
    Mob* createMob(
        const Point &pos, MobType* type, float angle
    ) override;
    void eraseMob(Mob* m) override;
    void clearTypes() override;
    
};
