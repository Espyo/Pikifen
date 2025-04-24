/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pikmin mob category class.
 */

#include <algorithm>

#include "pikmin_category.h"

#include "../../core/game.h"
#include "../mob/pikmin.h"


/**
 * @brief Constructs a new Pikmin category object.
 */
PikminCategory::PikminCategory() :
    MobCategory(
        MOB_CATEGORY_PIKMIN, "pikmin",
        "Pikmin", "Pikmin",
        "pikmin", al_map_rgb(100, 204, 73)
    ) {
    
}


/**
 * @brief Clears the list of registered types of Pikmin.
 */
void PikminCategory::clearTypes() {
    for(auto &t : game.content.mobTypes.list.pikmin) {
        delete t.second;
    }
    game.content.mobTypes.list.pikmin.clear();
}


/**
 * @brief Creates a Pikmin and adds it to the list of Pikmin.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
Mob* PikminCategory::createMob(
    const Point &pos, MobType* type, float angle
) {
    Pikmin* m = new Pikmin(pos, (PikminType*) type, angle);
    game.states.gameplay->mobs.pikmin.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of Pikmin.
 *
 * @return The type.
 */
MobType* PikminCategory::createType() {
    return new PikminType();
}


/**
 * @brief Clears a Pikmin from the list of Pikmin.
 *
 * @param m The mob to erase.
 */
void PikminCategory::eraseMob(Mob* m) {
    game.states.gameplay->mobs.pikmin.erase(
        find(
            game.states.gameplay->mobs.pikmin.begin(),
            game.states.gameplay->mobs.pikmin.end(),
            (Pikmin*) m
        )
    );
}


/**
 * @brief Returns a type of Pikmin given its name,
 * or nullptr on error.
 *
 * @param internalName Internal name of the mob type to get.
 * @return The type, or nullptr on error.
 */
MobType* PikminCategory::getType(const string &internalName) const {
    auto it = game.content.mobTypes.list.pikmin.find(internalName);
    if(it == game.content.mobTypes.list.pikmin.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of Pikmin by internal name.
 *
 * @param list This list gets filled with the mob type internal names.
 */
void PikminCategory::getTypeNames(vector<string> &list) const {
    for(auto &t : game.content.mobTypes.list.pikmin) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of Pikmin.
 *
 * @param internalName Internal name of the mob type.
 * @param type Mob type to register.
 */
void PikminCategory::registerType(const string &internalName, MobType* type) {
    game.content.mobTypes.list.pikmin[internalName] = (PikminType*) type;
}
