/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Resource mob category class.
 */

#include <algorithm>

#include "resource_category.h"

#include "../game.h"
#include "../mobs/resource.h"


/**
 * @brief Constructs a new resource category object.
 */
resource_category::resource_category() :
    mob_category(
        MOB_CATEGORY_RESOURCES, "resource",
        "Resource", "Resources",
        "resources", al_map_rgb(139, 204, 204)
    ) {
    
}


/**
 * @brief Clears the list of registered types of resource.
 */
void resource_category::clear_types() {
    for(auto &t : game.content.mob_types.resource) {
        delete t.second;
    }
    game.content.mob_types.resource.clear();
}


/**
 * @brief Creates a resource and adds it to the list of resources.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
mob* resource_category::create_mob(
    const point &pos, mob_type* type, float angle
) {
    resource* m = new resource(pos, (resource_type*) type, angle);
    game.states.gameplay->mobs.resources.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of resource.
 *
 * @return The type.
 */
mob_type* resource_category::create_type() {
    return new resource_type();
}


/**
 * @brief Clears a resource from the list of resources.
 *
 * @param m The mob to erase.
 */
void resource_category::erase_mob(mob* m) {
    game.states.gameplay->mobs.resources.erase(
        find(
            game.states.gameplay->mobs.resources.begin(),
            game.states.gameplay->mobs.resources.end(),
            (resource*) m
        )
    );
}


/**
 * @brief Returns a type of resource given its name.
 *
 * @param name Name of the mob type to get.
 * @return The type, or nullptr on error.
 */
mob_type* resource_category::get_type(const string &name) const {
    auto it = game.content.mob_types.resource.find(name);
    if(it == game.content.mob_types.resource.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of resource by name.
 *
 * @param list This list gets filled with the mob type names.
 */
void resource_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.content.mob_types.resource) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of resource.
 *
 * @param internal_name Internal name of the mob type.
 * @param type Mob type to register.
 */
void resource_category::register_type(const string &internal_name, mob_type* type) {
    game.content.mob_types.resource[internal_name] = (resource_type*) type;
}
