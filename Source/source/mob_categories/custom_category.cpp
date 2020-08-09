/*
 * Copyright (c) Andre 'Espyo' Silva 2013.
 * The following source file belongs to the open-source project Pikifen.
 * Please read the included README and LICENSE files for more information.
 * Pikmin is copyright (c) Nintendo.
 *
 * === FILE DESCRIPTION ===
 * Custom mob category class.
 */

#include <algorithm>

#include "custom_category.h"

#include "../game.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the category for the custom mob types.
 */
custom_category::custom_category() :
    mob_category(
        MOB_CATEGORY_CUSTOM, "Custom", "Custom",
        "Custom", al_map_rgb(178, 73, 204)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of custom mob.
 */
void custom_category::clear_types() {
    for(auto &t : game.mob_types.custom) {
        delete t.second;
    }
    game.mob_types.custom.clear();
}


/* ----------------------------------------------------------------------------
 * Creates a custom mob and adds it to the list of custom mobs.
 * pos:
 *   Starting coordinates.
 * type:
 *   Mob type.
 * angle:
 *   Starting angle.
 */
mob* custom_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    mob* m = new mob(pos, type, angle);
    return m;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty custom type.
 */
mob_type* custom_category::create_type() {
    return new mob_type(MOB_CATEGORY_CUSTOM);
}


/* ----------------------------------------------------------------------------
 * Clears a custom mob from the list of custom mobs.
 * m:
 *   The mob to erase.
 */
void custom_category::erase_mob(mob* m) { }


/* ----------------------------------------------------------------------------
 * Returns a custom type given its name, or NULL on error.
 * name:
 *   Name of the mob type to get.
 */
mob_type* custom_category::get_type(const string &name) const {
    auto it = game.mob_types.custom.find(name);
    if(it == game.mob_types.custom.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Returns all custom types by name.
 * list:
 *   This list gets filled with the mob type names.
 */
void custom_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.mob_types.custom) {
        list.push_back(t.first);
    }
}


/* ----------------------------------------------------------------------------
 * Registers a created custom type.
 * type:
 *   Mob type to register.
 */
void custom_category::register_type(mob_type* type) {
    game.mob_types.custom[type->name] = type;
}
