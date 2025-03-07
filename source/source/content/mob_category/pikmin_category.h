/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Pikmin mob category class.
 */

#pragma once

#include <string>
#include <vector>

#include "../../core/const.h"
#include "mob_category.h"


using std::string;
using std::vector;


/**
 * @brief Mob category for the Pikmin.
 */
class PikminCategory : public MobCategory {

public:

    //--- Function declarations ---
    
    PikminCategory();
    void get_type_names(vector<string> &list) const override;
    MobType* get_type(const string &internal_name) const override;
    MobType* create_type() override;
    void register_type(const string &internal_name, MobType* type) override;
    Mob* create_mob(
        const Point &pos, MobType* type, float angle
    ) override;
    void erase_mob(Mob* m) override;
    void clear_types() override;
    
};
