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


/**
 * @brief Constructs a new pellet category object.
 */
pellet_category::pellet_category() :
    mob_category(
        MOB_CATEGORY_PELLETS, "pellet",
        "Pellet", "Pellets",
        "pellets", al_map_rgb(73, 204, 126)
    ) {
    
}


/**
 * @brief Clears the list of registered types of pellet.
 */
void pellet_category::clear_types() {
    for(auto &t : game.content.mob_types.pellet) {
        delete t.second;
    }
    game.content.mob_types.pellet.clear();
}


/**
 * @brief Creates a pellet and adds it to the list of pellets.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
mob* pellet_category::create_mob(
    const point &pos, mob_type* type, float angle
) {
    pellet* m = new pellet(pos, (pellet_type*) type, angle);
    game.states.gameplay->mobs.pellets.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of pellet.
 *
 * @return The type.
 */
mob_type* pellet_category::create_type() {
    return new pellet_type();
}


/**
 * @brief Clears a pellet from the list of pellets.
 *
 * @param m The mob to erase.
 */
void pellet_category::erase_mob(mob* m) {
    game.states.gameplay->mobs.pellets.erase(
        find(
            game.states.gameplay->mobs.pellets.begin(),
            game.states.gameplay->mobs.pellets.end(),
            (pellet*) m
        )
    );
}


/**
 * @brief Returns a type of pellet given its name.
 *
 * @param name Name of the mob type to get.
 * @return The type, or nullptr on error.
 */
mob_type* pellet_category::get_type(const string &name) const {
    auto it = game.content.mob_types.pellet.find(name);
    if(it == game.content.mob_types.pellet.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of pellet by name.
 *
 * @param list This list gets filled with the mob type names.
 */
void pellet_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.content.mob_types.pellet) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of pellet.
 *
 * @param internal_name Internal name of the mob type.
 * @param type Mob type to register.
 */
void pellet_category::register_type(const string &internal_name, mob_type* type) {
    game.content.mob_types.pellet[internal_name] = (pellet_type*) type;
}
