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


/**
 * @brief Constructs a new ship category object.
 */
ship_category::ship_category() :
    mob_category(
        MOB_CATEGORY_SHIPS, "ship",
        "Ship", "Ships",
        "ships", al_map_rgb(100, 73, 204)
    ) {
    
}


/**
 * @brief Clears the list of registered types of ship.
 */
void ship_category::clear_types() {
    for(auto &t : game.content.mob_types.list.ship) {
        delete t.second;
    }
    game.content.mob_types.list.ship.clear();
}


/**
 * @brief Creates a ship and adds it to the list of ships.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
mob* ship_category::create_mob(
    const point &pos, mob_type* type, float angle
) {
    ship* m = new ship(pos, (ship_type*) type, angle);
    game.states.gameplay->mobs.ships.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of ship.
 *
 * @return The type.
 */
mob_type* ship_category::create_type() {
    return new ship_type();
}


/**
 * @brief Clears a ship from the list of ships.
 *
 * @param m The mob to erase.
 */
void ship_category::erase_mob(mob* m) {
    game.states.gameplay->mobs.ships.erase(
        find(
            game.states.gameplay->mobs.ships.begin(),
            game.states.gameplay->mobs.ships.end(),
            (ship*) m
        )
    );
}


/**
 * @brief Returns a type of ship given its name.
 *
 * @param name Name of the mob type to get.
 * @return The type, or nullptr on error.
 */
mob_type* ship_category::get_type(const string &name) const {
    auto it = game.content.mob_types.list.ship.find(name);
    if(it == game.content.mob_types.list.ship.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of ship by name.
 *
 * @param list This list gets filled with the mob type names.
 */
void ship_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.content.mob_types.list.ship) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of ship.
 *
 * @param internal_name Internal name of the mob type.
 * @param type Mob type to register.
 */
void ship_category::register_type(const string &internal_name, mob_type* type) {
    game.content.mob_types.list.ship[internal_name] = (ship_type*) type;
}
