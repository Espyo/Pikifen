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

#include "../../core/game.h"
#include "../mob/bridge.h"


/**
 * @brief Constructs a new bridge category object.
 */
bridge_category::bridge_category() :
    mob_category(
        MOB_CATEGORY_BRIDGES, "bridge",
        "Bridge", "Bridges",
        "bridges", al_map_rgb(204, 73, 152)
    ) {
    
}


/**
 * @brief Clears the list of registered types of bridges.
 */
void bridge_category::clear_types() {
    for(auto &t : game.content.mob_types.list.bridge) {
        delete t.second;
    }
    game.content.mob_types.list.bridge.clear();
}


/**
 * @brief Creates a bridge and adds it to the list of bridges.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
mob* bridge_category::create_mob(
    const point &pos, mob_type* type, float angle
) {
    bridge* m = new bridge(pos, (bridge_type*) type, angle);
    game.states.gameplay->mobs.bridges.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of bridge.
 *
 * @return The type.
 */
mob_type* bridge_category::create_type() {
    return new bridge_type();
}


/**
 * @brief Clears a bridge from the list of bridges.
 *
 * @param m The mob to erase.
 */
void bridge_category::erase_mob(mob* m) {
    game.states.gameplay->mobs.bridges.erase(
        find(
            game.states.gameplay->mobs.bridges.begin(),
            game.states.gameplay->mobs.bridges.end(),
            (bridge*) m
        )
    );
}


/**
 * @brief Returns a type of bridge given its internal name,
 * or nullptr on error.
 *
 * @param internal_name Internal name of the mob type to get.
 * @return The type, or nullptr on error.
 */
mob_type* bridge_category::get_type(const string &internal_name) const {
    auto it = game.content.mob_types.list.bridge.find(internal_name);
    if(it == game.content.mob_types.list.bridge.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of bridge by internal name.
 *
 * @param list This list gets filled with the mob type internal names.
 */
void bridge_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.content.mob_types.list.bridge) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of bridge.
 *
 * @param internal_name Internal name of the mob type.
 * @param type Mob type to register.
 */
void bridge_category::register_type(const string &internal_name, mob_type* type) {
    game.content.mob_types.list.bridge[internal_name] = (bridge_type*) type;
}
