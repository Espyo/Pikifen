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

#include "../game.h"
#include "../mob_types/interactable_type.h"


/* ----------------------------------------------------------------------------
 * Creates an instance of the interactable mob category.
 */
interactable_category::interactable_category() :
    mob_category(
        MOB_CATEGORY_INTERACTABLES, "Interactable", "Interactables",
        "Interactables", al_map_rgb(204, 139, 178)
    ) {
    
}


/* ----------------------------------------------------------------------------
 * Clears the list of registered types of interactables.
 */
void interactable_category::clear_types() {
    for(auto &t : game.mob_types.interactable) {
        delete t.second;
    }
    game.mob_types.interactable.clear();
}


/* ----------------------------------------------------------------------------
 * Creates an interactable and adds it to the list of interactables.
 * pos:
 *   Starting coordinates.
 * type:
 *   Mob type.
 * angle:
 *   Starting angle.
 */
mob* interactable_category::create_mob(
    const point &pos, mob_type* type, const float angle
) {
    interactable* m = new interactable(pos, (interactable_type*) type, angle);
    game.states.gameplay->mobs.interactables.push_back(m);
    return m;
}


/* ----------------------------------------------------------------------------
 * Creates a new, empty type of interactable.
 */
mob_type* interactable_category::create_type() {
    return new interactable_type();
}


/* ----------------------------------------------------------------------------
 * Clears an interactable from the list of interactables.
 * m:
 *   The mob to erase.
 */
void interactable_category::erase_mob(mob* m) {
    game.states.gameplay->mobs.interactables.erase(
        find(
        game.states.gameplay->mobs.interactables.begin(),
        game.states.gameplay->mobs.interactables.end(),
        (interactable*) m
        )
    );
}


/* ----------------------------------------------------------------------------
 * Returns a type of interactable given its name, or NULL on error.
 * name:
 *   Name of the mob type to get.
 */
mob_type* interactable_category::get_type(const string &name) const {
    auto it = game.mob_types.interactable.find(name);
    if(it == game.mob_types.interactable.end()) return NULL;
    return it->second;
}


/* ----------------------------------------------------------------------------
 * Returns all types of interactable by name.
 * list:
 *   This list gets filled with the mob type names.
 */
void interactable_category::get_type_names(vector<string> &list) const {
    for(auto &t : game.mob_types.interactable) {
        list.push_back(t.first);
    }
}


/* ----------------------------------------------------------------------------
 * Registers a created type of interactable.
 * type:
 *   Mob type to register.
 */
void interactable_category::register_type(mob_type* type) {
    game.mob_types.interactable[type->name] = (interactable_type*) type;
}
