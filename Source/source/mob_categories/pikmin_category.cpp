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

#include "../game.h"
#include "../mobs/pikmin.h"


/**
 * @brief Constructs a new Pikmin category object.
 */
pikmin_category::pikmin_category() :
    mob_category(
        MOB_CATEGORY_PIKMIN, "Pikmin", "Pikmin",
        "Pikmin", al_map_rgb(100, 204, 73)
    ) {
    
}


/**
 * @brief Clears the list of registered types of Pikmin.
 */
void pikmin_category::clear_types() {
    for(auto &t : game.mob_types.pikmin) {
        delete t.second;
    }
    game.mob_types.pikmin.clear();
}


/**
 * @brief Creates a Pikmin and adds it to the list of Pikmin.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
mob* pikmin_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    pikmin* m = new pikmin(pos, (pikmin_type*) type, angle);
    game.states.gameplay->mobs.pikmin_list.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of Pikmin.
 *
 * @return The type.
 */
mob_type* pikmin_category::create_type() {
    return new pikmin_type();
}


/**
 * @brief Clears a Pikmin from the list of Pikmin.
 *
 * @param m The mob to erase.
 */
void pikmin_category::erase_mob(mob* m) {
    game.states.gameplay->mobs.pikmin_list.erase(
        find(
            game.states.gameplay->mobs.pikmin_list.begin(),
            game.states.gameplay->mobs.pikmin_list.end(),
            (pikmin*) m
        )
    );
}


/**
 * @brief Returns a type of Pikmin given its name.
 *
 * @param name Name of the mob type to get.
 * @return The type, or nullptr on error.
 */
mob_type* pikmin_category::get_type(const string &name) const {
    auto it = game.mob_types.pikmin.find(name);
    if(it == game.mob_types.pikmin.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of Pikmin by name.
 *
 * @param list This list gets filled with the mob type names.
 */
void pikmin_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.mob_types.pikmin) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of Pikmin.
 *
 * @param type Mob type to register.
 */
void pikmin_category::register_type(mob_type* type) {
    game.mob_types.pikmin[type->name] = (pikmin_type*) type;
}
