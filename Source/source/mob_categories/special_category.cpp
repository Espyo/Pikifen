/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Special mob category class.
 */

#include <algorithm>

#include "special_category.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates a category for the special mob types.
 */
special_category::special_category() :
    mob_category(
        MOB_CATEGORY_SPECIAL, "Special", "Special",
        "", al_map_rgb(32, 160, 160)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all special types by name.
 */
void special_category::get_type_names(vector<string> &list) {
    for(auto t = spec_mob_types.begin(); t != spec_mob_types.end(); ++t) {
        list.push_back(t->first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a special type given its name, or NULL on error.
 */
mob_type* special_category::get_type(const string &name) {
    auto it = spec_mob_types.find(name);
    if(it == spec_mob_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty special type.
 */
mob_type* special_category::create_type() {
    return new mob_type(MOB_CATEGORY_SPECIAL);
}


/* ----------------------------------------------------------------------------
 * Registers a created special type.
 */
void special_category::register_type(mob_type* type) {
    spec_mob_types[type->name] = type;
}


/* ----------------------------------------------------------------------------
 * Creates a special mob and adds it to the list of special mobs.
 */
mob* special_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    mob* m = new mob(pos, type, angle);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears a special mob from the list of special mobs.
 */
void special_category::erase_mob(mob* m) { }


/* ----------------------------------------------------------------------------
 * Clears the list of special mob types.
 */
void special_category::clear_types() {
    for(auto t = spec_mob_types.begin(); t != spec_mob_types.end(); ++t) {
        //TODO warning: deleting object of polymorphic class type 'mob_type'
        //which has non-virtual destructor might cause undefined behaviour
        //[-Wdelete-non-virtual-dtor]
        delete t->second;
    }
    spec_mob_types.clear();
}


special_category::~special_category() { }
