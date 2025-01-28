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
pile_category::pile_category() :
    mob_category(
        MOB_CATEGORY_PILES, "pile",
        "Pile", "Piles",
        "piles", al_map_rgb(139, 204, 165)
    ) {
    
}


/**
 * @brief Clears the list of registered types of pile.
 */
void pile_category::clear_types() {
    for(auto &t : game.content.mob_types.list.pile) {
        delete t.second;
    }
    game.content.mob_types.list.pile.clear();
}


/**
 * @brief Creates a pile and adds it to the list of piles.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
mob* pile_category::create_mob(
    const point &pos, mob_type* type, float angle
) {
    pile* m = new pile(pos, (pile_type*) type, angle);
    game.states.gameplay->mobs.piles.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of pile.
 *
 * @return The type.
 */
mob_type* pile_category::create_type() {
    return new pile_type();
}


/**
 * @brief Clears a pile from the list of piles.
 *
 * @param m The mob to erase.
 */
void pile_category::erase_mob(mob* m) {
    game.states.gameplay->mobs.piles.erase(
        find(
            game.states.gameplay->mobs.piles.begin(),
            game.states.gameplay->mobs.piles.end(),
            (pile*) m
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
mob_type* pile_category::get_type(const string &internal_name) const {
    auto it = game.content.mob_types.list.pile.find(internal_name);
    if(it == game.content.mob_types.list.pile.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of pile by internal name.
 *
 * @param list This list gets filled with the mob type internal names.
 */
void pile_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.content.mob_types.list.pile) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of pile.
 *
 * @param internal_name Internal name of the mob type.
 * @param type Mob type to register.
 */
void pile_category::register_type(const string &internal_name, mob_type* type) {
    game.content.mob_types.list.pile[internal_name] = (pile_type*) type;
}
