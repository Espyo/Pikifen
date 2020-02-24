/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pikmin mob category class.
 */

#include <algorithm>

#include "pikmin_category.h"

#include "../mobs/pikmin.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the Pikmin category.
 */
pikmin_category::pikmin_category() :
    mob_category(
        MOB_CATEGORY_PIKMIN, "Pikmin", "Pikmin",
        "Pikmin", al_map_rgb(100, 204, 73)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all types of Pikmin by name.
 */
void pikmin_category::get_type_names(vector<string> &list) {
    for(auto t = pikmin_types.begin(); t != pikmin_types.end(); ++t) {
        list.push_back(t->first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a type of Pikmin given its name, or NULL on error.
 */
mob_type* pikmin_category::get_type(const string &name) {
    auto it = pikmin_types.find(name);
    if(it == pikmin_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of Pikmin.
 */
mob_type* pikmin_category::create_type() {
    return new pikmin_type();
}


/* ----------------------------------------------------------------------------
 * Registers a created type of Pikmin.
 */
void pikmin_category::register_type(mob_type* type) {
    pikmin_types[type->name] = (pikmin_type*) type;
}


/* ----------------------------------------------------------------------------
 * Creates a Pikmin and adds it to the list of Pikmin.
 */
mob* pikmin_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    pikmin* m = new pikmin(pos, (pikmin_type*) type, angle);
    pikmin_list.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears a Pikmin from the list of Pikmin.
 */
void pikmin_category::erase_mob(mob* m) {
    pikmin_list.erase(
        find(pikmin_list.begin(), pikmin_list.end(), (pikmin*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of Pikmin.
 */
void pikmin_category::clear_types() {
    for(auto t = pikmin_types.begin(); t != pikmin_types.end(); ++t) {
        delete t->second;
    }
    pikmin_types.clear();
}

pikmin_category::~pikmin_category() { }
