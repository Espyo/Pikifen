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

#include "../../core/game.h"
#include "../mob/ship.h"


/**
 * @brief Constructs a new ship category object.
 */
ShipCategory::ShipCategory() :
    MobCategory(
        MOB_CATEGORY_SHIPS, "ship",
        "Ship", "Ships",
        "ships", al_map_rgb(100, 73, 204)
    ) {
    
}


/**
 * @brief Clears the list of registered types of ship.
 */
void ShipCategory::clear_types() {
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
Mob* ShipCategory::create_mob(
    const Point &pos, MobType* type, float angle
) {
    Ship* m = new Ship(pos, (ShipType*) type, angle);
    game.states.gameplay->mobs.ships.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of ship.
 *
 * @return The type.
 */
MobType* ShipCategory::create_type() {
    return new ShipType();
}


/**
 * @brief Clears a ship from the list of ships.
 *
 * @param m The mob to erase.
 */
void ShipCategory::erase_mob(Mob* m) {
    game.states.gameplay->mobs.ships.erase(
        find(
            game.states.gameplay->mobs.ships.begin(),
            game.states.gameplay->mobs.ships.end(),
            (Ship*) m
        )
    );
}


/**
 * @brief Returns a type of ship given its name,
 * or nullptr on error.
 *
 * @param internal_name Internal name of the mob type to get.
 * @return The type, or nullptr on error.
 */
MobType* ShipCategory::get_type(const string &internal_name) const {
    auto it = game.content.mob_types.list.ship.find(internal_name);
    if(it == game.content.mob_types.list.ship.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of ship by internal name.
 *
 * @param list This list gets filled with the mob type internal names.
 */
void ShipCategory::get_type_names(vector<string> &list) const {
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
void ShipCategory::register_type(const string &internal_name, MobType* type) {
    game.content.mob_types.list.ship[internal_name] = (ShipType*) type;
}
