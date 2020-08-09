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

#include "../game.h"
#include "../mobs/onion.h"


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
 * Clears the list of registered types of Onion.
 */
void onion_category::clear_types() {
    for(auto &t : game.mob_types.onion) {
        delete t.second;
    }
    game.mob_types.onion.clear();
}


/* ----------------------------------------------------------------------------
 * Creates an Onion and adds it to the list of Onions.
 * pos:
 *   Starting coordinates.
 * type:
 *   Mob type.
 * angle:
 *   Starting angle.
 */
mob* onion_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    onion* m = new onion(pos, (onion_type*) type, angle);
    game.states.gameplay_st->mobs.onions.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of Onion.
 */
mob_type* onion_category::create_type() {
    return new onion_type();
}


/* ----------------------------------------------------------------------------
 * Clears an Onion from the list of Onions.
 * m:
 *   The mob to erase.
 */
void onion_category::erase_mob(mob* m) {
    game.states.gameplay_st->mobs.onions.erase(
        find(game.states.gameplay_st->mobs.onions.begin(), game.states.gameplay_st->mobs.onions.end(), (onion*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Returns a type of Onion given its name, or NULL on error.
 * name:
 *   Name of the mob type to get.
 */
mob_type* onion_category::get_type(const string &name) const {
    auto it = game.mob_types.onion.find(name);
    if(it == game.mob_types.onion.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Returns all types of Onion by name.
 * list:
 *   This list gets filled with the mob type names.
 */
void onion_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.mob_types.onion) {
        list.push_back(t.first);
    }
}


/* ----------------------------------------------------------------------------
 * Registers a created type of Onion.
 * type:
 *   Mob type to register.
 */
void onion_category::register_type(mob_type* type) {
    game.mob_types.onion[type->name] = (onion_type*) type;
}
