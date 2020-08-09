/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Ship mob category class.
 */

#include <algorithm>

#include "ship_category.h"

#include "../game.h"
#include "../mobs/ship.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the ship category.
 */
ship_category::ship_category() :
    mob_category(
        MOB_CATEGORY_SHIPS, "Ship", "Ships",
        "Ships", al_map_rgb(100, 73, 204)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of ship.
 */
void ship_category::clear_types() {
    for(auto &t : game.mob_types.ship) {
        delete t.second;
    }
    game.mob_types.ship.clear();
}


/* ----------------------------------------------------------------------------
 * Creates a ship and adds it to the list of ships.
 * pos:
 *   Starting coordinates.
 * type:
 *   Mob type.
 * angle:
 *   Starting angle.
 */
mob* ship_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    ship* m = new ship(pos, (ship_type*) type, angle);
    game.states.gameplay_st->mobs.ships.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of ship.
 */
mob_type* ship_category::create_type() {
    return new ship_type();
}


/* ----------------------------------------------------------------------------
 * Clears a ship from the list of ships.
 * m:
 *   The mob to erase.
 */
void ship_category::erase_mob(mob* m) {
    game.states.gameplay_st->mobs.ships.erase(
        find(
        game.states.gameplay_st->mobs.ships.begin(),
 game.states.gameplay_st->mobs.ships.end(), 
 (ship*) m
 )
    );
}


/* ----------------------------------------------------------------------------
 * Returns a type of ship given its name, or NULL on error.
 * name:
 *   Name of the mob type to get.
 */
mob_type* ship_category::get_type(const string &name) const {
    auto it = game.mob_types.ship.find(name);
    if(it == game.mob_types.ship.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Returns all types of ship by name.
 * list:
 *   This list gets filled with the mob type names.
 */
void ship_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.mob_types.ship) {
        list.push_back(t.first);
    }
}


/* ----------------------------------------------------------------------------
 * Registers a created type of ship.
 * type:
 *   Mob type to register.
 */
void ship_category::register_type(mob_type* type) {
    game.mob_types.ship[type->name] = (ship_type*) type;
}
