/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pellet mob category class.
 */

#include <algorithm>

#include "pellet_category.h"

#include "../game.h"
#include "../mobs/pellet.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the pellet category.
 */
pellet_category::pellet_category() :
    mob_category(
        MOB_CATEGORY_PELLETS, "Pellet", "Pellets",
        "Pellets", al_map_rgb(73, 204, 126)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of pellet.
 */
void pellet_category::clear_types() {
    for(auto &t : game.mob_types.pellet) {
        delete t.second;
    }
    game.mob_types.pellet.clear();
}


/* ----------------------------------------------------------------------------
 * Creates a pellet and adds it to the list of pellets.
 * pos:
 *   Starting coordinates.
 * type:
 *   Mob type.
 * angle:
 *   Starting angle.
 */
mob* pellet_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    pellet* m = new pellet(pos, (pellet_type*) type, angle);
    game.states.gameplay_st->mobs.pellets.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of pellet.
 */
mob_type* pellet_category::create_type() {
    return new pellet_type();
}


/* ----------------------------------------------------------------------------
 * Clears a pellet from the list of pellets.
 * m:
 *   The mob to erase.
 */
void pellet_category::erase_mob(mob* m) {
    game.states.gameplay_st->mobs.pellets.erase(
        find(
            game.states.gameplay_st->mobs.pellets.begin(),
            game.states.gameplay_st->mobs.pellets.end(),
            (pellet*) m
        )
    );
}


/* ----------------------------------------------------------------------------
 * Returns a type of pellet given its name, or NULL on error.
 * name:
 *   Name of the mob type to get.
 */
mob_type* pellet_category::get_type(const string &name) const {
    auto it = game.mob_types.pellet.find(name);
    if(it == game.mob_types.pellet.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Returns all types of pellet by name.
 * list:
 *   This list gets filled with the mob type names.
 */
void pellet_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.mob_types.pellet) {
        list.push_back(t.first);
    }
}


/* ----------------------------------------------------------------------------
 * Registers a created type of pellet.
 * type:
 *   Mob type to register.
 */
void pellet_category::register_type(mob_type* type) {
    game.mob_types.pellet[type->name] = (pellet_type*) type;
}
