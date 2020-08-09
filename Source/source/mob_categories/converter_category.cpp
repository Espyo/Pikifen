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


/* ----------------------------------------------------------------------------
 * Creates an instance of the converter category.
 */
converter_category::converter_category() :
    mob_category(
        MOB_CATEGORY_CONVERTERS, "Converter", "Converters",
        "Converters", al_map_rgb(73, 126, 204)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of converters.
 */
void converter_category::clear_types() {
    for(auto &t : game.mob_types.converter) {
        delete t.second;
    }
    game.mob_types.converter.clear();
}


/* ----------------------------------------------------------------------------
 * Creates a converter and adds it to the list of converters.
 * pos:
 *   Starting coordinates.
 * type:
 *   Mob type.
 * angle:
 *   Starting angle.
 */
mob* converter_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    converter* m = new converter(pos, (converter_type*) type, angle);
    game.states.gameplay_st->mobs.converters.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of converter.
 */
mob_type* converter_category::create_type() {
    return new converter_type();
}


/* ----------------------------------------------------------------------------
 * Clears a converter from the list of converters.
 * m:
 *   The mob to erase.
 */
void converter_category::erase_mob(mob* m) {
    game.states.gameplay_st->mobs.converters.erase(
        find(
            game.states.gameplay_st->mobs.converters.begin(),
            game.states.gameplay_st->mobs.converters.end(),
            (converter*) m
        )
    );
}


/* ----------------------------------------------------------------------------
 * Returns a type of converter given its name, or NULL on error.
 * name:
 *   Name of the mob type to get.
 */
mob_type* converter_category::get_type(const string &name) const {
    auto it = game.mob_types.converter.find(name);
    if(it == game.mob_types.converter.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Returns all types of converter by name.
 * list:
 *   This list gets filled with the mob type names.
 */
void converter_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.mob_types.converter) {
        list.push_back(t.first);
    }
}


/* ----------------------------------------------------------------------------
 * Registers a created type of converter.
 * type:
 *   Mob type to register.
 */
void converter_category::register_type(mob_type* type) {
    game.mob_types.converter[type->name] = (converter_type*) type;
}
