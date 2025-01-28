/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Treasure mob category class.
 */

#include <algorithm>

#include "treasure_category.h"

#include "../../core/game.h"
#include "../mob/treasure.h"


/**
 * @brief Constructs a new treasure category object.
 */
treasure_category::treasure_category() :
    mob_category(
        MOB_CATEGORY_TREASURES, "treasure",
        "Treasure", "Treasures",
        "treasures", al_map_rgb(204, 151, 71)
    ) {
    
}


/**
 * @brief Clears the list of registered types of treasure.
 */
void treasure_category::clear_types() {
    for(auto &t : game.content.mob_types.list.treasure) {
        delete t.second;
    }
    game.content.mob_types.list.treasure.clear();
}


/**
 * @brief Creates a treasure and adds it to the list of treasures.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
mob* treasure_category::create_mob(
    const point &pos, mob_type* type, float angle
) {
    treasure* m = new treasure(pos, (treasure_type*) type, angle);
    game.states.gameplay->mobs.treasures.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of treasure.
 *
 * @return The type.
 */
mob_type* treasure_category::create_type() {
    return new treasure_type();
}


/**
 * @brief Clears a treasure from the list of treasures.
 *
 * @param m The mob to erase.
 */
void treasure_category::erase_mob(mob* m) {
    game.states.gameplay->mobs.treasures.erase(
        find(
            game.states.gameplay->mobs.treasures.begin(),
            game.states.gameplay->mobs.treasures.end(),
            (treasure*) m
        )
    );
}


/**
 * @brief Returns a type of treasure given its name,
 * or nullptr on error.
 *
 * @param internal_name Internal name of the mob type to get.
 * @return The type, or nullptr on error.
 */
mob_type* treasure_category::get_type(const string &internal_name) const {
    auto it = game.content.mob_types.list.treasure.find(internal_name);
    if(it == game.content.mob_types.list.treasure.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of treasure by internal name.
 *
 * @param list This list gets filled with the mob type internal names.
 */
void treasure_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.content.mob_types.list.treasure) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of treasure.
 *
 * @param internal_name Internal name of the mob type.
 * @param type Mob type to register.
 */
void treasure_category::register_type(const string &internal_name, mob_type* type) {
    game.content.mob_types.list.treasure[internal_name] = (treasure_type*) type;
}
