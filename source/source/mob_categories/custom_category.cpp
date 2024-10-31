/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Custom mob category class.
 */

#include <algorithm>

#include "custom_category.h"

#include "../game.h"


/**
 * @brief Constructs a new custom category object.
 */
custom_category::custom_category() :
    mob_category(
        MOB_CATEGORY_CUSTOM, "custom",
        "Custom", "Custom",
        "custom", al_map_rgb(178, 73, 204)
    ) {
    
}


/**
 * @brief Clears the list of registered types of custom mob.
 */
void custom_category::clear_types() {
    for(auto &t : game.content.mob_types.list.custom) {
        delete t.second;
    }
    game.content.mob_types.list.custom.clear();
}


/**
 * @brief Creates a custom mob and adds it to the list of custom mobs.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
mob* custom_category::create_mob(
    const point &pos, mob_type* type, float angle
) {
    mob* m = new mob(pos, type, angle);
    return m;
}


/**
 * @brief Creates a new, empty custom type.
 *
 * @return The type.
 */
mob_type* custom_category::create_type() {
    return new mob_type(MOB_CATEGORY_CUSTOM);
}


/**
 * @brief Clears a custom mob from the list of custom mobs.
 *
 * @param m The mob to erase.
 */
void custom_category::erase_mob(mob* m) { }


/**
 * @brief Returns a custom type given its name, or nullptr on error.
 *
 * @param name Name of the mob type to get.
 * @return The type.
 */
mob_type* custom_category::get_type(const string &name) const {
    auto it = game.content.mob_types.list.custom.find(name);
    if(it == game.content.mob_types.list.custom.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all custom types by name.
 *
 * @param list This list gets filled with the mob type names.
 */
void custom_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.content.mob_types.list.custom) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created custom type.
 *
 * @param internal_name Internal name of the mob type.
 * @param type Mob type to register.
 */
void custom_category::register_type(const string &internal_name, mob_type* type) {
    game.content.mob_types.list.custom[internal_name] = type;
}
