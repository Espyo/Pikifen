/*
 * Copyright (c) Andre 'Espyo' Silva 2013-2018.
 * The following source file belongs to the open-source project
 * Pikifen. Please read the included
 * README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Treasure mob category class.
 */

#include <algorithm>

#include "treasure_category.h"
#include "../mobs/treasure.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates a treasure category.
 */
treasure_category::treasure_category() :
    mob_category(
        MOB_CATEGORY_TREASURES, "Treasure", "Treasures",
        TREASURES_FOLDER_PATH, al_map_rgb(255, 240, 128)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all types of treasure by name.
 */
void treasure_category::get_type_names(vector<string> &list) {
    for(auto t = treasure_types.begin(); t != treasure_types.end(); ++t) {
        list.push_back(t->first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a type of treasure given its name, or NULL on error.
 */
mob_type* treasure_category::get_type(const string &name) {
    auto it = treasure_types.find(name);
    if(it == treasure_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of treasure.
 */
mob_type* treasure_category::create_type() {
    return new treasure_type();
}


/* ----------------------------------------------------------------------------
 * Registers a created type of treasure.
 */
void treasure_category::register_type(mob_type* type) {
    treasure_types[type->name] = (treasure_type*) type;
}


/* ----------------------------------------------------------------------------
 * Creates a treasure and adds it to the list of treasures.
 */
mob* treasure_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    treasure* m = new treasure(pos, (treasure_type*) type, angle);
    treasures.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears a treasure from the list of treasures.
 */
void treasure_category::erase_mob(mob* m) {
    treasures.erase(
        find(treasures.begin(), treasures.end(), (treasure*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of treasure.
 */
void treasure_category::clear_types() {
    for(auto t = treasure_types.begin(); t != treasure_types.end(); ++t) {
        delete t->second;
    }
    treasure_types.clear();
}


treasure_category::~treasure_category() { }
