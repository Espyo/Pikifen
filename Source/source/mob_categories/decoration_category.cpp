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

#include "../game.h"
#include "../mobs/decoration.h"


/**
 * @brief Constructs a new decoration category object.
 */
decoration_category::decoration_category() :
    mob_category(
        MOB_CATEGORY_DECORATIONS, "Decoration", "Decorations",
        "Decorations", al_map_rgb(191, 204, 139)
    ) {
    
}


/**
 * @brief Clears the list of registered types of decorations.
 */
void decoration_category::clear_types() {
    for(auto &t : game.content.mob_types.decoration) {
        delete t.second;
    }
    game.content.mob_types.decoration.clear();
}


/**
 * @brief Creates a decoration and adds it to the list of decorations.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
mob* decoration_category::create_mob(
    const point &pos, mob_type* type, float angle
) {
    decoration* m = new decoration(pos, (decoration_type*) type, angle);
    game.states.gameplay->mobs.decorations.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of decoration.
 *
 * @return The type.
 */
mob_type* decoration_category::create_type() {
    return new decoration_type();
}


/**
 * @brief Clears a decoration from the list of decorations.
 *
 * @param m The mob to erase.
 */
void decoration_category::erase_mob(mob* m) {
    game.states.gameplay->mobs.decorations.erase(
        find(
            game.states.gameplay->mobs.decorations.begin(),
            game.states.gameplay->mobs.decorations.end(),
            (decoration*) m
        )
    );
}


/**
 * @brief Returns a type of decoration given its name.
 *
 * @param name Name of the mob type to get.
 * @return The type, or nullptr on error.
 */
mob_type* decoration_category::get_type(const string &name) const {
    auto it = game.content.mob_types.decoration.find(name);
    if(it == game.content.mob_types.decoration.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of decoration by name.
 *
 * @param list This list gets filled with the mob type names.
 */
void decoration_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.content.mob_types.decoration) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of decoration.
 *
 * @param type Mob type to register.
 */
void decoration_category::register_type(mob_type* type) {
    game.content.mob_types.decoration[type->name] = (decoration_type*) type;
}
