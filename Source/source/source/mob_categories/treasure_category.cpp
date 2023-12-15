/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Treasure mob category class.
 */

#include <algorithm>

#include "treasure_category.h"

#include "../game.h"
#include "../mobs/treasure.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the treasure category.
 */
treasure_category::treasure_category() :
    mob_category(
        MOB_CATEGORY_TREASURES, "Treasure", "Treasures",
        "Treasures", al_map_rgb(204, 151, 71)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of treasure.
 */
void treasure_category::clear_types() {
    for(auto &t : game.mob_types.treasure) {
        delete t.second;
    }
    game.mob_types.treasure.clear();
}


/* ----------------------------------------------------------------------------
 * Creates a treasure and adds it to the list of treasures.
 * pos:
 *   Starting coordinates.
 * type:
 *   Mob type.
 * angle:
 *   Starting angle.
 */
mob* treasure_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    treasure* m = new treasure(pos, (treasure_type*) type, angle);
    game.states.gameplay->mobs.treasures.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of treasure.
 */
mob_type* treasure_category::create_type() {
    return new treasure_type();
}


/* ----------------------------------------------------------------------------
 * Clears a treasure from the list of treasures.
 * m:
 *   The mob to erase.
 */
void treasure_category::erase_mob(mob* m) {
    game.states.gameplay->mobs.treasures.erase(
        find(
            game.states.gameplay->mobs.treasures.begin(),
            game.states.gameplay->mobs.treasures.end(),
            (treasure*) m
        )
    );
}


/* ----------------------------------------------------------------------------
 * Returns a type of treasure given its name, or NULL on error.
 * name:
 *   Name of the mob type to get.
 */
mob_type* treasure_category::get_type(const string &name) const {
    auto it = game.mob_types.treasure.find(name);
    if(it == game.mob_types.treasure.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Returns all types of treasure by name.
 * list:
 *   This list gets filled with the mob type names.
 */
void treasure_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.mob_types.treasure) {
        list.push_back(t.first);
    }
}


/* ----------------------------------------------------------------------------
 * Registers a created type of treasure.
 * type:
 *   Mob type to register.
 */
void treasure_category::register_type(mob_type* type) {
    game.mob_types.treasure[type->name] = (treasure_type*) type;
}
