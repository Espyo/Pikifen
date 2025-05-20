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

#include "../../core/game.h"
#include "../mob/drop.h"


/**
 * @brief Constructs a new drop category object.
 */
DropCategory::DropCategory() :
    MobCategory(
        MOB_CATEGORY_DROPS, "drop",
        "Drop", "Drops",
        "drops", al_map_rgb(204, 145, 145)
    ) {
    
}


/**
 * @brief Clears the list of registered types of drops.
 */
void DropCategory::clearTypes() {
    for(auto& t : game.content.mobTypes.list.drop) {
        delete t.second;
    }
    game.content.mobTypes.list.drop.clear();
}


/**
 * @brief Creates a drop and adds it to the list of drops.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
Mob* DropCategory::createMob(
    const Point& pos, MobType* type, float angle
) {
    Drop* m = new Drop(pos, (DropType*) type, angle);
    game.states.gameplay->mobs.drops.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of drop.
 *
 * @return The type.
 */
MobType* DropCategory::createType() {
    return new DropType();
}


/**
 * @brief Clears a drop from the list of drops.
 *
 * @param m The mob to erase.
 */
void DropCategory::eraseMob(Mob* m) {
    game.states.gameplay->mobs.drops.erase(
        find(
            game.states.gameplay->mobs.drops.begin(),
            game.states.gameplay->mobs.drops.end(),
            (Drop*) m
        )
    );
}


/**
 * @brief Returns a type of drop given its internal name,
 * or nullptr on error.
 *
 * @param internalName Internal name of the mob type to get.
 * @return The type, or nullptr on error.
 */
MobType* DropCategory::getType(const string& internalName) const {
    auto it = game.content.mobTypes.list.drop.find(internalName);
    if(it == game.content.mobTypes.list.drop.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of drop by internal name.
 *
 * @param list This list gets filled with the mob type internal names.
 */
void DropCategory::getTypeNames(vector<string>& list) const {
    for(auto& t : game.content.mobTypes.list.drop) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of drop.
 *
 * @param internalName Internal name of the mob type.
 * @param type Mob type to register.
 */
void DropCategory::registerType(const string& internalName, MobType* type) {
    game.content.mobTypes.list.drop[internalName] = (DropType*) type;
}
