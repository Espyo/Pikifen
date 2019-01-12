/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Header for the bridge mob category class.
 */

#ifndef BRIDGE_CATEGORY_INCLUDED
#define BRIDGE_CATEGORY_INCLUDED

#include <string>
#include <vector>

#include "../const.h"
#include "../mob_categories/mob_category.h"

using namespace std;


const string BRIDGES_FOLDER_PATH = TYPES_FOLDER_PATH + "/Bridges";


/* ----------------------------------------------------------------------------
 * Mob category for the bridges.
 */
class bridge_category : public mob_category {
public:
    virtual void get_type_names(vector<string> &list);
    virtual mob_type* get_type(const string &name);
    virtual mob_type* create_type();
    virtual void register_type(mob_type* type);
    virtual mob* create_mob(
        const point &pos, mob_type* type, const float angle
    );
    virtual void erase_mob(mob* m);
    virtual void clear_types();
    
    bridge_category();
    ~bridge_category();
};

#endif //ifndef BRIDGE_CATEGORY_INCLUDED
