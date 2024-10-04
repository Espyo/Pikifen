/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Scale mob category class.
 */

#include <algorithm>

#include "scale_category.h"

#include "../game.h"
#include "../mobs/scale.h"


/**
 * @brief Constructs a new scale category object.
 */
scale_category::scale_category() :
    mob_category(
        MOB_CATEGORY_SCALES, "Scale", "Scales",
        "Scales", al_map_rgb(139, 165, 204)
    ) {
    
}


/**
 * @brief Clears the list of registered types of scale.
 */
void scale_category::clear_types() {
    for(auto &t : game.content.mob_types.scale) {
        delete t.second;
    }
    game.content.mob_types.scale.clear();
}


/**
 * @brief Creates a scale and adds it to the list of scales.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
mob* scale_category::create_mob(
    const point &pos, mob_type* type, float angle
) {
    scale* m = new scale(pos, (scale_type*) type, angle);
    game.states.gameplay->mobs.scales.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of scale.
 *
 * @return The type.
 */
mob_type* scale_category::create_type() {
    return new scale_type();
}


/**
 * @brief Clears a scale from the list of scales.
 *
 * @param m The mob to erase.
 */
void scale_category::erase_mob(mob* m) {
    game.states.gameplay->mobs.scales.erase(
        find(
            game.states.gameplay->mobs.scales.begin(),
            game.states.gameplay->mobs.scales.end(),
            (scale*) m
        )
    );
}


/**
 * @brief Returns a type of scale given its name.
 *
 * @param name Name of the mob type to get.
 * @return The type, or nullptr on error.
 */
mob_type* scale_category::get_type(const string &name) const {
    auto it = game.content.mob_types.scale.find(name);
    if(it == game.content.mob_types.scale.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of scale by name.
 *
 * @param list This list gets filled with the mob type names.
 */
void scale_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.content.mob_types.scale) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of scale.
 *
 * @param type Mob type to register.
 */
void scale_category::register_type(mob_type* type) {
    game.content.mob_types.scale[type->name] = (scale_type*) type;
}
