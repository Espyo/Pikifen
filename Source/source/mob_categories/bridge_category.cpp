/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Bridge mob category class.
 */

#include <algorithm>

#include "bridge_category.h"

#include "../mobs/bridge.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the bridge category.
 */
bridge_category::bridge_category() :
    mob_category(
        MOB_CATEGORY_BRIDGES, "Bridge", "Bridges",
        "Bridges", al_map_rgb(204, 73, 152)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all types of bridge by name.
 */
void bridge_category::get_type_names(vector<string> &list) {
    for(auto &t : bridge_types) {
        list.push_back(t.first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a type of bridge given its name, or NULL on error.
 */
mob_type* bridge_category::get_type(const string &name) {
    auto it = bridge_types.find(name);
    if(it == bridge_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of bridge.
 */
mob_type* bridge_category::create_type() {
    return new bridge_type();
}


/* ----------------------------------------------------------------------------
 * Registers a created type of bridge.
 */
void bridge_category::register_type(mob_type* type) {
    bridge_types[type->name] = (bridge_type*) type;
}


/* ----------------------------------------------------------------------------
 * Creates a bridge and adds it to the list of bridges.
 */
mob* bridge_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    bridge* m = new bridge(pos, (bridge_type*) type, angle);
    bridges.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears a bridge from the list of bridges.
 */
void bridge_category::erase_mob(mob* m) {
    bridges.erase(
        find(bridges.begin(), bridges.end(), (bridge*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of bridges.
 */
void bridge_category::clear_types() {
    for(auto &t : bridge_types) {
        delete t.second;
    }
    bridge_types.clear();
}


bridge_category::~bridge_category() { }
