/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the ship mob category class.
 */

#ifndef SHIP_CATEGORY_INCLUDED
#define SHIP_CATEGORY_INCLUDED

#include <string>
#include <vector>

#include "../const.h"
#include "../mob_categories/mob_category.h"


using std::string;
using std::vector;


/* ----------------------------------------------------------------------------
 * Mob category for the ships.
 */
class ship_category : public mob_category {
public:
    void get_type_names(vector<string> &list) const;
    mob_type* get_type(const string &name) const;
    mob_type* create_type();
    void register_type(mob_type* type);
    mob* create_mob(
        const point &pos, mob_type* type, const float angle
    );
    void erase_mob(mob* m);
    void clear_types();
    
    ship_category();
};


#endif //ifndef SHIP_CATEGORY_INCLUDED
