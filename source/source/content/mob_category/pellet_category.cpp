/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pellet mob category class.
 */

#include <algorithm>

#include "pellet_category.h"

#include "../../core/game.h"
#include "../mob/pellet.h"


/**
 * @brief Constructs a new pellet category object.
 */
PelletCategory::PelletCategory() :
    MobCategory(
        MOB_CATEGORY_PELLETS, "pellet",
        "Pellet", "Pellets",
        "pellets", al_map_rgb(73, 204, 126)
    ) {
    
}


/**
 * @brief Clears the list of registered types of pellet.
 */
void PelletCategory::clearTypes() {
    for(auto &t : game.content.mobTypes.list.pellet) {
        delete t.second;
    }
    game.content.mobTypes.list.pellet.clear();
}


/**
 * @brief Creates a pellet and adds it to the list of pellets.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
Mob* PelletCategory::createMob(
    const Point &pos, MobType* type, float angle
) {
    Pellet* m = new Pellet(pos, (PelletType*) type, angle);
    game.states.gameplay->mobs.pellets.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of pellet.
 *
 * @return The type.
 */
MobType* PelletCategory::createType() {
    return new PelletType();
}


/**
 * @brief Clears a pellet from the list of pellets.
 *
 * @param m The mob to erase.
 */
void PelletCategory::eraseMob(Mob* m) {
    game.states.gameplay->mobs.pellets.erase(
        find(
            game.states.gameplay->mobs.pellets.begin(),
            game.states.gameplay->mobs.pellets.end(),
            (Pellet*) m
        )
    );
}


/**
 * @brief Returns a type of pellet given its name,
 * or nullptr on error.
 *
 * @param internal_name Internal name of the mob type to get.
 * @return The type, or nullptr on error.
 */
MobType* PelletCategory::getType(const string &internal_name) const {
    auto it = game.content.mobTypes.list.pellet.find(internal_name);
    if(it == game.content.mobTypes.list.pellet.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of pellet by internal name.
 *
 * @param list This list gets filled with the mob type internal names.
 */
void PelletCategory::getTypeNames(vector<string> &list) const {
    for(auto &t : game.content.mobTypes.list.pellet) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of pellet.
 *
 * @param internal_name Internal name of the mob type.
 * @param type Mob type to register.
 */
void PelletCategory::registerType(const string &internal_name, MobType* type) {
    game.content.mobTypes.list.pellet[internal_name] = (PelletType*) type;
}
