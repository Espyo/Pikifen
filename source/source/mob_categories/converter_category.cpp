/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Converter mob category class.
 */

#include <algorithm>

#include "converter_category.h"

#include "../game.h"
#include "../mobs/converter.h"


/**
 * @brief Constructs a new converter category object.
 *
 */
converter_category::converter_category() :
    mob_category(
        MOB_CATEGORY_CONVERTERS, "converter",
        "Converter", "Converters",
        "converters", al_map_rgb(73, 126, 204)
    ) {
    
}


/**
 * @brief Clears the list of registered types of converters.
 */
void converter_category::clear_types() {
    for(auto &t : game.content.mob_types.list.converter) {
        delete t.second;
    }
    game.content.mob_types.list.converter.clear();
}


/**
 * @brief Creates a converter and adds it to the list of converters.
 *
 * @param pos Starting coordinates.
 * @param type Mob type.
 * @param angle Starting angle.
 * @return The mob.
 */
mob* converter_category::create_mob(
    const point &pos, mob_type* type, float angle
) {
    converter* m = new converter(pos, (converter_type*) type, angle);
    game.states.gameplay->mobs.converters.push_back(m);
    return m;
}


/**
 * @brief Creates a new, empty type of converter.
 *
 * @return The type.
 */
mob_type* converter_category::create_type() {
    return new converter_type();
}


/**
 * @brief Clears a converter from the list of converters.
 *
 * @param m The mob to erase.
 */
void converter_category::erase_mob(mob* m) {
    game.states.gameplay->mobs.converters.erase(
        find(
            game.states.gameplay->mobs.converters.begin(),
            game.states.gameplay->mobs.converters.end(),
            (converter*) m
        )
    );
}


/**
 * @brief Returns a type of converter given its name.
 *
 * @param name Name of the mob type to get.
 * @return The type, or nullptr on error.
 */
mob_type* converter_category::get_type(const string &name) const {
    auto it = game.content.mob_types.list.converter.find(name);
    if(it == game.content.mob_types.list.converter.end()) return nullptr;
    return it->second;
}


/**
 * @brief Returns all types of converter by name.
 *
 * @param list This list gets filled with the mob type names.
 */
void converter_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.content.mob_types.list.converter) {
        list.push_back(t.first);
    }
}


/**
 * @brief Registers a created type of converter.
 *
 * @param internal_name Internal name of the mob type.
 * @param type Mob type to register.
 */
void converter_category::register_type(const string &internal_name, mob_type* type) {
    game.content.mob_types.list.converter[internal_name] = (converter_type*) type;
}
