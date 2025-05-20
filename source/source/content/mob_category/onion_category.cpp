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

#include "../../core/game.h"
#include "../mob/onion.h"


/**
 * @brief Constructs a new Onion category object.
 */
OnionCategory::OnionCategory() :
    MobCategory(
        MOB_CATEGORY_ONIONS, "onion",
        "Onion", "Onions",
        "onions", al_map_rgb(178, 204, 73)
    ) {
    
}


/**
 * @brief Clears the list of registered types of Onion.
 */
void OnionCategory::clearTypes() {
    for(auto& t : game.content.mobTypes.list.onion) {
        delete t.second;
    }
    game.content.mobTypes.list.onion.clear();
}


/**
 * @brief Creates an Onion and adds it to the list of Onions.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
Mob* OnionCategory::createMob(
    const Point& pos, MobType* type, float angle
) {
    Onion* m = new Onion(pos, (OnionType*) type, angle);
    game.states.gameplay->mobs.onions.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of Onion.
 *
 * @return The type.
 */
MobType* OnionCategory::createType() {
    return new OnionType();
}


/**
 * @brief Clears an Onion from the list of Onions.
 *
 * @param m The mob to erase.
 */
void OnionCategory::eraseMob(Mob* m) {
    game.states.gameplay->mobs.onions.erase(
        find(
            game.states.gameplay->mobs.onions.begin(),
            game.states.gameplay->mobs.onions.end(),
            (Onion*) m
        )
    );
}


/**
 * @brief Returns a type of Onion given its name,
 * or nullptr on error.
 *
 * @param internalName Internal name of the mob type to get.
 * @return The type, or nullptr on error.
 */
MobType* OnionCategory::getType(const string& internalName) const {
    auto it = game.content.mobTypes.list.onion.find(internalName);
    if(it == game.content.mobTypes.list.onion.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of Onion by internal name.
 *
 * @param list This list gets filled with the mob type internal names.
 */
void OnionCategory::getTypeNames(vector<string>& list) const {
    for(auto& t : game.content.mobTypes.list.onion) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of Onion.
 *
 * @param internalName Internal name of the mob type.
 * @param type Mob type to register.
 */
void OnionCategory::registerType(const string& internalName, MobType* type) {
    game.content.mobTypes.list.onion[internalName] = (OnionType*) type;
}
