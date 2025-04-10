/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Interectable mob category class.
 */

#include <algorithm>

#include "interactable_category.h"

#include "../../core/game.h"
#include "../mob_type/interactable_type.h"


/**
 * @brief Constructs a new interactable category object.
 */
InteractableCategory::InteractableCategory() :
    MobCategory(
        MOB_CATEGORY_INTERACTABLES, "interactable",
        "Interactable", "Interactables",
        "interactables", al_map_rgb(204, 139, 178)
    ) {
    
}


/**
 * @brief Clears the list of registered types of interactables.
 */
void InteractableCategory::clearTypes() {
    for(auto &t : game.content.mob_types.list.interactable) {
        delete t.second;
    }
    game.content.mob_types.list.interactable.clear();
}


/**
 * @brief Creates an interactable and adds it to the list of interactables.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
Mob* InteractableCategory::createMob(
    const Point &pos, MobType* type, float angle
) {
    Interactable* m = new Interactable(pos, (InteractableType*) type, angle);
    game.states.gameplay->mobs.interactables.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of interactable.
 *
 * @return The type.
 */
MobType* InteractableCategory::createType() {
    return new InteractableType();
}


/**
 * @brief Clears an interactable from the list of interactables.
 *
 * @param m The mob to erase.
 */
void InteractableCategory::eraseMob(Mob* m) {
    game.states.gameplay->mobs.interactables.erase(
        find(
            game.states.gameplay->mobs.interactables.begin(),
            game.states.gameplay->mobs.interactables.end(),
            (Interactable*) m
        )
    );
}


/**
 * @brief Returns a type of interactable given its name,
 * or nullptr on error.
 *
 * @param internal_name Internal name of the mob type to get.
 * @return The type, or nullptr on error.
 */
MobType* InteractableCategory::getType(const string &internal_name) const {
    auto it = game.content.mob_types.list.interactable.find(internal_name);
    if(it == game.content.mob_types.list.interactable.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of interactable by internal name.
 *
 * @param list This list gets filled with the mob type internal names.
 */
void InteractableCategory::getTypeNames(vector<string> &list) const {
    for(auto &t : game.content.mob_types.list.interactable) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of interactable.
 *
 * @param internal_name Internal name of the mob type.
 * @param type Mob type to register.
 */
void InteractableCategory::registerType(const string &internal_name, MobType* type) {
    game.content.mob_types.list.interactable[internal_name] = (InteractableType*) type;
}
