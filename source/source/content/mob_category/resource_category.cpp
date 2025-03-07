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

#include "../../core/game.h"
#include "../mob/resource.h"


/**
 * @brief Constructs a new resource category object.
 */
ResourceCategory::ResourceCategory() :
    MobCategory(
        MOB_CATEGORY_RESOURCES, "resource",
        "Resource", "Resources",
        "resources", al_map_rgb(139, 204, 204)
    ) {
    
}


/**
 * @brief Clears the list of registered types of resource.
 */
void ResourceCategory::clear_types() {
    for(auto &t : game.content.mob_types.list.resource) {
        delete t.second;
    }
    game.content.mob_types.list.resource.clear();
}


/**
 * @brief Creates a resource and adds it to the list of resources.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
Mob* ResourceCategory::create_mob(
    const Point &pos, MobType* type, float angle
) {
    Resource* m = new Resource(pos, (ResourceType*) type, angle);
    game.states.gameplay->mobs.resources.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of resource.
 *
 * @return The type.
 */
MobType* ResourceCategory::create_type() {
    return new ResourceType();
}


/**
 * @brief Clears a resource from the list of resources.
 *
 * @param m The mob to erase.
 */
void ResourceCategory::erase_mob(Mob* m) {
    game.states.gameplay->mobs.resources.erase(
        find(
            game.states.gameplay->mobs.resources.begin(),
            game.states.gameplay->mobs.resources.end(),
            (Resource*) m
        )
    );
}


/**
 * @brief Returns a type of resource given its name,
 * or nullptr on error.
 *
 * @param internal_name Internal name of the mob type to get.
 * @return The type, or nullptr on error.
 */
MobType* ResourceCategory::get_type(const string &internal_name) const {
    auto it = game.content.mob_types.list.resource.find(internal_name);
    if(it == game.content.mob_types.list.resource.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of resource by internal name.
 *
 * @param list This list gets filled with the mob type internal names.
 */
void ResourceCategory::get_type_names(vector<string> &list) const {
    for(auto &t : game.content.mob_types.list.resource) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of resource.
 *
 * @param internal_name Internal name of the mob type.
 * @param type Mob type to register.
 */
void ResourceCategory::register_type(const string &internal_name, MobType* type) {
    game.content.mob_types.list.resource[internal_name] = (ResourceType*) type;
}
