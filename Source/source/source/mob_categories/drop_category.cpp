/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Drop mob category class.
 */

#include <algorithm>

#include "drop_category.h"

#include "../game.h"
#include "../mobs/drop.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the drop category.
 */
drop_category::drop_category() :
    mob_category(
        MOB_CATEGORY_DROPS, "Drop", "Drops",
        "Drops", al_map_rgb(204, 145, 145)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of drops.
 */
void drop_category::clear_types() {
    for(auto &t : game.mob_types.drop) {
        delete t.second;
    }
    game.mob_types.drop.clear();
}


/* ----------------------------------------------------------------------------
 * Creates a drop and adds it to the list of drops.
 * pos:
 *   Starting coordinates.
 * type:
 *   Mob type.
 * angle:
 *   Starting angle.
 */
mob* drop_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    drop* m = new drop(pos, (drop_type*) type, angle);
    game.states.gameplay->mobs.drops.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of drop.
 */
mob_type* drop_category::create_type() {
    return new drop_type();
}


/* ----------------------------------------------------------------------------
 * Clears a drop from the list of drops.
 * m:
 *   The mob to erase.
 */
void drop_category::erase_mob(mob* m) {
    game.states.gameplay->mobs.drops.erase(
        find(
        game.states.gameplay->mobs.drops.begin(),
        game.states.gameplay->mobs.drops.end(),
        (drop*) m
        )
    );
}


/* ----------------------------------------------------------------------------
 * Returns a type of drop given its name, or NULL on error.
 * name:
 *   Name of the mob type to get.
 */
mob_type* drop_category::get_type(const string &name) const {
    auto it = game.mob_types.drop.find(name);
    if(it == game.mob_types.drop.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Returns all types of drop by name.
 * list:
 *   This list gets filled with the mob type names.
 */
void drop_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.mob_types.drop) {
        list.push_back(t.first);
    }
}


/* ----------------------------------------------------------------------------
 * Registers a created type of drop.
 * type:
 *   Mob type to register.
 */
void drop_category::register_type(mob_type* type) {
    game.mob_types.drop[type->name] = (drop_type*) type;
}
