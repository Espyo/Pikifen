/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the Onion mob category class.
 */

#ifndef ONION_CATEGORY_INCLUDED
#define ONION_CATEGORY_INCLUDED

#include <string>
#include <vector>

#include "../const.h"
#include "../mob_categories/mob_category.h"


using std::string;
using std::vector;


/* ----------------------------------------------------------------------------
 * Mob category for the Onions.
 */
class onion_category : public mob_category {
public:
    virtual void get_type_names(vector<string> &list) const;
    virtual mob_type* get_type(const string &name) const;
    virtual mob_type* create_type();
    virtual void register_type(mob_type* type);
    virtual mob* create_mob(
        const point &pos, mob_type* type, const float angle
    );
    virtual void erase_mob(mob* m);
    virtual void clear_types();
    
    onion_category();
};


#endif //ifndef ONION_CATEGORY_INCLUDED
