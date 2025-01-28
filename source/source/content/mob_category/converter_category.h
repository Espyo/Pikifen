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
class converter_category : public mob_category {

public:

    //--- Function declarations ---
    
    converter_category();
    void get_type_names(vector<string> &list) const override;
    mob_type* get_type(const string &internal_name) const override;
    mob_type* create_type() override;
    void register_type(const string &internal_name, mob_type* type) override;
    mob* create_mob(
        const point &pos, mob_type* type, float angle
    ) override;
    void erase_mob(mob* m) override;
    void clear_types() override;
    
};
