/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Pile mob category class.
 */

#include <algorithm>

#include "pile_category.h"

#include "../../core/game.h"
#include "../mob/pile.h"


/**
 * @brief Constructs a new pile category object.
 */
PileCategory::PileCategory() :
    MobCategory(
        MOB_CATEGORY_PILES, "pile",
        "Pile", "Piles",
        "piles", al_map_rgb(139, 204, 165)
    ) {
    
}


/**
 * @brief Clears the list of registered types of pile.
 */
void PileCategory::clearTypes() {
    for(auto &t : game.content.mobTypes.list.pile) {
        delete t.second;
    }
    game.content.mobTypes.list.pile.clear();
}


/**
 * @brief Creates a pile and adds it to the list of piles.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
Mob* PileCategory::createMob(
    const Point &pos, MobType* type, float angle
) {
    Pile* m = new Pile(pos, (PileType*) type, angle);
    game.states.gameplay->mobs.piles.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of pile.
 *
 * @return The type.
 */
MobType* PileCategory::createType() {
    return new PileType();
}


/**
 * @brief Clears a pile from the list of piles.
 *
 * @param m The mob to erase.
 */
void PileCategory::eraseMob(Mob* m) {
    game.states.gameplay->mobs.piles.erase(
        find(
            game.states.gameplay->mobs.piles.begin(),
            game.states.gameplay->mobs.piles.end(),
            (Pile*) m
        )
    );
}


/**
 * @brief Returns a type of pile given its name,
 * or nullptr on error.
 *
 * @param internal_name Internal name of the mob type to get.
 * @return The type, or nullptr on error.
 */
MobType* PileCategory::getType(const string &internal_name) const {
    auto it = game.content.mobTypes.list.pile.find(internal_name);
    if(it == game.content.mobTypes.list.pile.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of pile by internal name.
 *
 * @param list This list gets filled with the mob type internal names.
 */
void PileCategory::getTypeNames(vector<string> &list) const {
    for(auto &t : game.content.mobTypes.list.pile) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of pile.
 *
 * @param internal_name Internal name of the mob type.
 * @param type Mob type to register.
 */
void PileCategory::registerType(const string &internal_name, MobType* type) {
    game.content.mobTypes.list.pile[internal_name] = (PileType*) type;
}
