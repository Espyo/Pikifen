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

#include "../game.h"
#include "../mobs/bridge.h"


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
 * Clears the list of registered types of bridges.
 */
void bridge_category::clear_types() {
    for(auto &t : game.mob_types.bridge) {
        delete t.second;
    }
    game.mob_types.bridge.clear();
}


/* ----------------------------------------------------------------------------
 * Creates a bridge and adds it to the list of bridges.
 */
mob* bridge_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    bridge* m = new bridge(pos, (bridge_type*) type, angle);
    game.states.gameplay_st->mobs.bridge.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of bridge.
 */
mob_type* bridge_category::create_type() {
    return new bridge_type();
}


/* ----------------------------------------------------------------------------
 * Clears a bridge from the list of bridges.
 */
void bridge_category::erase_mob(mob* m) {
    game.states.gameplay_st->mobs.bridge.erase(
        find(game.states.gameplay_st->mobs.bridge.begin(), game.states.gameplay_st->mobs.bridge.end(), (bridge*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Returns a type of bridge given its name, or NULL on error.
 */
mob_type* bridge_category::get_type(const string &name) const {
    auto it = game.mob_types.bridge.find(name);
    if(it == game.mob_types.bridge.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Returns all types of bridge by name.
 */
void bridge_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.mob_types.bridge) {
        list.push_back(t.first);
    }
}


/* ----------------------------------------------------------------------------
 * Registers a created type of bridge.
 */
void bridge_category::register_type(mob_type* type) {
    game.mob_types.bridge[type->name] = (bridge_type*) type;
}
