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

#include "../mobs/ship.h"
#include "../vars.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the ship category.
 */
ship_category::ship_category() :
    mob_category(
        MOB_CATEGORY_SHIPS, "Ship", "Ships",
        SHIPS_FOLDER_PATH, al_map_rgb(128, 128, 192)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Returns all types of ship by name.
 */
void ship_category::get_type_names(vector<string> &list) {
    for(auto t = ship_types.begin(); t != ship_types.end(); ++t) {
        list.push_back(t->first);
    }
}


/* ----------------------------------------------------------------------------
 * Returns a type of ship given its name, or NULL on error.
 */
mob_type* ship_category::get_type(const string &name) {
    auto it = ship_types.find(name);
    if(it == ship_types.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of ship.
 */
mob_type* ship_category::create_type() {
    return new ship_type();
}


/* ----------------------------------------------------------------------------
 * Registers a created type of ship.
 */
void ship_category::register_type(mob_type* type) {
    ship_types[type->name] = (ship_type*) type;
}


/* ----------------------------------------------------------------------------
 * Creates a ship and adds it to the list of ships.
 */
mob* ship_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    ship* m = new ship(pos, (ship_type*) type, angle);
    ships.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Clears a ship from the list of ships.
 */
void ship_category::erase_mob(mob* m) {
    ships.erase(
        find(ships.begin(), ships.end(), (ship*) m)
    );
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of ship.
 */
void ship_category::clear_types() {
    for(auto t = ship_types.begin(); t != ship_types.end(); ++t) {
        delete t->second;
    }
    ship_types.clear();
}


ship_category::~ship_category() { }
