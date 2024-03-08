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


/**
 * @brief Constructs a new drop category object.
 */
drop_category::drop_category() :
    mob_category(
        MOB_CATEGORY_DROPS, "Drop", "Drops",
        "Drops", al_map_rgb(204, 145, 145)
    ) {
    
}


/**
 * @brief Clears the list of registered types of drops.
 */
void drop_category::clear_types() {
    for(auto &t : game.mob_types.drop) {
        delete t.second;
    }
    game.mob_types.drop.clear();
}


/**
 * @brief Creates a drop and adds it to the list of drops.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
mob* drop_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    drop* m = new drop(pos, (drop_type*) type, angle);
    game.states.gameplay->mobs.drops.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of drop.
 *
 * @return The type.
 */
mob_type* drop_category::create_type() {
    return new drop_type();
}


/**
 * @brief Clears a drop from the list of drops.
 *
 * @param m The mob to erase.
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


/**
 * @brief Returns a type of drop given its name.
 *
 * @param name Name of the mob type to get.
 * @return The type, or nullptr on error.
 */
mob_type* drop_category::get_type(const string &name) const {
    auto it = game.mob_types.drop.find(name);
    if(it == game.mob_types.drop.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of drop by name.
 *
 * @param list This list gets filled with the mob type names.
 */
void drop_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.mob_types.drop) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of drop.
 *
 * @param type Mob type to register.
 */
void drop_category::register_type(mob_type* type) {
    game.mob_types.drop[type->name] = (drop_type*) type;
}
