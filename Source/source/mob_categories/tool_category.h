/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the tool mob category class.
 */

#pragma once

#include <string>
#include <vector>

#include "../const.h"
#include "../mob_categories/mob_category.h"


using std::string;
using std::vector;


/**
 * @brief Mob category for tool-like carriable objects.
 */
class tool_category : public mob_category {

public:

    //--- Function declarations ---

    tool_category();
    void get_type_names(vector<string> &list) const override;
    mob_type* get_type(const string &name) const override;
    mob_type* create_type() override;
    void register_type(mob_type* type) override;
    mob* create_mob(
        const point &pos, mob_type* type, const float angle
    ) override;
    void erase_mob(mob* m) override;
    void clear_types() override;
    
};
