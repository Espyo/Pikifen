/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Decoration mob category class.
 */

#include <algorithm>

#include "decoration_category.h"

#include "../../core/game.h"
#include "../mob/decoration.h"


/**
 * @brief Constructs a new decoration category object.
 */
DecorationCategory::DecorationCategory() :
    MobCategory(
        MOB_CATEGORY_DECORATIONS, "decoration",
        "Decoration", "Decorations",
        "decorations", al_map_rgb(191, 204, 139)
    ) {
    
}


/**
 * @brief Clears the list of registered types of decorations.
 */
void DecorationCategory::clearTypes() {
    for(auto& t : game.content.mobTypes.list.decoration) {
        delete t.second;
    }
    game.content.mobTypes.list.decoration.clear();
}


/**
 * @brief Creates a decoration and adds it to the list of decorations.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
Mob* DecorationCategory::createMob(
    const Point& pos, MobType* type, float angle
) {
    Decoration* m = new Decoration(pos, (DecorationType*) type, angle);
    game.states.gameplay->mobs.decorations.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of decoration.
 *
 * @return The type.
 */
MobType* DecorationCategory::createType() {
    return new DecorationType();
}


/**
 * @brief Removes and deletes a decoration from the list of decorations.
 *
 * @param m The mob to delete.
 */
void DecorationCategory::deleteMob(Mob* m) {
    game.states.gameplay->mobs.decorations.erase(
        find(
            game.states.gameplay->mobs.decorations.begin(),
            game.states.gameplay->mobs.decorations.end(),
            (Decoration*) m
        )
    );
}


/**
 * @brief Returns a type of decoration given its internal name,
 * or nullptr on error.
 *
 * @param internalName Internal name of the mob type to get.
 * @return The type, or nullptr on error.
 */
MobType* DecorationCategory::getType(const string& internalName) const {
    auto it = game.content.mobTypes.list.decoration.find(internalName);
    if(it == game.content.mobTypes.list.decoration.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of decoration by internal name.
 *
 * @param list This list gets filled with the mob type internal names.
 */
void DecorationCategory::getTypeNames(vector<string>& list) const {
    for(auto& t : game.content.mobTypes.list.decoration) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of decoration.
 *
 * @param internalName Internal name of the mob type.
 * @param type Mob type to register.
 */
void DecorationCategory::registerType(
    const string& internalName, MobType* type
) {
    game.content.mobTypes.list.decoration[internalName] =
        (DecorationType*) type;
}
