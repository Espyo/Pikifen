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


/**
 * @brief Constructs a new Onion category object.
 */
onion_category::onion_category() :
    mob_category(
        MOB_CATEGORY_ONIONS, "Onion", "Onions",
        "Onions", al_map_rgb(178, 204, 73)
    ) {
    
}


/**
 * @brief Clears the list of registered types of Onion.
 */
void onion_category::clear_types() {
    for(auto &t : game.content.mob_types.onion) {
        delete t.second;
    }
    game.content.mob_types.onion.clear();
}


/**
 * @brief Creates an Onion and adds it to the list of Onions.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
mob* onion_category::create_mob(
    const point &pos, mob_type* type, float angle
) {
    onion* m = new onion(pos, (onion_type*) type, angle);
    game.states.gameplay->mobs.onions.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of Onion.
 *
 * @return The type.
 */
mob_type* onion_category::create_type() {
    return new onion_type();
}


/**
 * @brief Clears an Onion from the list of Onions.
 *
 * @param m The mob to erase.
 */
void onion_category::erase_mob(mob* m) {
    game.states.gameplay->mobs.onions.erase(
        find(
            game.states.gameplay->mobs.onions.begin(),
            game.states.gameplay->mobs.onions.end(),
            (onion*) m
        )
    );
}


/**
 * @brief Returns a type of Onion given its name.
 *
 * @param name Name of the mob type to get.
 * @return The type, or nullptr on error.
 */
mob_type* onion_category::get_type(const string &name) const {
    auto it = game.content.mob_types.onion.find(name);
    if(it == game.content.mob_types.onion.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of Onion by name.
 *
 * @param list This list gets filled with the mob type names.
 */
void onion_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.content.mob_types.onion) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of Onion.
 *
 * @param type Mob type to register.
 */
void onion_category::register_type(mob_type* type) {
    game.content.mob_types.onion[type->name] = (onion_type*) type;
}
