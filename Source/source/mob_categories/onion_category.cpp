/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Onion mob category class.
 */

#include <algorithm>

#include "onion_category.h"

#include "../mobs/onion.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the Onion category.
 */
onion_category::onion_category() :
    mob_category(
        MOB_CATEGORY_ONIONS, "Onion", "Onions",
        "Onions", al_map_rgb(178, 204, 73)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all types of Onion by name.
 */
void onion_category::get_type_names(vector<string> &list) {
    for(auto &t : onion_types) {
        list.push_back(t.first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a type of Onion given its name, or NULL on error.
 */
mob_type* onion_category::get_type(const string &name) {
    auto it = onion_types.find(name);
    if(it == onion_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of Onion.
 */
mob_type* onion_category::create_type() {
    return new onion_type();
}


/* ----------------------------------------------------------------------------
 * Registers a created type of Onion.
 */
void onion_category::register_type(mob_type* type) {
    onion_types[type->name] = (onion_type*) type;
}


/* ----------------------------------------------------------------------------
 * Creates an Onion and adds it to the list of Onions.
 */
mob* onion_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    onion* m = new onion(pos, (onion_type*) type, angle);
    onions.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears an Onion from the list of Onions.
 */
void onion_category::erase_mob(mob* m) {
    onions.erase(
        find(onions.begin(), onions.end(), (onion*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of Onion.
 */
void onion_category::clear_types() {
    for(auto &t : onion_types) {
        delete t.second;
    }
    onion_types.clear();
}


onion_category::~onion_category() { }
